#include "../common/println.hpp"
#include "../common/stringUtils.hpp"
#include "MainWindow.hpp"
#include "InstrumentDialog.hpp"
#include "MidiFile.hpp"
#include "App.hpp"
#include "WorkerThread.hpp"
#include "Encoder.hpp"
//#include "UniversalSpeech.h"
#include "../common/wxUtils.hpp"
#include <wx/listctrl.h>
#include <wx/thread.h>
#include <wx/progdlg.h>
#include <wx/aboutdlg.h>
#include <wx/accel.h>
#include <wx/settings.h>
#include <wx/gbsizer.h>
#include <wx/timer.h>
#include <wx/scrolbar.h>
#include <wx/slider.h>
#include <wx/log.h>
#include <wx/regex.h>
#include "../common/bass.h"
#include "../common/bass_fx.h"
#include "../common/bassmidi.h"
#include<cmath>
using namespace std;

extern void encAddAll ();
extern DWORD CompileMidiAsStream (const string& code, DWORD flags=0, int* mark1=0, int* mark2=0);
void MTTranspose (std::string& text, int count);

MainWindow::MainWindow (App& app):
wxFrame(nullptr, wxID_ANY,
U(APP_DISPLAY_NAME),
wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE),
app(app)
{
println("Initializing main window GUI...");
auto panel = new wxPanel(this);
textArea = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH2 | wxTE_NOHIDESEL | wxVSCROLL | wxHSCROLL | wxALWAYS_SHOW_SB | wxCLIP_CHILDREN);
textArea->SetMaxLength(16777215);
status = CreateStatusBar();

auto panelSizer = new wxBoxSizer(wxVERTICAL);
panelSizer->Add(textArea, 1, wxEXPAND);
panel->SetSizer(panelSizer);
panelSizer = new wxBoxSizer(wxVERTICAL);
panelSizer->Add(panel, 1, wxEXPAND);

auto menubar = new wxMenuBar();
auto fileMenu = new wxMenu();
auto editMenu = new wxMenu();
auto playMenu = new wxMenu();
auto recMenu = new wxMenu();
auto helpMenu = new wxMenu();
fileMenu->Append(IDM_OPEN, U(translate("Open")));
fileMenu->Append(IDM_SAVE, U(translate("Save")));
fileMenu->Append(IDM_SAVE_AS, U(translate("SaveAs")));
fileMenu->Append(wxID_EXIT, U(translate("Exit")));
editMenu->Append(wxID_COPY, U(translate("Copy")));
editMenu->Append(wxID_CUT, U(translate("Cut")));
editMenu->Append(wxID_PASTE, U(translate("Paste")));
editMenu->Append(IDM_TRANSPOSE_UP, U(translate("TransposeUp")));
editMenu->Append(IDM_TRANSPOSE_DOWN, U(translate("TransposeDown")));
editMenu->Append(IDM_FIND, U(translate("Find")));
editMenu->Append(IDM_FIND_REPLACE, U(translate("FindReplace")));
editMenu->Append(IDM_FIND_NEXT, U(translate("FindNext")));
editMenu->Append(IDM_FIND_PREV, U(translate("FindPrev")));
playMenu->Append(IDM_PLAY_PAUSE, U(translate("PlayPause")));
playMenu->Append(IDM_PLAY_FROM_CURSOR, U(translate("PlayFromCursor")));
playMenu->Append(IDM_SEEK_BKWD, U(translate("SeekBkwd")));
playMenu->Append(IDM_SEEK_FWD, U(translate("SeekFwd")));
playMenu->Append(IDM_SEEK_BKWD2, U(translate("SeekBkwd2")));
playMenu->Append(IDM_SEEK_FWD2, U(translate("SeekFwd2")));
playMenu->Append(IDM_VOLUME_DOWN, U(translate("VolumeDown")));
playMenu->Append(IDM_VOLUME_UP, U(translate("VolumeUp")));
playMenu->AppendCheckItem(IDM_LOOP, U(translate("PlayLoop")));
recMenu->Append(IDM_REC_START_STOP, U(translate("StartRec")));
recMenu->Append(IDM_SELINST, U(translate("SelectInstrument")));
helpMenu->Append(wxID_ABOUT, U(translate("About")));
menubar->Append(fileMenu, U(translate("FileMenu")));
menubar->Append(editMenu, U(translate("EditMenu")));
menubar->Append(playMenu, U(translate("PlayMenu")));
menubar->Append(recMenu, U(translate("RecMenu")));
menubar->Append(helpMenu, U(translate("HelpMenu")));
SetMenuBar(menubar);

