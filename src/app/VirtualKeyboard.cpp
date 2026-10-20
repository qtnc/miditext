#include "../common/wxUtils.hpp"
#include "App.hpp"
#include "../common/bass.h"
#include "../common/bass_fx.h"
#include "../common/bassmidi.h"
#include<unordered_map>

static std::unordered_map<int,int> SC_TO_KEYS = {
{ 0x56, 48 },
{ 0x2C, 50 },
{ 0x2D, 52 },
{ 0x2E, 53 },
{ 0x2F, 55 },
{ 0x30, 57 },
{ 0x31, 59 },
{ 0x32, 60 },
{ 0x33, 62 },
{ 0x34, 64 },
{ 0x35, 65 },
{ 0x1E, 49 },
{ 0x1F, 51 },
{ 0x20, -1 }, // unassigned
{ 0x21, 54 },
{ 0x22, 56 },
{ 0x23, 58 },
{ 0x24, -1 }, // unassigned
{ 0x25, 61 },
{ 0x26, 63 },
{ 0x27, -1 }, // unassigned
{ 0x28, 66 },
{ 0x2B, 68 },
{ 0x10, 60 },
{ 0x11, 62 },
{ 0x12, 64 },
{ 0x13, 65 },
{ 0x14, 67 },
{ 0x15, 69 },
{ 0x16, 71 },
{ 0x17, 72 },
{ 0x18, 74 },
{ 0x19, 76 },
{ 0x1A, 77 },
{ 0x1B, 79 },
{ 0x29, 58 },
{ 0x02, -1 }, // unassigned
{ 0x03, 61 },
{ 0x04, 63 },
{ 0x05, -1 }, // unassigned
{ 0x06, 66 },
{ 0x07, 68 },
{ 0x08, 70 },
{ 0x09, -1 }, // unassigned
{ 0x0A, 73 },
{ 0x0B, 75 },
{ 0x0C, -1 }, // unassigned
{ 0x0D, 78 }
};

static int octave = 0;

static bool pressNote (DWORD stream, int sc, bool down) {
int note = SC_TO_KEYS[sc];
if (note<=0) return false;
BASS_MIDI_StreamEvent(stream, 0, MIDI_EVENT_NOTE, MAKEWORD(note + 12*octave, down? 127 : 0));
return true;
}

void App::virtualKeyboardChangeInstrument (int banklsb, int bankmsb, int program) {
unsigned char data[] = { 0xB0, 0, bankmsb, 32, banklsb, 0xC0, program };
BASS_MIDI_StreamEvents(vkbStream, BASS_MIDI_EVENTS_RAW, data, sizeof(data));
}

bool App::virtualKeyboardKeyDown (wxKeyEvent& e) {
if (e.IsAutoRepeat()) return true;
int k = e.GetKeyCode(), sc = (e.GetRawKeyFlags()>>16)&0xFF;
if (pressNote(vkbStream, sc, true)) return true;
switch(k){
case WXK_UP: 
octave = std::max(-4, std::min(octave +1, 4));
break;
case WXK_DOWN: 
octave = std::max(-4, std::min(octave -1, 4));
break;
}
return false;
}

bool App::virtualKeyboardKeyUp (wxKeyEvent& e) {
int k = e.GetKeyCode(), sc = (e.GetRawKeyFlags()>>16)&0xFF;
if (pressNote(vkbStream, sc, false)) return true;
return false;
}
