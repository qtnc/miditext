#include "MidiFile.hpp"
#include "../common/stringUtils.hpp"
#include "../common/bass.h"
#include "../common/bassmidi.h"
#include<string>
#include<unordered_map>
#include<boost/regex.hpp>
#include "../common/println.hpp"
extern "C"{
#include<lua/lua.h>
#include<lua/lualib.h>
#include<lua/lauxlib.h>
}
using namespace std;
using namespace boost;

typedef boost::regex tregex;
typedef boost::cregex_iterator tcregex_iterator;
typedef boost::cmatch tcmatch;
typedef boost::sub_match<const char*> tsub_match;

struct MT_CMD {
const string cmd, val, var, longval, suffix;
int oct, alter;
int begin, end;
};

struct MT_REPEAT_STACK {
int count, start;
unordered_map<int,int> marks;
MT_REPEAT_STACK (int c, int s): count(c), start(s), marks() {}
};

struct MT_CHAN {
int pos=0, prevPos=0, velocity=127, octave=5, transpose=0, maxNoteLength=0, multiplier=480;
bool drum=false;
struct{ int duration=0, volume=0, count=0, channel=0, octave=0; } echo;
vector<MT_REPEAT_STACK> repeatStack;
vector<MT_CMD> onNoteOn;
};

static unordered_map<char,int> NOTES = {
{'C', -12},
{'D', -10},
{'E', -8},
{'F', -7},
{'G', -5},
{'A', -3},
{'B', -1},
{'c', 0},
{'d', 2},
{'e', 4},
{'f', 5},
{'g', 7},
{'a', 9},
{'b', 11}
};

static unordered_map<std::string, int> CTRL_NAMES = {
{ "bank", 0 }, { "bankmsb", 0 },
{ "vib", 1 }, { "vibrato", 1 },
{ "mod", 1 }, { "modulation", 1 },
{ "breath", 2 },
{ "foot", 4 },
{ "portatime", 5 }, { "portamentotime", 5 },
{ "data", 6 }, { "datamsb", 6 }, 
{ "volume", 7 }, { "vol", 7 },
{ "balance", 8 },
{ "pan", 10 }, { "panning", 10 },
{ "expression", 11 }, { "expr", 11 },
{ "banklsb", 32 },
{ "datalsb", 38 }, 
{ "sustain", 64 }, { "hold", 64 }, { "pedal", 64 }, { "hold1", 64 },
{ "portamento", 65 }, { "porta", 65 },
{ "sostenuto", 66 },
{ "soft", 67 },
{ "legato", 68 },
{ "hold2", 69 },
{ "resonnance", 71 }, { "filterq", 71 }, { "timbre", 71 },
{ "release", 72 },
{ "attack", 73 },
{ "brightness", 74 }, { "filtercutoff", 74 },
{ "decay", 75 },
{ "vibratorate", 76 }, { "vibratospeed", 76 }, { "vibspeed", 76 },
{ "vibratodepth", 77 }, { "vibdepth", 77 },
{ "vibratodelay", 78 }, { "vibdelay", 78 },
{ "portanote", 84 }, { "portamentonote", 84 },
{ "reverb", 91 },
{ "tremolo", 92 },
{ "chorus", 93 },
{ "detune", 94 }, { "userfx", 94 },
{ "phaser", 95 },
{ "dataincrement", 96 }, { "datainc", 96 },
{ "datadecrement", 97 }, { "datadec", 97 },
{ "nrpnlsb", 98 }, 
{ "nrpnmsb", 99 }, 
{ "rpnlsb", 100 }, 
{ "rpnmsb", 101 },
{ "allsoundsoff", 120 },
{ "reset", 121 },
{ "allnotesoff", 123 },
{ "monophonic", 126 }, 
{ "polyphonic", 127 }
};

static const string ALTER_P1 = "#+^'", ALTER_M1 = "_-,";

static const auto regoptions = regex_constants::perl | regex_constants::mod_s |  regex_constants::collate;
static const string reg1t = "\\G\\s*\\K(?:(?'cmd'[a-zA-Z$]+)(?:[:=](?'longval'[/\\w$-]\\S*)|\\((?'longval'[^()]*)\\)|\\{(?'longval'[^{}]*)\\}|\\[(?'longval'[^][]*)\\])|(?'oct'-?\\d?)(?'cmd'[a-zA-Z])(?'alter'[-+=#_]?)(?'abcoct'[,']*)(?'val'\\d*(?:/\\d*)?\\**)(?'var'(?:\\$[\\w\\d]+)?)(?'suffix'[<>!?~]*)|(?'cmd'[][(){}+*%,;:.!?<|&^=>/^~#@\\\\_-]+)(?'val'\\d*))";
static const string reg0t = "^[/#;][^\r\n]*$";
static const tregex reg1(reg1t, regoptions), reg0(reg0t, regoptions);
const string regpd2t = ("^(\\d*)(/\\d*)?(\\*?)$");
tregex regpd2(regpd2t, regoptions);