refreshTimer = new wxTimer(this, 99);
otherTimer = new wxTimer(this, 98);
refreshTimer->Start(500);

Bind(wxEVT_TIMER, &MainWindow::OnTimer, this);
Bind(wxEVT_MENU, &MainWindow::OnOpenFileDlg, this, IDM_OPEN);
Bind(wxEVT_MENU, &MainWindow::OnSaveFileDlg, this, IDM_SAVE_AS);
Bind(wxEVT_MENU, &MainWindow::OnSaveFile, this, IDM_SAVE);
Bind(wxEVT_MENU, &MainWindow::OnPlayPause, this, IDM_PLAY_PAUSE);
Bind(wxEVT_MENU, &MainWindow::OnLoopChange, this, IDM_LOOP);
Bind(wxEVT_MENU, &MainWindow::OnPlayFromCursor, this, IDM_PLAY_FROM_CURSOR);
Bind(wxEVT_MENU, [&](auto&e){ app.seekPlayback(5); }, IDM_SEEK_FWD);
Bind(wxEVT_MENU, [&](auto&e){ app.seekPlayback(30); }, IDM_SEEK_FWD2);
Bind(wxEVT_MENU, [&](auto&e){ app.seekPlayback(-5); }, IDM_SEEK_BKWD);
Bind(wxEVT_MENU, [&](auto&e){ app.seekPlayback(-30); }, IDM_SEEK_BKWD2);
Bind(wxEVT_MENU, [&](auto&e){ app.changeVol(-0.02f); }, IDM_VOLUME_DOWN);
Bind(wxEVT_MENU, [&](auto&e){ app.changeVol(0.02f); }, IDM_VOLUME_UP);
Bind(wxEVT_MENU, [this](auto&e){ TransposeSelection(1); }, IDM_TRANSPOSE_UP);
Bind(wxEVT_MENU, [this](auto&e){ TransposeSelection(-1); }, IDM_TRANSPOSE_DOWN);
Bind(wxEVT_MENU, [&](auto&e){ showInstrumentDialog(); }, IDM_SELINST);
Bind(wxEVT_MENU, &MainWindow::OnFind, this, IDM_FIND);
Bind(wxEVT_MENU, &MainWindow::OnFindNext, this, IDM_FIND_NEXT);
Bind(wxEVT_MENU, &MainWindow::OnFindPrev, this, IDM_FIND_PREV);
Bind(wxEVT_MENU, &MainWindow::OnFindReplace, this, IDM_FIND_REPLACE);
Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
Bind(wxEVT_CLOSE_WINDOW, &MainWindow::OnClose, this);
Bind(wxEVT_THREAD, &MainWindow::OnProgress, this);
textArea->SetFocus();
SetSizerAndFit(panelSizer);

