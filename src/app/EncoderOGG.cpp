#include "Encoder.hpp"
#include "../common/bass.h"
#include "../common/bassenc.h"
#include "../common/bassenc_ogg.h"
#include "../common/wxUtils.hpp"
#include "App.hpp"
#include "../common/println.hpp"
using namespace std;

struct FormatOptionsDlgOGG: wxDialog {
wxComboBox* cbQuality;
FormatOptionsDlgOGG (App& app, wxWindow* parent):
wxDialog(parent, -1, U(translate("FormatOptionsDlg"))) 
{
int kbps[] = { 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 500 };
wxString sQualities[11];
for (int i=0; i<11; i++) sQualities[i] = U(format("Q{}, %d kbps", i, kbps[i]));
auto lblQuality = new wxStaticText(this, wxID_ANY, U(translate("Quality")) );
cbQuality = new wxComboBox(this, 504, wxEmptyString, wxDefaultPosition, wxDefaultSize, 11, sQualities, wxCB_DROPDOWN | wxCB_READONLY);

auto sizer = new wxFlexGridSizer(2, 0, 0);
sizer->Add(lblQuality);
sizer->Add(cbQuality);

auto dlgSizer = new wxBoxSizer(wxVERTICAL);
dlgSizer->Add(sizer, 1, wxEXPAND);
auto btnSizer = new wxStdDialogButtonSizer();
btnSizer->AddButton(new wxButton(this, wxID_OK));
btnSizer->AddButton(new wxButton(this, wxID_CANCEL));
btnSizer->Realize();
dlgSizer->Add(btnSizer, 0, wxEXPAND);
SetSizerAndFit(dlgSizer);
cbQuality->SetFocus();
}
};

struct EncoderOGG: Encoder {
int quality;

EncoderOGG (): 
Encoder("OGG Vorbis", "ogg", BASS_ENCODE_TYPE_OGG),
quality(4)
{}

string getOptions () {
return format("-q {}", quality);
}

virtual DWORD startEncoderFile (DWORD input, const std::string& file) final override {
string options = getOptions();
return BASS_Encode_OGG_StartFile(input, const_cast<char*>(options.c_str()), BASS_ENCODE_AUTOFREE, const_cast<char*>(file.c_str()) );
}

virtual bool hasFormatDialog () final override { return true; }

virtual bool showFormatDialog (wxWindow* parent) final override {
FormatOptionsDlgOGG fod(wxGetApp(), parent);
fod.cbQuality->SetSelection(quality);
if (wxID_OK==fod.ShowModal()) {
quality = fod.cbQuality->GetSelection();
return true;
}
return false;
}

};

void encAddOGG () {
Encoder::encoders.push_back(make_shared<EncoderOGG>());
}