static inline string sub (const tsub_match& p) {
return string(p.first, p.second);
}

template <class T> int elt (const T& val, int def, const std::vector<T>& vals) {
auto it = std::find(vals.begin(), vals.end(), val);
return it==vals.end()? def : it-vals.begin();
}

void MTTranspose (string& text, int count) {
lua_State* L = luaL_newstate();
luaL_openlibs(L);
luaL_loadfilex(L, "transpose.lua", NULL) || lua_pcall(L, 0, LUA_MULTRET, 0);
lua_settop(L,0);
lua_getglobal(L, "transpose");
lua_pushstring(L, text.c_str()) ;
lua_pushinteger(L, count);
lua_call(L, 2, 1);
text = lua_tostring(L,-1);
lua_close(L);
}

void MTConvertFromV3 (string& text) {
lua_State* L = luaL_newstate();
luaL_openlibs(L);
luaL_loadfilex(L, "convertFromV3.lua", NULL) || lua_pcall(L, 0, LUA_MULTRET, 0);
lua_settop(L,0);
lua_getglobal(L, "convertFromV3");
lua_pushstring(L, text.c_str()) ;
lua_call(L, 1, 1);
text = lua_tostring(L,-1);
lua_close(L);
}

static void SplitCommands (const string& str0, vector<MT_CMD>& mtcmds) {
string str = str0;
auto strBegin = str.data(), strEnd = str.data()+str.size(), finish=strBegin;
for (tcregex_iterator _end, it(strBegin, strEnd, reg0); it!=_end; ++it) {
auto m = *it;
std::fill((char*)m[0].first, (char*)m[0].second, ' ');
}
for (tcregex_iterator _end, it(strBegin, strEnd, reg1); it!=_end; ++it) {
auto m = *it;
string octS = sub(m["oct"]), 
abcoctS = sub(m["abcoct"]),
alterS = sub(m["alter"]);
int alter=0, 
oct  = octS.empty()? 0 : stoi(octS), 
abcoct = std::count(abcoctS.begin(), abcoctS.end(), '\'') - std::count(abcoctS.begin(), abcoctS.end(), ',');
if (!alterS.empty() && ALTER_P1.find(alterS)<ALTER_P1.size()) alter=1;
else if (!alterS.empty() && ALTER_M1.find(alterS)<ALTER_M1.size()) alter= -1;
finish = const_cast<char*>( m[0].second );
mtcmds.push_back({
sub(m["cmd"]),
sub(m["val"]),
sub(m["var"]),
sub(m["longval"]),
sub(m["suffix"]),
oct + abcoct, 
alter,
m[0].first -strBegin, m[0].second -strBegin
});
}
string unparsed(finish, strEnd);
trim(unparsed);
if (!unparsed.empty())  throw syntax_error("syntax error", finish-strBegin);
}

static inline void checkmark (const int& m, int& out, const int& i, const MT_CMD& e) {
if (m>=e.begin) out=i;
}

static inline string GetVariable (lua_State* L, MT_CHAN& ch, const string& name) {
lua_settop(L,0);
lua_getglobal(L, (name).c_str() );
if (!lua_isnoneornil(L,-1)) return (lua_tostring(L,-1));
throw logic_error(format("{} is undefined", name));
}

static void SetVariable (lua_State* L, const string& name, const string& value) {
string code = (name + ("=") + value);
lua_settop(L,0);
if (luaL_dostring(L, code.c_str() )) {
const char* errstr = lua_tostring(L,-1);
throw logic_error(string(errstr));
}}

static vector<string> SplitLongValue (const MT_CMD& e, MT_CHAN& ch, size_t min, size_t max) {
vector<string> v = ::split(e.longval, " ,;", true);
if (v.size()<min) throw range_error(format("Too few parameters specified; expecting at least {}, got {}", min, v.size()));
else if (v.size()>max) throw range_error(format("Too much parameters specified; expecting at most {}, got {}", max, v.size()));
for (string& s: v) trim(s);
return v;
}

static int ParseDuration(const string& str, int multiplier) {
const auto mtype = match_flag_type::match_default;
const char *valBegin = str.data(), *valEnd = valBegin + str.size();
tcmatch m;
if (!regex_search(valBegin, valEnd, m, regpd2, mtype)) return multiplier;
string numS = sub(m[1]), denomS = sub(m[2]), dottedS = sub(m[3]);
int num = numS.empty()? 1 : stoi(numS), denom = 1;
bool dotted = !dottedS.empty();
if (denomS==("/")) denom=2;
else if (!denomS.empty()) denom = stoi(denomS.substr(1));
if (dotted) return 3 * multiplier * num / (2 * denom);
else return multiplier * num / denom;
}

