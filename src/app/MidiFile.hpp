#include<string>
#include<vector>
#include<iosfwd>
#include<stdexcept>
#include<boost/shared_array.hpp>
using boost::shared_array;

struct MidiEvent {
int tick, track, status, data1, data2;
shared_array<unsigned char> data;

std::string getDataAsString ();
void setDataAsString (const std::string&);
int getDataAsInt24 ();
void setDataAsInt24 (int);
void setData (const void*  a, int l, int d2=0);
inline MidiEvent (int t, int r, int s, int d1=0, int d2=0): tick(t), track(r), status(s), data1(d1), data2(d2), data(nullptr) {}
inline MidiEvent (int t, int r, int s, const shared_array<unsigned char>& a, int l, int d2=0): tick(t), track(r), status(s), data1(l), data2(d2), data(a) {}
inline MidiEvent (int t, int r, int s, const void* d, int l, int d2=0): tick(t), track(r), status(s), data1(l), data2(d2), data(nullptr) { setData(d,l); }
inline MidiEvent (int t, int r, int s, const std::string& str, int d2=0): tick(t), track(r), status(s), data1(0), data2(d2), data(nullptr) { setDataAsString(str); }
};

struct MidiFile {
static const int OPTIMIZE=1, INCLUDE_SOURCE=2;
std::vector<MidiEvent> events;
int ppq=480, nTracks=1, bpm=120;
bool recordActive=false;
double recordOffsetTime = -1;

bool open (std::istream& in);
bool save (std::ostream& out, int flags= INCLUDE_SOURCE);
std::string decompile (int rounding=1);
std::string getCodeText ();
void setCodeText (const std::string&);
void sortEvents ();
void record (double time, const unsigned char* data, int length);
void startRecord();
void stopRecord();
};//MidiFile

struct syntax_error: std::runtime_error {
const int offset;
syntax_error (const char* what_, int of): runtime_error(what_), offset(of) {}
};

template<class T> inline shared_array<T> make_shared_array (size_t n) {
return shared_array<T>(new T[n]);
}
