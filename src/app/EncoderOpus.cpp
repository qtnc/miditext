#include "Encoder.hpp"
#include "../common/bass.h"
#include "../common/bassenc.h"
#include "../common/bassenc_opus.h"
#include "../common/wxUtils.hpp"
#include "../common/println.hpp"
#include "App.hpp"
using namespace std;

struct FormatOptionsDlgOpus: wxDialog {
static vector<int> bitrates;

wxComboBox *cbBitrate, *cbComplexity;
FormatOptionsDlgOpus (App& app, wxWindow* parent):
wxDialog(parent, -1, U(translate("FormatOptionsDlg"))) 
{
wxString sBitrates[bitrates.size()], sComplexities[11];
for (int i=0; i<11; i++) sComplexities[i] = U(format("{}", i));
for (int i=0, n=bitrates.size(); i<n; i++) sBitrates[i] = U(format("{} kbps", bitrates[i]));
auto lblBitrate = new wxStaticText(this, wxID_ANY, U(translate("lblBitrate")) );
cbBitrate = new wxComboBox(this, 504, wxEmptyString, wxDefaultPosition, wxDefaultSize, bitrates.size(), sBitrates, wxCB_DROPDOWN | wxCB_READONLY);
auto lblComplexity = new wxStaticText(this, wxID_ANY, U(translate("Complexity")) );
cbComplexity = new wxComboBox(this, 503, wxEmptyString, wxDefaultPosition, wxDefaultSize, 11, sComplexities, wxCB_DROPDOWN | wxCB_READONLY);

auto sizer = new wxFlexGridSizer(2, 0, 0);
sizer->Add(lblBitrate);
sizer->Add(cbBitrate);
sizer->Add(lblComplexity);
sizer->Add(cbComplexity);

auto dlgSizer = new wxBoxSizer(wxVERTICAL);
dlgSizer->Add(sizer, 1, wxEXPAND);
auto btnSizer = new wxStdDialogButtonSizer();
btnSizer->AddButton(new wxButton(this, wxID_OK));
btnSizer->AddButton(new wxButton(this, wxID_CANCEL));
btnSizer->Realize();
dlgSizer->Add(btnSizer, 0, wxEXPAND);
SetSizerAndFit(dlgSizer);
cbBitrate->SetFocus();
}
};
vector<int> FormatOptionsDlgOpus::bitrates = { 6, 8, 12, 16, 20, 24, 28, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 192, 224, 256 };

struct EncoderOpus: Encoder {
int complexity, bitrate;

EncoderOpus (): 
Encoder("Opus", "opus", "audio/opus"),
complexity(10), bitrate(144)
{}

string getOptions () {
return format("--comp {} --vbr --bitrate {}", complexity, bitrate);
}

virtual DWORD startEncoderFile (DWORD input, const std::string& file) final override {
string options = getOptions();
return BASS_Encode_OPUS_StartFile(input, const_cast<char*>(options.c_str()), BASS_ENCODE_AUTOFREE, const_cast<char*>(file.c_str()) );
}

virtual bool hasFormatDialog () final override { return true; }

virtual bool showFormatDialog (wxWindow* parent) final override {
FormatOptionsDlgOpus fod(wxGetApp(), parent);
int bIndex = std::find(FormatOptionsDlgOpus::bitrates.begin(), FormatOptionsDlgOpus::bitrates.end(), bitrate) -FormatOptionsDlgOpus::bitrates.begin();
fod.cbBitrate->SetSelection(bIndex);
fod.cbComplexity->SetSelection(complexity);
if (wxID_OK==fod.ShowModal()) {
bitrate = FormatOptionsDlgOpus::bitrates[fod.cbBitrate->GetSelection()];
complexity = fod.cbComplexity->GetSelection();
return true;
}
return false;
}

};

void encAddOpus () {
Encoder::encoders.push_back(make_shared<EncoderOpus>());
}
