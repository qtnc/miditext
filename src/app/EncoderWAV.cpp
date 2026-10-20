#ifdef __WIN32
#define UNICODE
#include<winsock2.h>
#endif
#include "Encoder.hpp"
#include "../common/bass.h"
#include "../common/bassenc.h"
#include "../common/wxUtils.hpp"
#include "../app/App.hpp"
#include<fmt/format.h>
using namespace std;
using fmt::format;

struct FormatOptionsDlgWAV: wxDialog {
static const DWORD FPFLAGS[6];

wxComboBox *cbFormat;

FormatOptionsDlgWAV (App& app, wxWindow* parent):
wxDialog(parent, -1, U(translate("FormatOptionsDlg"))) 
{
wxString sFormats[6] = { U(translate("WAVFPAuto")), U(translate("WAV8bits")), U(translate("WAV16bits")), U(translate("WAV24bits")), U(translate("WAV32bits")), U(translate("WAVFloat"))  };
auto lblFormat = new wxStaticText(this, wxID_ANY, U(translate("WAVFormat")) );
cbFormat = new wxComboBox(this, 501, wxEmptyString, wxDefaultPosition, wxDefaultSize, 6, sFormats, wxCB_DROPDOWN | wxCB_READONLY);

auto sizer = new wxFlexGridSizer(2, 0, 0);
sizer->Add(lblFormat);
sizer->Add(cbFormat);

auto dlgSizer = new wxBoxSizer(wxVERTICAL);
dlgSizer->Add(sizer, 1, wxEXPAND);
auto btnSizer = new wxStdDialogButtonSizer();
btnSizer->AddButton(new wxButton(this, wxID_OK));
btnSizer->AddButton(new wxButton(this, wxID_CANCEL));
btnSizer->Realize();
dlgSizer->Add(btnSizer, 0, wxEXPAND);
SetSizerAndFit(dlgSizer);

cbFormat->SetFocus();
}

void fillTo (DWORD& fpflags) {
fpflags = FPFLAGS[cbFormat->GetSelection()];
}

void fillFrom (DWORD fpflags) {
int fIndex = std::find(FPFLAGS, FPFLAGS+6, fpflags) -FPFLAGS;
cbFormat->SetSelection(fIndex);
}

};
const DWORD FormatOptionsDlgWAV::FPFLAGS[6] = { BASS_ENCODE_FP_AUTO, BASS_ENCODE_FP_8BIT, BASS_ENCODE_FP_16BIT, BASS_ENCODE_FP_24BIT, BASS_ENCODE_FP_32BIT, 0 };

struct EncoderWAV: Encoder {
DWORD fpflags;

EncoderWAV (): 
Encoder("Wave", "wav", "audio/wav"),
fpflags(BASS_ENCODE_FP_AUTO)
{}

inline DWORD getFlags () {
return BASS_ENCODE_PCM | BASS_ENCODE_RF64 | BASS_ENCODE_DITHER | BASS_ENCODE_AUTOFREE | fpflags;
}

virtual DWORD startEncoderFile (DWORD input, const std::string& file) final override {
return BASS_Encode_Start(input, const_cast<char*>(file.c_str()), getFlags(), nullptr, nullptr);
}

virtual bool hasFormatDialog () final override { return true; }

virtual bool showFormatDialog (wxWindow* parent) final override {
FormatOptionsDlgWAV fod(wxGetApp(), parent);
fod.fillFrom(fpflags);
if (wxID_OK==fod.ShowModal()) {
fod.fillTo(fpflags);
return true;
}
return false;
}

};

void encAddWAV () {
Encoder::encoders.push_back(make_shared<EncoderWAV>());
}
