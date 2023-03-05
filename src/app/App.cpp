#include "App.hpp"
#include "MainWindow.hpp"
#include "WorkerThread.hpp"
#include "MidiFile.hpp"
#include "Encoder.hpp"
#include "../common/bass.h"
#include "../common/bass_fx.h"
#include "../common/bassmidi.h"
#include "../common/bassmix.h"
#include "../common/bassenc.h"
#include "../common/wxUtils.hpp"
#include "../common/stringUtils.hpp"
#include "../common/println.hpp"
#include <wx/cmdline.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/textdlg.h>
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/snglinst.h>
#include <wx/ipc.h>
#include <wx/datetime.h>
#include <wx/translation.h>
#include<string>
#include<vector>
#include<unordered_map>
using namespace std;

wxIMPLEMENT_APP(App);

extern bool BASS_SimpleInit (int device);
extern bool BASS_RecordSimpleInit (int device);
extern DWORD BASS_StreamCreateCopy (DWORD source, bool decode=true);
extern vector<pair<int,string>> BASS_GetDeviceList (bool includeLoopback = false);
extern vector<pair<int,string>> BASS_RecordGetDeviceList (bool includeLoopback = false);

extern MidiFile CompileMidi (const std::string& code, int* mark1=nullptr, int* mark2=nullptr);
extern DWORD CompileMidiAsStream (const string& code, DWORD flags=0, int* mark1=0, int* mark2=0);

struct CustomFileTranslationLoader: wxTranslationsLoader {
virtual wxMsgCatalog* LoadCatalog (const wxString& domain, const wxString& lang) final override {
string filename = "lang/" + U(domain) + "_" + U(lang) + ".mo";
wxMsgCatalog* re = nullptr;
bool existing = false;
{ ifstream in(filename, ios::binary); if (in) existing=true; }
if (existing) re = wxMsgCatalog::CreateFromFile( U(filename), domain );
return re;
}
     virtual wxArrayString GetAvailableTranslations(const wxString& domain) const final override {
vector<string> langs = { "fr", "en", "it", "es", "pt", "de", "ru" };
wxArrayString v;
for (auto& s: langs) v.push_back(s);
return v;
}};

bool App::OnInit () {
initDirs();
initConfig();
initLocale();
initTranslations();
initAudio();

if (!wxApp::OnInit()) return false;

win = new MainWindow(*this);
win->Show(true);
worker = new WorkerThread(*this);
worker->Run();
Bind(wxEVT_CHAR_HOOK, &App::OnGlobalCharHook, this);

if (!curFile.empty()) loadFile(curFile);
return true;
}

void App::OnInitCmdLine (wxCmdLineParser& cmd) {
wxApp::OnInitCmdLine(cmd);
cmd.AddParam(wxEmptyString, wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL);
}

bool App::OnCmdLineParsed (wxCmdLineParser& cmd) {
vector<string> files;
for (int i=0, n=cmd.GetParamCount(); i<n; i++) files.push_back(UFN(cmd.GetParam(i)));
if (!files.empty()) curFile = files[0];
return true;
}


bool App::initDirs () {
SetAppName(APP_NAME);
SetClassName(APP_NAME);
SetVendorName(APP_VENDOR);

auto& stdPaths = wxStandardPaths::Get();
appDir = wxFileName(stdPaths.GetExecutablePath()).GetPath();
userDir = stdPaths.GetUserDataDir();
userLocalDir = stdPaths.GetUserLocalDataDir();


auto userDirFn = wxFileName::DirName(userDir);
auto userLocalDirFn = wxFileName::DirName(userLocalDir);

pathList.Add(userDir);
//pathList.Add(userLocalDir);
pathList.Add(appDir);

return 
userDirFn .Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)
&& userLocalDirFn .Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)
&& userDirFn.IsDirReadable()
&& userLocalDirFn.IsDirReadable();
}