static inline int ParseDuration(lua_State* L, const string& str, MT_CHAN& ch, int multiplier) {
if (starts_with(str,("$"))) return ParseDuration(GetVariable(L, ch, str.substr(1)), multiplier);
else return ParseDuration(str, multiplier);
}

static inline int ParseDuration(lua_State* L, const MT_CMD& e, MT_CHAN& ch, int multiplier) {
return ParseDuration(L, e.val.empty()? e.var : e.val, ch, multiplier);
}

static int ParseSimpleNote (const string& str, MT_CHAN& ch) {
auto options = regex_constants::perl | regex_constants::mod_s | regex_constants::collate;
tregex reg(("^(?'oct'-?\\d?)(?'note'[a-gA-G])(?'alter'[-+=#_^]?)$"), options);
tcmatch m;
if (regex_search(str.data(), str.data()+str.size(), m, reg, match_flag_type::match_default)) {
string octS = sub(m["oct"]), alterS = sub(m["alter"]);
int oct=ch.octave, alter=0, note=NOTES[sub(m["note"])[0]];
if (!alterS.empty() && ALTER_P1.find(alterS)<ALTER_P1.size()) alter=1;
else if (!alterS.empty() && ALTER_M1.find(alterS)<ALTER_M1.size()) alter= -1;
if (!octS.empty()) oct += stoi(octS);
return 12*oct + note + alter;
}
throw logic_error("integer expected");
}

static int ParseInt (const string& str, int min, int max, int def=-1, bool raise=true) {
if (str.empty()) {
if (raise) throw range_error(format("A value between {} and {} must be specified", min, max));
else return def;
}
int val;
try {
val = stoi(str);
} catch (std::exception& e) {
if (raise) throw range_error(format("A value between {} and {} must be specified", min, max));
else return def;
}
if ((val>max || val<min)  && raise) throw range_error(format("Value out of bounds; expecting a value between {} and {}, got {}", min, max, val));
return std::min(std::max(min, val), max);
}

static int ParseInt (lua_State* L, const string& str, MT_CHAN& ch, int min, int max, int def=-1, bool raise=true) {
if (starts_with(str,("$"))) return ParseInt(GetVariable(L, ch,str.substr(1)), min, max, def, raise);
else if (starts_with(str,("&"))) return ParseSimpleNote(str.substr(1), ch);
else return ParseInt(str, min, max, def, raise);
}

static inline int ParseInt (lua_State* L, const MT_CMD& e, MT_CHAN& ch, int min, int max, int def=-1, bool raise=true) {
return ParseInt(L, e.val.empty()? e.var : e.val, ch, min, max, def, raise);
}

static inline bool IsEmptyValue (const MT_CMD& e) {
return e.val.empty() && e.var.empty();
}

static int ParseControllerIdOrName (lua_State* L, const std::string& str, MT_CHAN& ch) {
int ctrl = ParseInt(L, str, ch, 0, 127, -1, false);
if (ctrl<0 || ctrl>127) {
auto it = CTRL_NAMES.find(str);
if (it==CTRL_NAMES.end()) throw logic_error(format("Invalid value; expecting a value between 0 and 127 or CC name, got {}", str));
ctrl = it->second;
}
return ctrl;
}

static void ParseIntOrStringAndAppend (lua_State* L, vector<unsigned char>& out, MT_CHAN& ch, string& s) {
if (starts_with(s, "$")) {
lua_settop(L,0);
string name = trim_copy(s).substr(1);
lua_getglobal(L, (name).c_str() );
if (lua_isnoneornil(L, -1)) throw logic_error(format("{} is undefined", name));
else if (lua_isinteger(L, -1)) out.push_back(lua_tointeger(L, -1));
else if (lua_isboolean(L, -1)) out.push_back(lua_toboolean(L, -1)? 127 : 0);
else if (lua_isstring(L, -1)) {
size_t len=0;
const char* ls = lua_tolstring(L, -1, &len);
out.insert(out.end(), ls, ls+len);
}
}
else if (starts_with(s,("\""))) {
auto start = s.begin()+1, end=s.begin()+s.rfind(("\""));
out.insert(out.end(), start, end);
}
else if (iends_with(s,("f"))) {
istringstream z(s);
float f;
z >> f;
const unsigned char* ptr = (const unsigned char*)&f;
out.insert(out.end(), ptr, ptr+sizeof(float));
}
else if (iends_with(s,("d"))) {
istringstream z(s);
double f;
z>> f;
const unsigned char* ptr = (const unsigned char*)&f;
out.insert(out.end(), ptr, ptr+sizeof(double));
}
else if (iends_with(s,("S")) || iends_with(s, ("h")) ) {
short sh = stoi(s);
const unsigned char* ptr = (const unsigned char*)&sh;
out.insert(out.end(), ptr, ptr+sizeof(short));
}
else if (iends_with(s,("J"))) {
long long l = stoll(s);
const unsigned char* ptr = (const unsigned char*)&l;
out.insert(out.end(), ptr, ptr+sizeof(long long));
}
else if (iends_with(s,("L"))) {
int n = stoi(s);
const unsigned char* ptr = (const unsigned char*)&n;
out.insert(out.end(), ptr, ptr+sizeof(int));
}
else if (iends_with(s,("M"))) {
int n = stoi(s);
const unsigned char* ptr = (const unsigned char*)&n;
out.insert(out.end(), ptr, ptr+sizeof(int) -1);
}
else {
int val = stoi(s);
out.push_back(val);
}}

