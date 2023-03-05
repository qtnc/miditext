#include "SoundfontMapping.hpp"
#include "../common/stringUtils.hpp"
#include<sstream>
#include<algorithm>
using namespace std;

#define MKPRESET(P,L,H) (((P)&0x7F) | (((L)&0x7F)<<16) | (((H)&0xFF)<<23))

bool SoundfontEntry::unload () {
bool wasLoaded = !!font;
if (font) BASS_MIDI_FontFree(font);
font=0;
return wasLoaded;
}

bool SoundfontEntry::load () {
unload();
font = BASS_MIDI_FontInit(file.c_str(), flags);
if (font && volume>0) BASS_MIDI_FontSetVolume(font, volume);
return font;
}

int SoundfontMapping::findFontByName (const std::string& name) {
auto it = std::find_if(fonts.begin(), fonts.end(), [&](auto& e){ return iequals(e.name, name); });
return it==fonts.end()? -1 : it-fonts.begin();
}

void SoundfontMapping::addFont (const std::string& file, const std::string& name, DWORD flags, float volume) {
SoundfontEntry entry(file, name, flags, volume);
if (entry.load()) fonts.push_back(entry);
}

void SoundfontMapping::addFontLine (const std::string& s) {
auto t = split(s, ",; ", true);
if (t.empty()) return;
std::string file = t[0], name = t.size()>=2? t[1] : "";
DWORD flags = 0; float volume = 1.0f;
for (int i=2, n=t.size(); i<n; i++) {
auto& a = t[i];
if (a.find_first_not_of("0123456789.")==string::npos) {
istringstream in(a);
in >> volume;
volume /= 100.0f;
}
else if (iequals(a, "linattmod")) flags |= BASS_MIDI_FONT_LINATTMOD;
else if (iequals(a, "lindecvol")) flags |= BASS_MIDI_FONT_LINDECVOL;
else if (iequals(a, "nofx")) flags |= BASS_MIDI_FONT_NOFX;
else if (iequals(a, "nolimit") || iequals(a, "nolimits")) flags |= BASS_MIDI_FONT_NOLIMITS;
else if (iequals(a, "norampin")) flags |= BASS_MIDI_FONT_NORAMPIN;
else if (iequals(a, "xgdrums")) flags |= BASS_MIDI_FONT_XGDRUMS;
}
addFont(file, name, flags, volume);
}

void SoundfontMapping::addMapping (int fontIndex, int sbank, int spreset, int dbank, int dpreset, int dbanklsb) {
if (fontIndex<0 || fontIndex>=static_cast<int>(fonts.size())) return;
mappings.push_back({ fonts[fontIndex].font, sbank, spreset, dbank, dpreset, dbanklsb });
}

void SoundfontMapping::addMapping (const std::string& s) {
auto t = split(s, ",; ", true);
if (t.empty()) return;
int fontIndex = findFontByName(t[0]);
if (fontIndex<0  && t[0].find_first_not_of("0123456789")==string::npos) fontIndex = stoi(t[0]);
if (fontIndex<0 || fontIndex>=static_cast<int>(fonts.size())) return;
int spreset = -1, sbank = -1, dpreset = -1, dbank = 0, dbanklsb = 0;
switch(t.size()) {
case 2: dbank=sbank = stoi(t[1]); break;
case 4: dbanklsb=stoi(t[3]); [[fallthrough]];
case 3: dpreset=spreset=stoi(t[1]); dbank=sbank = stoi(t[2]); break;
case 6: dbanklsb=stoi(t[5]); [[fallthrough]];
case 5: spreset=stoi(t[1]); sbank=stoi(t[2]); dpreset=stoi(t[3]); dbank=stoi(t[4]); break;
default: break;
}
addMapping(fontIndex, spreset, sbank, dpreset, dbank, dbanklsb);
}

int SoundfontMapping::findMapping (int fontIndex, int preset) {
auto spreset = LOWORD(preset), sbank = HIWORD(preset);
for (auto& m: mappings) {
if (m.font != fonts[fontIndex].font) continue;
if (sbank==m.sbank && spreset==m.spreset) return MKPRESET(m.dpreset, m.dbanklsb, m.dbank);
else if (m.spreset==-1 && m.sbank==sbank) return MKPRESET(spreset, m.dbanklsb, m.dbank);
else if (m.spreset==-1 && m.sbank==-1) return MKPRESET(spreset, m.dbanklsb, sbank+m.dbank);
}
return -1;
}

void SoundfontMapping::loadPresetNames () {
presets.clear();
for (int fontIndex=0, nFonts=fonts.size(); fontIndex<nFonts; fontIndex++) {
auto font = fonts[fontIndex].font;
BASS_MIDI_FONTINFO info;
if (!BASS_MIDI_FontGetInfo(font, &info)) continue;
DWORD lst[info.presets];
if (!BASS_MIDI_FontGetPresets(font, lst)) continue;
for (DWORD i=0; i<info.presets; i++) {
int preset = LOWORD(lst[i]), bank = HIWORD(lst[i]);
const char* name = BASS_MIDI_FontGetPreset(font, preset, bank);
if (!name || !*name) continue;
int dst = findMapping(fontIndex, lst[i]);
presets.emplace_back( MKPRESET(preset, fontIndex, bank), dst, name);
}}
std::stable_sort(presets.begin(), presets.end(), [&](auto&a, auto&b){
int ap = a.dval&0x7F, abl = (a.dval>>16)&0x7F, abh = (a.dval>>23)&0xFF;
int bp = b.dval&0x7F, bbl = (b.dval>>16)&0x7F, bbh = (b.dval>>23)&0xFF;
return abl<bbl
|| (abl==bbl && abh<bbh)
|| (abh==bbh && abl==bbl && ap<bp);
});
}
