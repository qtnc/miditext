#include "Encoder.hpp"
#include "MidiFile.hpp"
#include "../common/wxUtils.hpp"
#include "App.hpp"
#include<fstream>
using namespace std;

struct FormatOptionsDlgMIDI: wxDialog {
wxCheckBox *cbOptimize, *cbStrip;

FormatOptionsDlgMIDI (App& app, wxWindow* parent):
wxDialog(parent, -1, U(translate("FormatOptionsDlg"))) 
{
auto lblEmpty1 = new wxStaticText(this, wxID_ANY, wxEmptyString);
cbOptimize = new wxCheckBox(this, 501, U(translate("cbMidiOptimize")), wxDefaultPosition, wxDefaultSize);
auto lblEmpty2 = new wxStaticText(this, wxID_ANY, wxEmptyString);
cbStrip = new wxCheckBox(this, 502, U(translate("cbMidiStrip")), wxDefaultPosition, wxDefaultSize);

auto sizer = new wxFlexGridSizer(2, 0, 0);
sizer->Add(lblEmpty1);
sizer->Add(cbOptimize);
sizer->Add(lblEmpty2);
sizer->Add(cbStrip);

auto dlgSizer = new wxBoxSizer(wxVERTICAL);
dlgSizer->Add(sizer, 1, wxEXPAND);
auto btnSizer = new wxStdDialogButtonSizer();
btnSizer->AddButton(new wxButton(this, wxID_OK));
btnSizer->AddButton(new wxButton(this, wxID_CANCEL));
btnSizer->Realize();
dlgSizer->Add(btnSizer, 0, wxEXPAND);
SetSizerAndFit(dlgSizer);
cbOptimize->SetFocus();
}
};

struct EncoderMIDI: Encoder {
bool optimize, strip;

EncoderMIDI (): 
Encoder("MIDI", "mid", "audio/mid"),
optimize(true), strip(false)
{}

virtual bool isUsingSaveFile () override { return true; }

virtual bool saveFile (MidiFile& mf, const std::string& file) final override {
int flags = (optimize? MidiFile::OPTIMIZE : 0) | (strip? 0 : MidiFile::INCLUDE_SOURCE);
strip=false;
ofstream out(file, ios::binary);
return mf.save(out, flags);
}

virtual bool hasFormatDialog () final override { return true; }

virtual bool showFormatDialog (wxWindow* parent) final override {
FormatOptionsDlgMIDI fod(wxGetApp(), parent);
fod.cbOptimize->SetValue(optimize);
fod.cbStrip->SetValue(strip);
if (wxID_OK==fod.ShowModal()) {
optimize = fod.cbOptimize->GetValue();
strip = fod.cbStrip->GetValue();
return true;
}
return false;
}

};

void encAddMIDI () {
Encoder::encoders.push_back(make_shared<EncoderMIDI>());
}
