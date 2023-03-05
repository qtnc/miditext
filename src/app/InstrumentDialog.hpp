#ifndef ____INSTRUMENTDIALOG_1___
#define ____INSTRUMENTDIALOG_1___
#include "../common/wxUtils.hpp"
#include<functional>

struct InstrumentDialog: wxDialog {
typedef std::function<void(struct SoundfontPreset*)> CloseCallback;
App& app;
struct wxTextCtrl* tfFilter;
struct wxTextCtrl* tfTestCode;
struct wxListView* lcList;
CloseCallback closeCb;
std::string input;
long lastInputTime;

static std::string testCode, testCodeSnd, testCodeDrums;

InstrumentDialog (App& app, wxWindow* parent, const CloseCallback& ccb = nullptr);
void updateList ();
void onFilterChange ();
void onFilterEnter ();
void onSelectionChange ();
void onTestCodeChange ();
std::string& getTestCode (int preset);
void onListEnter ();
void OnListKeyChar (wxKeyEvent& e);
struct SoundfontPreset* getSelectedPreset ();
};

#endif
