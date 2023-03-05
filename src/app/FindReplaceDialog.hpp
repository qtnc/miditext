#ifndef ____FINDREPLACEDIALOG_1___
#define ____FINDREPLACEDIALOG_1___
#include "../common/wxUtils.hpp"

struct FindReplaceInfo {
wxString find, replace;
bool replacing, icase, regex;
};

struct FindReplaceDialog: wxDialog {
struct wxTextCtrl *tfFind, *tfReplace;
struct wxCheckBox *cbIcase, *cbRegex;
FindReplaceInfo& info;

FindReplaceDialog (App& app, wxWindow* parent, FindReplaceInfo& info1);
void OnValidate (wxCommandEvent& e);
};

#endif