static void CompileCommands (const vector<MT_CMD>& cmds, MidiFile& m, std::vector<MT_CHAN>& chans, lua_State* L, int curChan, std::vector<std::pair<int,int>>& marks);

static void AddNote (lua_State* L, MidiFile& m, int note, int velocity, int increment, int chIndex, std::vector<MT_CHAN>& chans) {
auto& ch = chans[chIndex];
auto& evs = m.events;
int duration = ch.maxNoteLength? std::min(increment, ch.maxNoteLength) : increment;
note = std::min(std::max(0, note), 127);
evs.push_back(MidiEvent(ch.pos, 0, 0x90+chIndex, note, velocity));
evs.push_back(MidiEvent(ch.pos+duration, 0, 0x90+chIndex, note, 0));
if (ch.echo.duration && ch.echo.volume && ch.echo.count) for (int z=0, 
pos=ch.pos+ch.echo.duration,
vol = velocity * ch.echo.volume /100,
num = std::min(std::max(0, note + 12*ch.echo.octave), 127);
z<ch.echo.count && num>=0 && num<127 && vol>0;
z++, 
pos+=ch.echo.duration,
vol = vol * ch.echo.volume /100
) {
evs.push_back(MidiEvent(pos, 0, 0x90+ch.echo.channel, num, vol));
evs.push_back(MidiEvent(pos+duration, 0, 0x90+ch.echo.channel, num, 0));
}
if (!ch.onNoteOn.empty()) {
lua_pushinteger(L, note);
lua_setglobal(L, "note");
lua_pushinteger(L, velocity);
lua_setglobal(L, "velocity");
SetVariable(L, "duration", format("'{}/{}'", duration, ch.multiplier));
std::vector<std::pair<int,int>> unusedMarks;
CompileCommands(ch.onNoteOn, m, chans, L, chIndex, unusedMarks);
}
ch.prevPos = ch.pos;
ch.pos += increment;
}

static void AddSlide(vector<MidiEvent>& evs, int start, int duration, int status, int data1, int data2, int min, int max, int MidiEvent::* ptr) {
int diff = max-min;
double valInc= (max-min)>0? 1 : -1, timeInc = 1.0 * duration/abs(diff);
if (timeInc<1) { timeInc=1; valInc = 1.0 * diff/duration; }
for (double time=0, val=min; time<=duration && (val-max)*(min-max)>=0; time+=timeInc, val+=valInc) {
MidiEvent e((int)round(time+start), 0, status, data1, data2);
e.*ptr = (int)round(val);
evs.push_back(e);
}}

static void HandleRepeat (lua_State* L, const MT_CMD& e, MT_CHAN& ch, int& pos) {
if (ch.repeatStack.empty()) throw logic_error("Missing matching repeatition bar");
auto& mark = ch.repeatStack.back();
int count = ParseInt(L, e, ch, 1, 1000000);
mark.marks[count] = pos;
if (mark.count==count) return;
else if (count<mark.count) {
pos = mark.marks[mark.count];
return;
}
if (mark.count>10) return;
pos =mark.start;
mark.count++;
}

static MidiEvent CreateKeySigEv (const MT_CMD& e, int tick) {
if (e.longval.size()<2 || e.longval.size()>3) throw logic_error("Syntax error in key signature");
shared_array<unsigned char> a(new unsigned char[2]);
int alter=0, note = string(("FCGDAEBfcgdaeb")) .find(e.longval[0]);
if (note<0 || note>=14) throw logic_error("Syntax error in key signature");
else note = (note%7) -1;
if (ALTER_P1.find(e.longval[1])<ALTER_P1.size()) alter=1;
else if (ALTER_M1.find(e.longval[1])<ALTER_M1.size()) alter= -1;
a[1] = ends_with(e.longval, ("m"))? 1 : 0;
a[0] = ( -3*a[1] + 7*alter + note )%8;
return MidiEvent(tick, 0, 255, a, 0, 0x59);
}

static MidiEvent CreateTimeSigEv (const MT_CMD& e, int tick) {
shared_array<unsigned char> a(new unsigned char[4]);
vector<string> v = split(e.longval, ("/"));
if (v.size()!=2) throw logic_error("Syntax error in time signature");
int num = ParseInt(v[0], 2, 32), denom = ParseInt(v[1], 2, 32);
double denomD = log(denom)/log(2);
if (denomD!=(int)denomD) throw logic_error("Syntax error in time signature");
a[0] = num;
a[1] = (int)denomD;
a[2] = num*6;
a[3] = 8;
return MidiEvent(tick, 0, 255, &a[0], 4, 0x58);
}

