#ifndef _____SF2MAPPING_HPP_0
#define _____SF2MAPPING_HPP_0
#ifdef __WIN32
#define UNICODE
#include<winsock2.h>
#endif
#include "../common/bass.h"
#include "../common/bassmidi.h"
#include<vector>
#include<string>

struct SoundfontEntry {
HSOUNDFONT font;
std::string file;
std::string name;
DWORD flags;
float volume;

SoundfontEntry (const std::string& file1, const std::string& name1, DWORD flags1=0, float volume1=1.0f): name(name1), file(file1), volume(volume1), flags(flags1), font(0) {}
bool load ();
bool unload ();
inline operator bool () { return !!font; }
};

struct SoundfontPreset {
int sval, dval;
std::string name;
SoundfontPreset (int s0, int d0, const std::string& name0): sval(s0), dval(d0), name(name0) {}
};

struct SoundfontMapping {
std::vector<SoundfontEntry> fonts;
std::vector<BASS_MIDI_FONTEX> mappings;
std::vector<SoundfontPreset> presets;

int findFontByName (const std::string& name);
void addFont (const std::string& file, const std::string& name, DWORD flags=0, float volume=1.0f);
void addFontLine (const std::string& s);
void addMapping (int fontIndex, int sbank, int spreset, int dbank, int dpreset, int dbanklsb);
void addMapping (const std::string& s);
int findMapping (int fontIndex, int preset);
void loadPresetNames ();
};

#endif