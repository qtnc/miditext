#include "InstrumentDialog.hpp"
#include "App.hpp"
#include "MainWindow.hpp"
#include "MidiFile.hpp"
#include "../common/stringUtils.hpp"
#include <wx/listctrl.h>
#include "../common/bass.h"
#include "../common/bassmidi.h"
using namespace std;

extern DWORD CompileMidiAsStream (const string& code, DWORD flags=0, int* mark1=0, int* mark2=0);

std::string 
InstrumentDialog::testCode = "c4",
InstrumentDialog::testCodeSnd = "L4 c<c<c>c>c>c>c>c",
InstrumentDialog::testCodeDrums = "o3\n(c/g_/d/a_/)4\nR3 > (c/4)4 (B/4)4 (A/4)4 d_2";

InstrumentDialog::InstrumentDialog (App& app, wxWindow* parent, const CloseCallback& ccb):
wxDialog(parent, wxID_ANY, U(translate("SelectInstrumentDlg")) ),
input(), lastInputTime(0), app(app), closeCb(ccb) {
auto lblFilter = new wxStaticText(this, wxID_ANY, U(translate("QuickFilter")), wxPoint(-2, -2), wxSize(1, 1) );
tfFilter = new wxTextCtrl(this, 500, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
auto lblInstlist = new wxStaticText(this, wxID_ANY, U(translate("InstrumentList")), wxPoint(-2, -2), wxSize(1, 1) );
lcList = new wxListView(this, 501, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
auto lblTestCode = new wxStaticText(this, wxID_ANY, U(translate("TestCode")), wxDefaultPosition, wxDefaultSize );
tfTestCode = new wxTextCtrl(this, 600, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_MULTILINE | wxVSCROLL  | wxTE_RICH2);

tfTestCode->Bind(wxEVT_TEXT, [&](auto&e){ onTestCodeChange(); });
tfFilter->Bind(wxEVT_TEXT, [&](auto&e){ onFilterChange(); });
tfFilter->Bind(wxEVT_TEXT_ENTER, [&](auto&e){ onFilterEnter(); });
lcList->Bind(wxEVT_LIST_ITEM_SELECTED, [&](auto&e){ onSelectionChange(); });
lcList->Bind(wxEVT_LIST_ITEM_ACTIVATED, [&](auto&e){ onListEnter(); });
lcList->Bind(wxEVT_CHAR, &InstrumentDialog::OnListKeyChar, this);
lcList->Bind(wxEVT_KILL_FOCUS, [&](auto&e){ app.stopPlayback(); e.Skip(); });
Bind(wxEVT_ACTIVATE, [&](auto&e){ updateList(); });
Bind(wxEVT_BUTTON, [&](auto&e){ onListEnter(); }, wxID_OK);
Bind(wxEVT_BUTTON, [&](auto&e){ app.win->clearTimeout(); e.Skip();  }, wxID_CANCEL);

auto sizer = new wxBoxSizer(wxVERTICAL);
sizer->Add(tfFilter, 0, wxEXPAND);
sizer->Add(lcList, 1, wxEXPAND);
auto tcSizer = new wxBoxSizer(wxHORIZONTAL);
tcSizer->Add(lblTestCode, 0, 0);
tcSizer->Add(tfTestCode, 1, 0);
sizer->Add(tcSizer, 0, wxEXPAND);
auto btnSizer = new wxStdDialogButtonSizer();
btnSizer->AddButton(new wxButton(this, wxID_OK));
btnSizer->AddButton(new wxButton(this, wxID_CANCEL));
btnSizer->Realize();
sizer->Add(btnSizer, 0, wxEXPAND);
SetSizerAndFit(sizer);
lcList->SetFocus();
}

string& InstrumentDialog::getTestCode (int n) {
int preset = n&0x7F, bank = (n>>23)&0xFF;
if (bank>=127) return testCodeDrums;
else if (preset>=113) return testCodeSnd;
else return testCode;
}

void InstrumentDialog::onFilterChange () {
app.win->setTimeout(1000, [&](){ updateList(); });
}

void InstrumentDialog::onFilterEnter () {
app.win->setTimeout(100, [&](){ updateList(); lcList->SetFocus(); });
}

void InstrumentDialog::onListEnter () {
app.win->clearTimeout();
auto p = getSelectedPreset();
if (closeCb) closeCb(p);
EndModal(wxID_OK);
}

void InstrumentDialog::onTestCodeChange () {
auto p = getSelectedPreset();
if (!p) return;
getTestCode(p->sval) = U(tfTestCode->GetValue());
}

void InstrumentDialog::onSelectionChange () {
app.stopPlayback();
auto p = getSelectedPreset();
auto& app = this->app;
if (!p) return;
string code = getTestCode(p->sval);
tfTestCode->SetValue(U(code));
app.win->setTimeout(600, [=,&app](){ 
try {
app.curStream = CompileMidiAsStream(code, BASS_STREAM_AUTOFREE | BASS_SAMPLE_FLOAT);
} catch (syntax_error& e) {
string msg = e.what();
wxMessageBox(U(msg), U(translate("Error")), wxICON_ERROR | wxOK);
//textArea->SetInsertionPoint(e.offset);
return;
}
if (!app.curStream) return;
int preset = p->sval&0x7F, bank = (p->sval>>23)&0xFF, fontIndex = (p->sval>>16)&0x7F;
BASS_MIDI_FONTEX sfconf = { app.sfmap.fonts[fontIndex].font, preset, bank, 0, 0, 0 };
BASS_MIDI_StreamSetFonts(app.curStream, &sfconf, 1 | BASS_MIDI_FONT_EX);
app.pausePlayback();
});
}

void InstrumentDialog::OnListKeyChar (wxKeyEvent& e) {
wxChar ch[2] = {0,0};
ch[0] = e.GetUnicodeKey();
if (ch[0]<32) { e.Skip(); return; }
long time = e.GetTimestamp();
if (time-lastInputTime>600) input.clear();
lastInputTime=time;
input += U(ch);
int selection = lcList->GetFirstSelected(), len = lcList->GetItemCount();
for (int i=input.size()>1?0:1; i<len; i++) {
int indexInList = (selection + i)%len;
auto text = U(lcList->GetItemText(indexInList, 1));
if (istarts_with(text, input)) {
selection=indexInList;
break;
}}
lcList->Select(selection);
lcList->Focus(selection);
}

SoundfontPreset* InstrumentDialog::getSelectedPreset () {
int i = lcList->GetFirstSelected();
if (i<0) return nullptr;
auto iptr = lcList->GetItemData(i);
return reinterpret_cast<SoundfontPreset*>(iptr);
}

void InstrumentDialog::updateList () {
if (app.sfmap.presets.empty()) app.sfmap.loadPresetNames();
auto filter = U(tfFilter->GetValue());
int iFilter = !filter.empty() && string::npos==filter.find_first_not_of("0123456789")? stoi(filter) : -1;
int selection = lcList->GetFirstSelected(), newSelection = wxNOT_FOUND;
if (selection>=0) selection = lcList->GetItemData(selection);
lcList->Freeze();
lcList->ClearAll();
lcList->AppendColumn(wxEmptyString);
lcList->AppendColumn(U(translate("hdrName")));
lcList->AppendColumn(U(translate("hdrBank")), wxLIST_FORMAT_RIGHT);
lcList->AppendColumn(U(translate("hdrBankLSB")), wxLIST_FORMAT_RIGHT);
lcList->AppendColumn(U(translate("hdrPreset")), wxLIST_FORMAT_RIGHT);
int itemIndex = -1;
for (auto& p: app.sfmap.presets) {
if (!filter.empty() && iFilter<0 && !icontains(p.name, filter)) continue;
int preset = p.dval&0x7F, banklsb = (p.dval>>16)&0x7F, bank = (p.dval>>23)&0xFF;
if (iFilter>=0 && preset!=iFilter && bank!=iFilter && banklsb!=iFilter) continue;
auto iptr = reinterpret_cast<intptr_t>(&p);
lcList->InsertItem(++itemIndex, wxEmptyString);
lcList->SetItem(itemIndex, 1, U(p.name + ". "));
lcList->SetItem(itemIndex, 2, U(to_string(bank) + ", "));
lcList->SetItem(itemIndex, 3, U(to_string(banklsb) + ", "));
lcList->SetItem(itemIndex, 4, U(to_string(preset)));
lcList->SetItemData(itemIndex, iptr);
if (iptr==selection) newSelection = itemIndex;
}
lcList->Thaw();
lcList->Select(newSelection);
lcList->Focus(newSelection);
}