vector<wxAcceleratorEntry> entries = {
/*{ wxACCEL_NORMAL, WXK_PAGEUP, ACTION_HISTORY_PREV  },
{ wxACCEL_NORMAL, WXK_PAGEDOWN, ACTION_HISTORY_NEXT },
{ wxACCEL_CTRL, WXK_PAGEUP, ACTION_HISTORY_FIRST },
{ wxACCEL_CTRL, WXK_PAGEDOWN, ACTION_HISTORY_LAST },
{ wxACCEL_SHIFT, WXK_PAGEUP, ACTION_HISTORY_FIRST },
{ wxACCEL_SHIFT, WXK_PAGEDOWN, ACTION_HISTORY_LAST },
{ wxACCEL_ALT, WXK_LEFT, ACTION_VIEW_PREV },
{ wxACCEL_ALT, WXK_RIGHT, ACTION_VIEW_NEXT },
{ wxACCEL_ALT, '0', ACTION_VIEW_GOTO+9 },
{ wxACCEL_ALT, '1', ACTION_VIEW_GOTO+0 },
{ wxACCEL_ALT, '2', ACTION_VIEW_GOTO+1 },
{ wxACCEL_ALT, '3', ACTION_VIEW_GOTO+2 },
{ wxACCEL_ALT, '4', ACTION_VIEW_GOTO+3 },
{ wxACCEL_ALT, '5', ACTION_VIEW_GOTO+4 },
{ wxACCEL_ALT, '6', ACTION_VIEW_GOTO+5 },
{ wxACCEL_ALT, '7', ACTION_VIEW_GOTO+6 },
{ wxACCEL_ALT, '8', ACTION_VIEW_GOTO+7 },
{ wxACCEL_ALT, '9', ACTION_VIEW_GOTO+8 },
{ wxACCEL_CTRL, WXK_SPACE, ACTION_COPY_CUR_HISTORY },
{ wxACCEL_CTRL, WXK_RETURN, ACTION_ACTIVATE_CURHISTORY__LINK },
{ wxACCEL_NORMAL, WXK_F6, ACTION_VOL_SELECT },
{ wxACCEL_NORMAL, WXK_F8, ACTION_VOL_PLUS },
{ wxACCEL_NORMAL, WXK_F7, ACTION_VOL_MINUS },
{ wxACCEL_CTRL | wxACCEL_SHIFT, WXK_DELETE, ACTION_DEBUG }*/
};
wxAcceleratorTable table(entries.size(), &entries[0]);
SetAcceleratorTable(table);
SetFocus();
textArea->SetFocus();
println("Initialized main window GUI");
}

int MainWindow::popupMenu (const vector<string>& items, int selection) {
wxMenu menu;
for (int i=0, n=items.size(); i<n; i++) menu.Append(i+1, U(items[i]), wxEmptyString, selection<0? wxITEM_NORMAL : wxITEM_RADIO);
if (selection>=0) menu.Check(selection+1, true);
int result = GetPopupMenuSelectionFromUser(menu);
return result==wxID_NONE? -1 : result -1;
}

void MainWindow::openProgress (const std::string& title, const std::string& message, int maxValue) {
RunEDT([=](){
progressCancelled=false;
progressDialog = new wxProgressDialog(U(title), U(message), maxValue, this, wxPD_CAN_ABORT | wxPD_AUTO_HIDE | wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);
});
}

void MainWindow::closeProgress () {
progressDialog->Destroy();
progressDialog=nullptr;
wxWakeUpIdle();
}

void MainWindow::updateProgress (int value) {
wxThreadEvent e(wxEVT_THREAD, 9999);
e.SetInt(value);
wxQueueEvent(this, e.Clone());
}

void MainWindow::setProgressMax (int value) {
wxThreadEvent e(wxEVT_THREAD, 9997);
e.SetInt(value);
wxQueueEvent(this, e.Clone());
}

void MainWindow::setProgressText (const std::string& msg) {
wxThreadEvent e(wxEVT_THREAD, 9998);
e.SetString(U(msg));
wxQueueEvent(this, e.Clone());
}

void MainWindow::setProgressTitle (const std::string& msg) {
wxThreadEvent e(wxEVT_THREAD, 9996);
e.SetString(U(msg));
wxQueueEvent(this, e.Clone());
}

void MainWindow::OnProgress (wxThreadEvent& e) {
switch(e.GetId()) {
case 9999: {
int value = e.GetInt();
if (progressDialog) progressCancelled = !progressDialog->Update(value);
}break;
case 9998: {
wxString value = e.GetString();
if (progressDialog) progressCancelled = !progressDialog->Update(progressDialog->GetValue(), value);
}break;
case 9997: {
int value = e.GetInt();
if (progressDialog) progressDialog->SetRange(value);
}break;
case 9996: {
wxString value = e.GetString();
if (progressDialog) progressDialog->SetTitle(value);
}break;
}}

bool MainWindow::isProgressCancelled () { return progressCancelled; }

void MainWindow::setTimeout (int ms, const function<void()>& func) {
timerFunc = func;
otherTimer->StartOnce(ms);
}