static void CompileCommands (const vector<MT_CMD>& cmds, MidiFile& m, std::vector<MT_CHAN>& chans, lua_State* L, int curChan, std::vector<std::pair<int,int>>& marks) {
for (int i=0, n=cmds.size(); i<n; i++) {
const MT_CMD& e = cmds[i];
MT_CHAN& ch = chans[curChan];
if (e.cmd.empty()) continue;
for (auto& m: marks) checkmark(m.first, m.second, ch.pos, e);
try {

// Short single letter commands
if (e.cmd.size()==1 && e.longval.empty()) switch(e.cmd[0]){
case 'a' ... 'g' :
case 'A' ... 'G' :
AddNote(L, m, 
12*(ch.octave+e.oct) + NOTES[e.cmd[0]] + e.alter + (ch.drum? 0 : ch.transpose), 
ch.velocity, 
ParseDuration(L, e, ch, ch.multiplier),
curChan, chans);
break;
case 'o':
ch.octave = ParseInt(L, e, ch, 0, 10);
break;
case 'v':
ch.velocity = ParseInt(L, e, ch, 1, 127);
break;
case 'r': case 's': case 'z':
ch.prevPos = ch.pos;
ch.pos += ParseDuration(L, e, ch, ch.multiplier);
break;
case 'R': case 'S': case 'Z':
ch.prevPos = ch.pos;
ch.pos -= ParseDuration(L, e, ch, ch.multiplier);
break;
case '<':
ch.octave--;
break;
case '>':
ch.octave++;
break;
case '&':
ch.pos = ch.prevPos;
break;
case 'L':
ch.multiplier = ParseDuration(L, e, ch, m.ppq);
break;
case 'm':
ch.maxNoteLength = ParseDuration(L, e, ch, ch.multiplier);
break;
case 'P':
ch.pos = ParseDuration(L, e, ch, ch.multiplier);
break;
case 'h':
m.events.push_back(MidiEvent(ch.pos, 0, 0xE0+curChan, ParseInt(L, e, ch, 0, 16383)));
break;
case 'H': 
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 101, 0));
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 100, 0));
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 6,  ParseInt(L, e, ch, 0, 127) ));
break;
case 'w':
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 1, ParseInt(L, e, ch, 0, 127)));
break;
case 'x':
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 11, ParseInt(L, e, ch, 0, 127)));
break;
case 'V':
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 7, ParseInt(L, e, ch, 0, 127)));
break;
case 'n': 
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 10, ParseInt(L, e, ch, 0, 127)));
break;
case 'u': 
if (IsEmptyValue(e))  m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 66, 127));
else  m.events.push_back(MidiEvent(ch.pos, 0, 0xD0+curChan, ParseInt(L, e, ch, 0, 127) ));
break;
case 'U':
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 66, 0));
break;
case 'q':
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 67, 127));
break;
case 'Q':
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 67, 0));
break;
case 'j': 
if (IsEmptyValue(e)) m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 65, 127));
else m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 5, ParseInt(L, e, ch, 0, 127) ));
break;
case 'J': 
if (IsEmptyValue(e)) m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 65, 0));
else m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 84, ParseInt(L, e, ch, 0, 127) ));
break;
case 'k':
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 64, 127));
break;
case 'K':
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 65, 0));
break;
case 'W': {
int val = ParseInt(L, e, ch, 0, 16383);
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 101, 0));
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 100, 5));
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 6, val>>7));
//m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 38, val&0x7F));
}break;
case 'p': case 'i': {
int val = ParseInt(L, e, ch, 0, 2097151);
if (val>127) m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 0, (val>>14)&0x7F));
if (val>127) m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 32, (val>>7)&0x7F));
m.events.push_back(MidiEvent(ch.pos, 0, 0xC0+curChan, val&0x7F));
}break;
case 't': {
int bpm = ParseInt(L, e, ch, 1, 60000000);
int mpq = 60000000/bpm;
MidiEvent ev(ch.pos, 0, 255, 0, 81);
ev.setDataAsInt24(mpq);
ev.data2 = 81;
m.events.push_back(ev);
}break;
case '(': case '{': case '[':
ch.repeatStack.emplace_back( 1, i);
break;
case '|': 
if (!IsEmptyValue(e)) HandleRepeat(L, e, ch, i);
break;
case ')': case '}': case ']': {
int count = ParseInt(L, e, ch, 1, 1000000);
auto& mark = ch.repeatStack.at(ch.repeatStack.size() -1);
if (mark.count==count) ch.repeatStack.pop_back();
else {
i = mark.start;
mark.count++;
}}break;
default: throw logic_error(format("Unknown command: {}", e.cmd));
}//switch single letter short command