bool App::initConfig () {
string configIniPath = UFN(pathList.FindAbsoluteValidPath(CONFIG_FILENAME));
if (configIniPath.empty()) cout << "No " << CONFIG_FILENAME << " found, fallback to defaults" << endl;
else {
cout << CONFIG_FILENAME << " found: " << configIniPath << endl;
config.setFlags(PM_BKESC);
config.load(configIniPath);
}

for (int i=1; ; i++) {
auto s = config.get("font" + to_string(i), "");
if (s.empty()) break;
sfmap.addFontLine(s);
}
for (int i=1; ; i++) {
auto s = config.get("fontmap" + to_string(i), "");
if (s.empty()) break;
sfmap.addMapping(s);
}
println("{} soundfonts loaded, {} mappings", sfmap.fonts.size(), sfmap.mappings.size());
return true;
}

string App::findWritablePath (const string& wantedPath) {
int lastSlash = wantedPath.find_last_of("/\\");
string path, file;
if (lastSlash==string::npos) { path = ""; file=wantedPath; }
else { path=wantedPath.substr(0, lastSlash); file=wantedPath.substr(lastSlash+1); }
for (int i=pathList.GetCount() -1; i>=0; i--) {
auto wxfn = wxFileName(pathList.Item(i), wxEmptyString);
if (wxfn.IsFileWritable() || wxfn.IsDirWritable()) {
string dirpath = UFN(wxfn.GetFullPath());
if (path.size()) {
if (!ends_with(dirpath, "/") && !ends_with(dirpath, "\\")) dirpath += "/";
dirpath += path;
}
if (wxFileName(dirpath, "").Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
wxfn = wxFileName(dirpath, file);
if (wxfn.IsFileWritable() || wxfn.IsDirWritable()) {
return UFN(wxfn.GetFullPath());
}}}}
return string();
}

bool App::saveConfig () {
string filename = findWritablePath(CONFIG_FILENAME);
if (filename.empty()) {
cout << "No valid writable path found to save configuration " << CONFIG_FILENAME << endl;
return false;
}
cout << "Saving configuration to " << filename << endl;

config.set("loop", loop);
config.set("volume", streamVol * 100.0f);

return config.save(filename);
}

bool App::initAudio () {
BASS_SetConfig(BASS_CONFIG_UNICODE, true);
BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, true);
BASS_SetConfig(BASS_CONFIG_FLOATDSP, true);
BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1);
BASS_SetConfig(BASS_CONFIG_BUFFER, 150);
BASS_SetConfig(BASS_CONFIG_MIXER_BUFFER, 1);

streamVol = config.get("volume", streamVol * 100.0f) / 100.0f;
loop = config.get("loop", loop);

if (!BASS_SimpleInit(-1)) return false;

wxDir dir(appDir);
wxString dllFile;
if (dir.GetFirst(&dllFile, U("bass?*.dll"))) do {
string sFile = UFN(dllFile);
if (sFile=="bass.dll" || sFile=="bass_fx.dll" || sFile=="bassmix.dll" || starts_with(sFile, "bassenc")) continue;
auto plugin = BASS_PluginLoad(sFile.c_str(), 0);
} while(dir.GetNext(&dllFile));

BASS_SetConfig(BASS_CONFIG_MIDI_AUTOFONT, 2);
BASS_SetConfig(BASS_CONFIG_MIDI_VOICES, config.get("maxVoices", 256)); 
BASS_SetConfigPtr(BASS_CONFIG_MIDI_DEFFONT, (const char*)nullptr);
BASS_MIDI_StreamSetFonts(0, &sfmap.mappings[0], sfmap.mappings.size() | BASS_MIDI_FONT_EX);

return true;
}

bool App::initLocale () {
locale = config.get("locale", "default");
wxlocale = new wxLocale();
println("Locale is configured to {}", locale);
if (locale=="default") {
wxlocale->Init();
}
else {
auto info = wxLocale::FindLanguageInfo(U(locale));
if (info) wxlocale->Init(info->Language);
}
locale = U(wxlocale->GetCanonicalName());
println("Locale initialized to {}", locale);
auto& translations = *wxTranslations::Get();
translations.SetLoader(new CustomFileTranslationLoader());
translations.AddStdCatalog();
return true;
}

