#ifndef ___ENCODER0_1____
#define ___ENCODER0_1____
#ifdef __WIN32
#define UNICODE
#include<winsock2.h>
#endif
#include<string>
#include<memory>
#include<vector>
#include "../common/bass.h"
#include "../common/bassenc.h"

void encAddAll ();

struct Encoder {
std::string name, extension, mimetype;
Encoder (const std::string& nam, const std::string& ext, const std::string& mt): name(nam), extension(ext), mimetype(mt)  {}
virtual DWORD startEncoderFile (DWORD input, const std::string& file) { return 0; }
virtual bool saveFile (struct MidiFile& input, const std::string& file) { return false; }
virtual bool isUsingSaveFile () { return false; }
virtual bool hasFormatDialog () { return false; }
virtual bool showFormatDialog (struct wxWindow* parent) { return false; }

static std::vector<std::shared_ptr<Encoder>> encoders;
};

#endif