void MainWindow::clearTimeout () {
otherTimer->Stop();
}

void MainWindow::showAboutBox (wxWindow* parent) {
string name = APP_DISPLAY_NAME;
wxAboutDialogInfo info;
info.SetCopyright("Copyright (C) 2023, QuentinC");
info.SetName(name);
info.SetVersion(VERSION_STRING);
info.SetWebSite("https://quentinc.net/");
wxAboutBox(info, parent);
}

static void openFileUpdatePreview (wxFileDialog* fd) {
App& app = wxGetApp();
auto& curFile = app.curSelectedFile;
string newFile = UFN(fd->GetCurrentlySelectedFilename());
if (newFile.size() && newFile!=curFile) {
app.stopPlayback();
app.win->setTimeout(1000, [=,&app]()mutable{
app.startPlayback(newFile);
});
}}

static wxWindow* createOpenFilePanel (wxWindow* parent) {
App& app = wxGetApp();
wxWindow* panel = new wxPanel(parent);
auto btnPlay = new wxButton(panel, wxID_ANY, U(translate("PreviewPlay")) );
auto lblPosition = new wxStaticText(panel, wxID_ANY, U(translate("PreviewPosition")), wxPoint(-2, -2), wxSize(1, 1) );
app.win->slPreviewPosition = new wxSlider(panel, wxID_ANY, 0, 0, 60, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
auto lblVolume = new wxStaticText(panel, wxID_ANY, U(translate("PreviewVolume")), wxPoint(-2, -2), wxSize(1, 1) );
app.win->slPreviewVolume = new wxSlider(panel, wxID_ANY, app.streamVol*100, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
auto sizer = new wxBoxSizer(wxHORIZONTAL);
sizer->Add(btnPlay, 0, 0);
sizer->Add(app.win->slPreviewPosition, 1, 0);
sizer->Add(app.win->slPreviewVolume, 1, 0);
panel->SetSizerAndFit(sizer);
btnPlay->Bind(wxEVT_BUTTON, [&](auto&e){ app.pausePlayback(); });
app.win->slPreviewVolume->Bind(wxEVT_SCROLL_CHANGED, [&](auto& e){ app.changeVol( app.win->slPreviewVolume->GetValue() / 100.0f, true); });
app.win->slPreviewPosition->Bind(wxEVT_SCROLL_CHANGED, [&](auto& e){ app.seekPlayback( app.win->slPreviewPosition->GetValue(), true); });
panel->Bind(wxEVT_UPDATE_UI, [=](auto& e){  openFileUpdatePreview(wxStaticCast(parent, wxFileDialog)); });
return panel;
}

static wxWindow* createSaveFilePanel (wxWindow* parent) {
App& app = wxGetApp();
wxWindow* panel = new wxPanel(parent);
auto sizer = new wxBoxSizer(wxHORIZONTAL);
auto btnFormat = new wxButton(panel, wxID_ANY, U(translate("FormatOptionsBtn")));
sizer->Add(btnFormat);
panel->Bind(wxEVT_UPDATE_UI, [=](auto& e){ 
        wxFileDialog* fd = wxStaticCast(parent, wxFileDialog);
int filterIndex = fd->GetCurrentlySelectedFilterIndex();
bool enable = false;
if (filterIndex>=0 && filterIndex<Encoder::encoders.size()) {
auto& encoder = *Encoder::encoders[filterIndex];
enable = encoder.hasFormatDialog();
}
btnFormat->Enable(enable);
});
btnFormat->Bind(wxEVT_BUTTON, [=](auto& e){
        wxFileDialog* fd = wxStaticCast(parent, wxFileDialog);
int filterIndex = fd->GetCurrentlySelectedFilterIndex();
if (filterIndex<0 || filterIndex>=Encoder::encoders.size()) return;
auto& encoder = *Encoder::encoders[filterIndex];
if (encoder.hasFormatDialog()) encoder.showFormatDialog(fd->GetParent());
btnFormat->SetFocus();
});
return panel;
}

void MainWindow::OnOpenFileDlg () {
static wxString wildcardFilter = "MIDI (*.mid)|*.mid|All files (*.*)|*.*";
wxFileDialog fd(this, U(translate("OpenFileDlg")), wxEmptyString, wxEmptyString, wildcardFilter, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
fd.SetExtraControlCreator(createOpenFilePanel);
fd.SetFilterIndex(0);
wxLogNull logNull;
if (wxID_OK==fd.ShowModal()) {
string fn = U(fd.GetPath());
app.loadFile(fn);
}
if (otherTimer->IsRunning()) otherTimer->Stop();
app.stopPlayback();
slPreviewVolume = slPreviewPosition = nullptr;
}

void MainWindow::OnSaveFile () {
if (app.curFile.empty()) OnSaveFileDlg();
else app.saveFile("", 0);
}

void MainWindow::OnSaveFileDlg () {
vector<string> filters;
if (!Encoder::encoders.size()) encAddAll();
for (auto& encoder: Encoder::encoders) {
filters.push_back(::format("{} (*.{})", encoder->name, encoder->extension));
filters.push_back("*." + encoder->extension);
}
wxFileDialog fd(this, U(translate("SaveFileDlg")), wxEmptyString, wxEmptyString, U(join(filters, "|")), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
fd.SetExtraControlCreator(createSaveFilePanel);
if (wxID_OK!=fd.ShowModal()) return;
string file = UFN(fd.GetPath());
int filterIndex = fd.GetFilterIndex();
app.saveFile(file, filterIndex);
}

void MainWindow::showInstrumentDialog () {
SoundfontPreset* p = nullptr;
InstrumentDialog id(app, this, [&](auto q)mutable{ p=q; });
if (id.ShowModal()==wxID_OK) {
int preset = p->dval&0x7F, banklsb = (p->dval>>16)&0x7F, bank = (p->dval>>23)&0x7F;
textArea->WriteText(U(format("p({}, {}, {}) ", bank, banklsb, preset)));
}}

void MainWindow::OnPlayPause () { 
app.pausePlayback();
}

void MainWindow::OnPlayFromCursor () {
int selStart, selEnd;
string code = U(textArea->GetValue());
textArea->GetSelection( reinterpret_cast<long*>(&selStart), reinterpret_cast<long*>(&selEnd));
app.stopPlayback();
try {
app.curStream = CompileMidiAsStream(code, BASS_STREAM_AUTOFREE | BASS_SAMPLE_FLOAT | (app.loop? BASS_SAMPLE_LOOP : 0), &selStart, &selEnd);
BASS_ChannelSetAttribute(app.curStream, BASS_ATTRIB_VOL, app.streamVol);
} catch (syntax_error& e) {
string msg = e.what();
wxMessageBox(U(msg), U(translate("Error")), wxICON_ERROR | wxOK);
textArea->SetInsertionPoint(e.offset);
return;
}
if (selStart>=0) BASS_ChannelSetPosition(app.curStream, selStart, BASS_POS_MIDI_TICK);
if (selStart>=0 && selEnd>0 && selStart!=selEnd) {
BASS_ChannelSetPosition(app.curStream, selEnd, BASS_POS_MIDI_TICK);
auto endPos = BASS_ChannelGetPosition(app.curStream, BASS_POS_BYTE);
BASS_ChannelSetPosition(app.curStream, endPos, BASS_POS_END);
BASS_ChannelSetPosition(app.curStream, selStart, BASS_POS_MIDI_TICK);
auto startPos = BASS_ChannelGetPosition(app.curStream, BASS_POS_BYTE);
BASS_ChannelSetPosition(app.curStream, startPos, BASS_POS_LOOP);
}
app.pausePlayback();
}


void MainWindow::OnLoopChange () {
app.loop = !app.loop;
DWORD stream = app.curStream;
DWORD flags = app.loop? BASS_SAMPLE_LOOP : 0;
BASS_ChannelFlags(stream, flags, BASS_SAMPLE_LOOP);
GetMenuBar() ->Check(IDM_LOOP, app.loop);
//speechSay(U(translate(app.loop? "LoopOn" : "LoopOff")).wc_str(), true);
}

void MainWindow::OnTimer (wxTimerEvent& e) {
if (e.GetId()==98) { timerFunc(); return; }
DWORD stream = app.curStream;
if (stream) {
auto bytePos = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);
auto byteLen = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
int secPos = BASS_ChannelBytes2Seconds(stream, bytePos);
int secLen = BASS_ChannelBytes2Seconds(stream, byteLen);
if (slPreviewPosition) {
slPreviewPosition->SetRange(0, secLen);
slPreviewPosition->SetLineSize(5);
slPreviewPosition->SetPageSize(30);
slPreviewPosition->SetValue(secPos);
}
}}

void MainWindow::TransposeSelection (int n) {
long start, end;
textArea->GetSelection(&start, &end);
wxString wText = textArea->GetRange(start, end);
std::string text = U(wText);
MTTranspose(text, n);
wText = U(text);
textArea->Replace(start, end, wText);
end = start + wText.size();
textArea->SetSelection(start, end);
}

void MainWindow::OnFind (wxCommandEvent&e ) {
lastFind.replacing = false;
FindReplaceDialog frd(app, this, lastFind);
if (frd.ShowModal()==wxID_OK) OnFindNext(e);
}

void MainWindow::OnFindNext (wxCommandEvent&e ) {
if (lastFind.find.empty()) { OnFind(e); return; }

long x, y, pos, len;
wxString text = textArea->GetValue();
wxString find = lastFind.find;
textArea->GetSelection(&x, &y);
textArea->PositionToXY(1+std::min(x, y), &x, &y);
pos = xyToPosition(text, x, y);
if (!lastFind.regex) find = wxRegEx::QuoteMeta(find);
wxRegEx reg(find, wxRE_NEWLINE | (lastFind.icase? wxRE_ICASE : 0));
if (reg.Matches(text.data()+pos, 0, text.size()-pos)) {
reg.GetMatch((size_t*)&x, (size_t*)&len, 0);
positionToXY(text, pos+x, x, y);
pos = textArea->XYToPosition(x, y);
textArea->SetSelection(pos, pos+len);
}
else wxBell();
}

void MainWindow::OnFindPrev (wxCommandEvent&e ) {
if (lastFind.find.empty()) { OnFind(e); return; }


long x, y, start, pos, len, lastLen;
wxString text = textArea->GetValue();
wxString find = lastFind.find;
textArea->GetSelection(&start, &y);
textArea->PositionToXY(std::max(start, y), &x, &y);
start = pos = xyToPosition(text, x, y);
if (!lastFind.regex) find = wxRegEx::QuoteMeta(find);
wxRegEx reg(find, wxRE_NEWLINE | (lastFind.icase? wxRE_ICASE : 0));

x = 0;
while (reg.Matches(text.data() + x, 0, text.size() -x)) {
reg.GetMatch((size_t*)&y, (size_t*)&len, 0);
x += y;
if (x>=pos) break;
start = x;
x++;
lastLen = len;
}
if (start<pos) {
positionToXY(text, start, x, y);
pos = textArea->XYToPosition(x, y);
textArea->SetSelection(pos, pos+lastLen);
}
else wxBell();
}

void MainWindow::OnFindReplace (wxCommandEvent&e ) {
lastFind.replacing = true;
FindReplaceDialog frd(app, this, lastFind);
if (frd.ShowModal()!=wxID_OK) return;
wxString find = lastFind.find;
if (!lastFind.regex) find = wxRegEx::QuoteMeta(find);
wxString text = textArea->GetValue();
long start, end;
textArea->GetSelection(&start, &end);
wxRegEx reg(find, wxRE_NEWLINE | (lastFind.icase? wxRE_ICASE : 0));
reg.Replace(&text, lastFind.replace);
textArea->SetValue(text);
textArea->SetSelection(start, end);
}

void MainWindow::OnClose (wxCloseEvent& e) {
//if (e.CanVeto() && app.config.get("general.confirmOnQuit", true) && wxNO==wxMessageBox(U(translate("confirmquit")), GetTitle(), wxICON_EXCLAMATION | wxYES_NO)) e.Veto();
//else 
app.OnQuit();
e.Skip();
}