// Long single letter commands
else if (e.cmd.size()==1 && e.val.empty() && e.var.empty()) switch(e.cmd[0]){
case 'X': m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 1)); break;
case 'C': m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 2)); break;
case 'T': m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 3)); break;
case 'I': m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 4)); break;
case 'L': m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 5)); break;
case 'R': m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 6)); break;
case 'P': m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 7)); break;
case 'M': m.events.push_back(CreateTimeSigEv(e, ch.pos)); break;
case 'Q': m.events.push_back(CreateKeySigEv(e, ch.pos)); break;
case 'V': 
curChan = ParseInt(L, e.longval, ch, 1, 16) -1; 
lua_pushinteger(L, curChan+1);
lua_setglobal(L,"channel");
break;
case 'A': {
vector<string> v = SplitLongValue(e,ch, 2, 2);
int key = ParseInt(L, v[0], ch, 0, 127), 
val = ParseInt(L, v[1], ch, 0, 127);
m.events.push_back(MidiEvent(ch.pos, 0, 0xA0+curChan, key, val));
}break;
case 'p': case 'i': {
vector<string> v = SplitLongValue(e,ch, 2, 3);
int msb = v.size()==2? ParseInt(L, v[0], ch, 0, 16383) :
(ParseInt(L, v[0], ch, 0, 127)<<7) | ParseInt(L, v[1], ch, 0, 127), 
lsb = ParseInt(L, v[v.size() -1], ch, 0, 127);
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 0, (msb>>7)&0x7F));
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 32, msb&0x7F));
m.events.push_back(MidiEvent(ch.pos, 0, 0xC0+curChan, lsb&0x7F));
}break;
default: throw logic_error(format("Unknown command: {}", e.cmd));
}//switch single letter long command