bool App::initTranslations () {
vector<string> locales = {
config.get("locale", locale),
config.get("locale", locale).substr(0, 5),
config.get("locale", locale).substr(0, 2),
locale,
locale.substr(0, 5),
locale.substr(0, 2),
"en"
};
for (string& l: locales) {
string transPath = UFN(pathList.FindAbsoluteValidPath(format("lang/miditext_{}.properties", l)));
if (!transPath.empty()) {
lang.setFlags(PM_BKESC);
lang.load(transPath);
break;
}}
return true;
}

void App::startPlayback (const std::string& filename) {
stopPlayback();
DWORD flags = BASS_SAMPLE_FLOAT | BASS_STREAM_PRESCAN | BASS_STREAM_AUTOFREE | (loop? BASS_SAMPLE_LOOP : 0);
curStream = BASS_StreamCreateFile(false, filename.c_str(), 0, 0, flags);
if (!curStream) return;
curSelectedFile = filename;
BASS_ChannelSetAttribute(curStream, BASS_ATTRIB_VOL, streamVol);
BASS_ChannelPlay(curStream, false);
}

void App::stopPlayback () {
if (curStream) BASS_ChannelStop(curStream);
curSelectedFile.clear();
curStream=0;
}

void App::pausePlayback () {
if (!curStream) return;
auto state = BASS_ChannelIsActive(curStream);
switch(state){
case BASS_ACTIVE_PLAYING:
case BASS_ACTIVE_STALLED:
BASS_ChannelPause(curStream);
break;
case BASS_ACTIVE_STOPPED:
case BASS_ACTIVE_PAUSED:
BASS_ChannelPlay(curStream, false);
break;
}}

void App::seekPlayback (double time, bool abs) {
if (!curStream) return;
if (!abs) time += BASS_ChannelBytes2Seconds(curStream, BASS_ChannelGetPosition(curStream, BASS_POS_BYTE));
BASS_ChannelSetPosition(curStream, BASS_ChannelSeconds2Bytes(curStream, time), BASS_POS_BYTE);
}

void App::changeVol (float value, bool abs) {
if (!abs) streamVol += value;
streamVol = std::max(0.01f, std::min(streamVol, 1.0f));
if (curStream) BASS_ChannelSetAttribute(curStream, BASS_ATTRIB_VOL, streamVol);
}

void App::loadFile (const string& filename) {
MidiFile mf;
ifstream in(filename, ios::binary);
if (!mf.open(in)) return;
curFile = filename;
string code = mf.getCodeText();
replace_all(code, "\r", "");
win->textArea->SetValue(U(code));
updateWindowTitle();
}

void App::saveFile (const string& filename0, int encoderIndex) {
string filename = filename0; auto& app = *this;
if (!Encoder::encoders.size()) encAddAll();
if (filename.empty()) filename = curFile;
if (filename.empty()) return;
try {
auto& encoder = *Encoder::encoders[encoderIndex];
string code = U(win->textArea->GetValue());
bool updateCurFile = false;
if (encoder.isUsingSaveFile()) {
MidiFile mf = CompileMidi(code);
updateCurFile = encoder.saveFile(mf, filename) && encoderIndex==0;
}
else {
DWORD stream = CompileMidiAsStream(code, BASS_STREAM_DECODE | BASS_STREAM_PRESCAN | BASS_SAMPLE_FLOAT);
saveEncode(encoder, stream, filename);
}
if (updateCurFile) {
curFile=filename;
updateWindowTitle();
}
} catch (exception& e) {
string msg = e.what();
wxMessageBox(U(msg), U(translate("Error")), wxICON_ERROR | wxOK);
}}

void App::updateWindowTitle () {
if (curFile.empty()) { win->SetTitle("MidiText"); return; }
auto i = curFile.find_last_of("/\\");
string fn = i==string::npos? curFile : curFile.substr(i+1);
win->SetTitle(U("MidiText - " + fn));
}

