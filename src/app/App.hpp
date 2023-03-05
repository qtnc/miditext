#ifndef _____APP_HPP_0
#define _____APP_HPP_0
#include "../common/constants.h"
#include "../common/PropertyMap.hpp"
#include "SoundfontMapping.hpp"
#include "../common/wxUtils.hpp"
#include <wx/thread.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include<string>
#include<vector>
#include<unordered_map>
#include<fstream>
#include<functional>


struct App;
wxDECLARE_APP(App);

struct App: wxApp {
struct MainWindow* win = nullptr;
struct WorkerThread* worker = nullptr;
wxLocale* wxlocale;
std::string locale;

PropertyMap config, lang;
wxPathList pathList;
wxString appDir, userDir, userLocalDir;

SoundfontMapping sfmap;
std::string curFile, curSelectedFile;
bool loop = false;
float streamVol = 0.2f;
DWORD curStream = 0;

bool initDirs ();
bool initConfig ();
bool initLocale ();
bool initTranslations ();
bool initSpeech ();
bool initAudio ();

bool saveConfig ();
std::string findWritablePath (const std::string& filename);

void OnGlobalCharHook (struct wxKeyEvent& e);

virtual bool OnInit () override;
virtual void OnInitCmdLine (wxCmdLineParser& cmd) override;
virtual bool OnCmdLineParsed (wxCmdLineParser& cmd) override;
void OnQuit ();

void loadFile (const std::string& filename);
void saveFile (const std::string& filename, int encoderIndex);
void saveEncode (struct Encoder& encoder, DWORD stream, const std::string& file);
void updateWindowTitle ();

void startPlayback (const std::string& filename);
void stopPlayback ();
void pausePlayback ();
void seekPlayback (double time, bool absolute = false);
void changeVol (float value, bool absolute = false);
};

template <class F> inline void RunEDT (const F& f) {
wxGetApp().CallAfter(f);
}


#endif

