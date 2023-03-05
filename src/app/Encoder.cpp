#include "Encoder.hpp"
#include "../common/println.hpp"
using namespace std;

extern void encAddMIDI ();
extern void encAddMP3 ();
extern void encAddOGG ();
extern void encAddFlac ();
extern void encAddOpus ();

vector<shared_ptr<Encoder>> Encoder::encoders;

void encAddAll () {
encAddMIDI();
encAddMP3();
encAddOGG();
encAddOpus();
encAddFlac();
}

