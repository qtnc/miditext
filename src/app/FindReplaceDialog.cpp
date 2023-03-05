#include "FindReplaceDialog.hpp"
#include "App.hpp"
#include "MainWindow.hpp"
using namespace std;

FindReplaceDialog::FindReplaceDialog (App& app, wxWindow* parent, FindReplaceInfo& info1): 
wxDialog(parent, wxID_ANY, U(translate(info1.replacing? "FindReplaceDlg":"FindDlg")) ),
info(info1) {
auto lblFind = new wxStaticText(this, wxID_ANY, U(translate("FRFind")), wxPoint(-2, -2), wxSize(1, 1) );
tfFind = new wxTextCtrl(this, wxID_ANY, info.find, wxDefaultPosition, wxDefaultSize, wxHSCROLL);
auto lblReplace = new wxStaticText(this, wxID_ANY, U(translate("FRReplace")), wxPoint(-2, -2), wxSize(1, 1) );
tfReplace = new wxTextCtrl(this, wxID_ANY, info.replace, wxDefaultPosition, wxDefaultSize, wxHSCROLL);
cbIcase = new wxCheckBox(this, wxID_ANY, U(translate("FRIcase")));
cbRegex = new wxCheckBox(this, wxID_ANY, U(translate("FRRegex")));
cbIcase->SetValue(info.icase);
cbRegex->SetValue(info.regex);
tfReplace->Enable(info.replacing);

auto gSizer = new wxBoxSizer(wxVERTICAL);
auto sizer = new wxFlexGridSizer(0, 2, 0, 0);
sizer->Add(lblFind, 0);
sizer->Add(tfFind, 0, wxEXPAND);
sizer->Add(lblReplace);
sizer->Add(tfReplace, 0, wxEXPAND);
sizer->Add(cbIcase);
sizer->Add(cbRegex);
gSizer->Add(sizer, 1, wxEXPAND);
auto btnSizer = new wxStdDialogButtonSizer();
auto okBtn = new wxButton(this, wxID_OK);
btnSizer->AddButton(okBtn);
btnSizer->AddButton(new wxButton(this, wxID_CANCEL));
btnSizer->Realize();
gSizer->Add(btnSizer, 0, wxEXPAND);
SetSizerAndFit(gSizer);

Bind(wxEVT_BUTTON, &FindReplaceDialog::OnValidate, this, wxID_OK);
tfFind->SetFocus();
SetAffirmativeId(wxID_OK);
SetDefaultItem(okBtn);
}

void FindReplaceDialog::OnValidate (wxCommandEvent& e) {
e.Skip();
info.find = tfFind->GetValue();
info.replace = tfReplace->GetValue();
info.regex = cbRegex->GetValue();
info.icase = cbIcase->GetValue();
}

