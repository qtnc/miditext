#include "Encoder.hpp"
#include "../common/bass.h"
#include "../common/bassenc.h"
#include "../common/bassenc_flac.h"
using namespace std;

struct EncoderFlac: Encoder {

EncoderFlac (): 
Encoder("Free lossless audio codec (flac)", "flac", "audio/flac")
{}

string getOptions () {
return "--best";
}

virtual DWORD startEncoderFile (DWORD input, const std::string& file) final override {
string options = getOptions();
return BASS_Encode_FLAC_StartFile(input, const_cast<char*>(options.c_str()), BASS_ENCODE_AUTOFREE, const_cast<char*>(file.c_str()) );
}

};

void encAddFlac () {
Encoder::encoders.push_back(make_shared<EncoderFlac>());
}