void App::saveEncode (Encoder& encoder, DWORD source, const std::string& file) {
if (!source || file.empty()) return;
auto& app = *this;
worker->submit([=, &encoder, &app](){
DWORD encoderHandle = encoder.startEncoderFile(source, file);
auto lastSlash = file.find_last_of("/\\");
string sFile = file.substr(lastSlash==string::npos? 0 : lastSlash+1);
unique_ptr<char[]> buffer = make_unique<char[]>(65536);
int read=0, length = BASS_ChannelGetLength(source, BASS_POS_BYTE);
win->openProgress(format(translate("Saving"), sFile), format(translate("Saving"), sFile), length);
for (int count=0; count<length; ) {
read = BASS_ChannelGetData(source, &buffer[0], 65536);
if (read<0 || win->isProgressCancelled()) break;
count += read;
win->setProgressTitle(format(translate("SavingWP"), sFile, static_cast<int>(100.0 * count / length)));
win->updateProgress(count);
}
win->closeProgress();
BASS_StreamFree(source);
});}

/*void App::startMix () {
if (mixHandle) return;
BASS_SimpleInit(0);
BASS_SetDevice(0);
mixHandle = BASS_Mixer_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT | BASS_STREAM_AUTOFREE | BASS_MIXER_NONSTOP);
BASS_ChannelSetAttribute(mixHandle, BASS_ATTRIB_VOL, 0);
BASS_ChannelPlay(mixHandle, false);
plugCurStreamToMix(*this);
}

void App::stopMix () {
BASS_ChannelStop(mixHandle);
mixHandle = 0;
}

void App::tryStopMix () {
if (!encoderHandle && !micHandle1 && !micHandle2) stopMix();
}

void App::stopCaster () {
tryStopMix();
BASS_Encode_Stop(encoderHandle);
if (encoderHandle) {
App& app = *this;
speechSay(U(translate("CastStopped")).wc_str(), true);
}
encoderHandle = 0;
}

static void CALLBACK encoderNotification (HENCODE encoderHandle, DWORD status, void* udata) {
if (status==BASS_ENCODE_NOTIFY_CAST || status==BASS_ENCODE_NOTIFY_ENCODER || status==BASS_ENCODE_NOTIFY_FREE) {
App& app = wxGetApp();
app .stopCaster();
speechSay(U(translate("CastStopped")).wc_str(), true);
}}

void App::startCaster (Caster& caster, Encoder& encoder, const string& server, const string& port, const string& user, const string& pass, const string& mount) {
stopCaster();
startMix();
encoderHandle = caster.startCaster(mixHandle, encoder, server, port, user, pass, mount);
if (encoderHandle) {
BASS_Encode_SetNotify(encoderHandle, &encoderNotification, nullptr);
App& app = *this;
speechSay(U(translate("CastStarted")).wc_str(), true);
}
else stopCaster();
}*/

void App::OnGlobalCharHook (wxKeyEvent& e) {
auto k = e.GetKeyCode(), mods = e.GetModifiers();
e.Skip();
if (curStream && win && win->slPreviewVolume && win->slPreviewPosition) {
if (mods==wxMOD_NONE && k==WXK_F8) pausePlayback();
else if (mods==wxMOD_NONE && k==WXK_F6) { seekPlayback(-5); e.Skip(false); }
else if (mods==wxMOD_SHIFT && k==WXK_F6) { seekPlayback(-30); e.Skip(false); }
else if (mods==wxMOD_NONE && k==WXK_F7) seekPlayback(5);
else if (mods==wxMOD_SHIFT  && k==WXK_F7) seekPlayback(30);
else if (mods==wxMOD_NONE && k==WXK_F11) changeVol(-0.02f);
else if (mods==wxMOD_NONE && k==WXK_F12 ) changeVol(0.02f);
}}

void App::OnQuit  () {
config.set("stream.volume", 100.0f * streamVol);
config.set("stream.loop", loop);
saveConfig();
}