// Elaborate commands
else if (e.cmd==("ctrl")) {
vector<string> v = SplitLongValue(e,ch, 2, 2);
int ctrl = ParseControllerIdOrName(L, v[0], ch);
int val = ParseInt(L, v[1], ch, 0, 127);
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, ctrl, val));
}
else if (e.cmd=="slide") {
vector<string> v = SplitLongValue(e,ch, 4, 6);
int ctrl, k;
if (v[0]=="pitch" || v[0]=="pitchbend") ctrl = -1;
else if (v[0]=="pressure" || v[0]=="channelpressure") ctrl = -2;
else if (v[0]=="aftertouch" || v[0]=="polypressure") { ctrl=-3; k=ParseInt(L, v[1], ch, 0, 127); }
else ctrl = ParseControllerIdOrName(L, v[0], ch);
int lim = ctrl==-1? 16383 : 127,
min = ParseInt(L, v[v.size() -3], ch, 0, lim),
max = ParseInt(L, v[v.size() -2], ch, 0, lim),
dur = ParseDuration(L, v[v.size() -1], ch, ch.multiplier);
switch(ctrl){
case 0 ... 127: AddSlide(m.events, ch.pos, dur, 0xB0 + curChan, ctrl, 0, min, max, &MidiEvent::data2); break;
case -1: AddSlide(m.events, ch.pos, dur, 0xE0 + curChan, 0, 0, min, max, &MidiEvent::data1); break;
case -2: AddSlide(m.events, ch.pos, dur, 0xD0 + curChan, 0, 0, min, max, &MidiEvent::data1); break;
case -3: AddSlide(m.events, ch.pos, dur, 0xA0 + curChan, k, 0, min, max, &MidiEvent::data2); break;
default: throw logic_error(format("Unknown slide destination: {}", v[0]));
}}
else if (e.cmd==("rpn")) {
vector<string> v = SplitLongValue(e,ch, 3, 4);
int chsb = ParseInt(L, v[0], ch, 0, 127), 
clsb = ParseInt(L, v[1], ch, 0, 127),
vhsb, vlsb;
if (v.size()>=4) {
vhsb = ParseInt(L, v[2], ch, 0, 127);
vlsb = ParseInt(L, v[3], ch, 0, 127);
}
else {
vhsb = ParseInt(L, v[2], ch, 0, 16383);
vlsb = vhsb&0x7F;
vhsb>>=7;
}
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 101, chsb));
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 100, clsb));
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 6, vhsb));
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 38, vlsb));
}
else if (e.cmd==("nrpn")) {
vector<string> v = SplitLongValue(e,ch, 3, 4);
int chsb = ParseInt(L, v[0], ch, 0, 127), 
clsb = ParseInt(L, v[1], ch, 0, 127),
vhsb, vlsb;
if (v.size()>=4) {
vhsb = ParseInt(L, v[2], ch, 0, 127);
vlsb = ParseInt(L, v[3], ch, 0, 127);
}
else {
vhsb = ParseInt(L, v[2], ch, 0, 16383);
vlsb = vhsb&0x7F;
vhsb>>=7;
}
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 99, chsb));
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 98, clsb));
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 6, vhsb));
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, 38, vlsb));
}
else if (e.cmd==("echo")) {
if (e.longval==("off")) { ch.echo.duration = ch.echo.volume = ch.echo.count = 0; }
else {
vector<string> v = SplitLongValue(e,ch, 1, 5);
ch.echo.duration = ParseDuration(L, v[0], ch, ch.multiplier);
ch.echo.volume = v.size()>=2? ParseInt(L, v[1], ch, 1, 100) : 67;
ch.echo.count = v.size()>=3? ParseInt(L, v[2], ch, 1, 100) : 3;
ch.echo.channel = v.size()>=4? ParseInt(L, v[3], ch, 1, 16) -1 : curChan;
ch.echo.octave = v.size()>=5? ParseInt(L, v[4], ch, -9, 9) : 0;
}}
else if (e.cmd=="note") {
vector<string> v = SplitLongValue(e,ch, 2, 3);
int note = ParseInt(L, v[0], ch, 0, 127),
vel = v.size()>=3? ParseInt(L, v[1], ch, 1, 127) : ch.velocity,
dur = ParseDuration(L, v[v.size() -1], ch, ch.multiplier);
AddNote(L, m, note, vel, dur, curChan, chans);
}
else if (e.cmd==("transpose")) {
int val = ParseInt(L, e.longval, ch, -108, 108);
for (MT_CHAN& c: chans) c.transpose = val;
}
else if (e.cmd=="onnoteon") {
ch.onNoteOn.clear();
if (e.longval!="off") SplitCommands(e.longval, ch.onNoteOn);
}
else if (e.cmd==("pressuredest")) {
vector<string> v = SplitLongValue(e,ch, 2, 2);
int val = ParseInt(L, v[1], ch, 0, 127);
int type = elt(v[0], -1, { "pitch", "filter", "volume", "vibrato", "filterlfo", "tremolo"  });
if (type<0) type = ParseInt(L, v[0], ch, 0, 127);
unsigned char data[] = { 127, 127, 9, 1, static_cast<unsigned char>(curChan), static_cast<unsigned char>(type), static_cast<unsigned char>(val), 247 };
m.events.push_back(MidiEvent(ch.pos, 0, 240, data, sizeof(data)/sizeof(unsigned char)  ));
}
else if (e.cmd==("aftertouchdest")) {
vector<string> v = SplitLongValue(e,ch, 2, 2);
int val = ParseInt(L, v[1], ch, 0, 127);
int type = elt(v[0], -1, { ("pitch"), ("filter"), ("volume"), ("vibrato"), "filterlfo", "tremolo" });
if (type<0) type = ParseInt(L, v[0], ch, 0, 127);
unsigned char data[] = { 127, 127, 9, 2, static_cast<unsigned char>(curChan), static_cast<unsigned char>(type), static_cast<unsigned char>(val), 247 };
m.events.push_back(MidiEvent(ch.pos, 0, 240, data, sizeof(data)/sizeof(unsigned char) ));
}
else if (e.cmd==("meta")) {
vector<string> v = SplitLongValue(e,ch, 0, 1000000);
vector<unsigned char> out;
int type = ParseInt(L, v[0], ch, 0, 127);
v.erase(v.begin()+0);
for (string& s: v) ParseIntOrStringAndAppend(L, out, ch, s);
m.events.push_back(MidiEvent(ch.pos, 0, 255, &out[0], out.size(), type ));
}
else if (e.cmd==("sysex")) {
vector<string> v = SplitLongValue(e,ch, 0, 1000000);
vector<unsigned char> out;
for (string& s: v) ParseIntOrStringAndAppend(L, out, ch, s);
if (out.empty() || out[out.size() -1]!=247) out.push_back(247);
for (int z=0, zn=out.size() -1; z<zn; z++) if (out[z]>=128) throw logic_error(format("Invalid byte in sysex message: value={}, offset={}", (int)out[z], z));
m.events.push_back(MidiEvent(ch.pos, 0, 240, &out[0], out.size() ));
}
else if (e.cmd==("text")) m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 1));
else if (e.cmd==("copyright")) m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 2));
else if (e.cmd==("title")) m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 3));
else if (e.cmd==("instrument")) m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 4));
else if (e.cmd==("lyric")) m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 5));
else if (e.cmd==("mark")) m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 6));
else if (e.cmd==("cue")) m.events.push_back(MidiEvent(ch.pos, 0, 255, (e.longval), 7));
else if (e.cmd==("||") || e.cmd==("|]") || e.cmd==("[|")) ch.repeatStack.pop_back();
else if (e.cmd[e.cmd.size() -1]==':' && e.cmd.find_first_not_of("|[]")==e.cmd.size() -1) {
ch.repeatStack.emplace_back( 1, i );
}
else if (e.cmd[0]==':' && e.cmd.find_first_not_of("|[]",1)==string::npos) {
HandleRepeat(L, e, ch, i);
}
else if (istarts_with(e.cmd, ("cr")) && e.cmd.size()==3) {
vector<string> v = SplitLongValue(e,ch, 3, 4);
int ctrl, lim = e.cmd[2]=='h'? 16383 : 127,
min = ParseInt(L, v[v.size() -3], ch, 0, lim),
max = ParseInt(L, v[v.size() -2], ch, 0, lim),
dur = ParseDuration(L, v[v.size() -1], ch, ch.multiplier);
switch(e.cmd[2]) {
case 'v': case 'V': AddSlide(m.events, ch.pos, dur, 0xB0 + curChan, 7, 0, min, max, &MidiEvent::data2); break;
case 'n': AddSlide(m.events, ch.pos, dur, 0xB0 + curChan, 10, 0, min, max, &MidiEvent::data2); break;
case 'x': AddSlide(m.events, ch.pos, dur, 0xB0 + curChan, 11, 0, min, max, &MidiEvent::data2); break;
case 'w': AddSlide(m.events, ch.pos, dur, 0xB0 + curChan, 1, 0, min, max, &MidiEvent::data2); break;
case 'd': AddSlide(m.events, ch.pos, dur, 0xB0 + curChan, 6, 0, min, max, &MidiEvent::data2); break;
case 'c': 
ctrl = ParseControllerIdOrName(L, v[0], ch);
AddSlide(m.events, ch.pos, dur, 0xB0 + curChan, ctrl, 0, min, max, &MidiEvent::data2); 
break;
case 'k': case 'A': 
ctrl = ParseInt(L, v[0], ch, 0, 127);
AddSlide(m.events, ch.pos, dur, 0xA0 + curChan, ctrl, 0, min, max, &MidiEvent::data2); 
break;
case 'h': AddSlide(m.events, ch.pos, dur, 0xE0 + curChan, 0, 0, min, max, &MidiEvent::data1); break;
case 'u': AddSlide(m.events, ch.pos, dur, 0xD0 + curChan, 0, 0, min, max, &MidiEvent::data1); break;
default: throw logic_error(format("Unknown command: {}", e.cmd));
}}
else if (starts_with(e.cmd, ("$"))) {
SetVariable(L, e.cmd.substr(1), trim_copy(e.longval));
}
else {
auto it = CTRL_NAMES.find(e.cmd);
if (it==CTRL_NAMES.end()) throw logic_error(format("Unknown command: {}", e.cmd));
int val;
if (e.longval=="on") val=127;
else if (e.longval=="off") val=0;
else val = ParseInt(L, e.longval, ch, 0, 127);
m.events.push_back(MidiEvent(ch.pos, 0, 0xB0+curChan, it->second, val));
}
} catch (std::exception& ex) {
throw syntax_error(ex.what(), e.begin);
}}//end MT_CMD loop
}

