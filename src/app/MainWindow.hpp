#ifndef _____MAIN_WINDOW_0
#define _____MAIN_WINDOW_0
#include "../common/wxUtils.hpp"
 #include "FindReplaceDialog.hpp"

struct MainWindow: wxFrame {
struct App& app;

struct wxStatusBar* status;
struct wxTextCtrl *textArea;
struct wxSlider *slPreviewPosition=nullptr, *slPreviewVolume=nullptr;

struct wxTimer *refreshTimer = nullptr, *otherTimer = nullptr;
struct wxProgressDialog* progressDialog = nullptr;
bool progressCancelled=false;
std::function<void()> timerFunc = nullptr;

FindReplaceInfo lastFind;

MainWindow (App& app);
void showAboutBox (wxWindow* parent);

void openProgress (const std::string& title, const std::string& message, int maxValue);
void closeProgress ();
void updateProgress (int value);
void setProgressText (const std::string& msg);
void setProgressTitle (const std::string& msg);
void setProgressMax (int max);
bool isProgressCancelled ();
void OnProgress (struct wxThreadEvent& e);
void setTimeout (int ms, const std::function<void()>& func);
void clearTimeout ();

int popupMenu (const std::vector<std::string>& items, int selection=-1);

void OnClose (wxCloseEvent& e);
void OnTimer (struct wxTimerEvent& e);

void OnLoopChange ();
void OnLoopChange (wxCommandEvent& e) { OnLoopChange(); }
void OnPlayPause ();
void OnPlayPause (wxCommandEvent& e) { OnPlayPause(); }
void OnPlayFromCursor ();
void OnPlayFromCursor (wxCommandEvent& e) { OnPlayFromCursor(); }
void showInstrumentDialog ();

void OnOpenFileDlg ();
void OnOpenFileDlg (wxCommandEvent& e) { OnOpenFileDlg(); }
void OnSaveFile ();
void OnSaveFile (wxCommandEvent& e) { OnSaveFile(); }
void OnSaveFileDlg ();
void OnSaveFileDlg (wxCommandEvent& e) { OnSaveFileDlg(); }
void OnAbout (wxCommandEvent & e) { showAboutBox(this); }
void OnFind (wxCommandEvent&e );
void OnFindNext (wxCommandEvent&e );
void OnFindPrev (wxCommandEvent&e );
void OnFindReplace (wxCommandEvent&e );
};

#endif