static void CompileCommands (const vector<MT_CMD>& cmds, MidiFile& m, std::vector<std::pair<int,int>>& marks) {
m.ppq = 480;
m.nTracks = 0;

vector<MT_CHAN> chans(16);
chans[9].drum=true;
lua_State* L = luaL_newstate();
luaL_openlibs(L);
lua_pushinteger(L, 0);
lua_setglobal(L,"channel");
lua_pushinteger(L, m.ppq);
lua_setglobal(L, "ppq");

CompileCommands(cmds, m, chans, L, 0, marks);

lua_close(L); 
L=NULL;
}

MidiFile CompileMidi (const string& text, std::vector<std::pair<int,int>>& marks) {
vector<MT_CMD> cmds;
MidiFile mf;
mf.setCodeText(text);
SplitCommands(text, cmds);
CompileCommands(cmds, mf, marks);
return mf;
}

MidiFile CompileMidi (const string& text) {
std::vector<std::pair<int,int>> marks;
return CompileMidi(text, marks);
}

DWORD CompileMidiAsStream (const string& text, DWORD flags, std::vector<std::pair<int,int>>& marks) {
MidiFile mf = CompileMidi(text, marks);
ostringstream out(ios::binary);
mf.save(out);
string buf = out.str();
DWORD midi = BASS_MIDI_StreamCreateFile(true, buf.data(), 0, buf.size(), BASS_MIDI_SINCINTER | BASS_MIDI_NOTEOFF1 | BASS_MIDI_DECAYEND | BASS_MIDI_DECAYSEEK | BASS_SAMPLE_FLOAT | (flags&BASS_STREAM_DECODE? 0 : BASS_STREAM_AUTOFREE) | flags, 0);
return midi;
}

DWORD CompileMidiAsStream (const string& text, DWORD flags) {
std::vector<std::pair<int,int>> marks;
return CompileMidiAsStream(text, flags, marks);
}

