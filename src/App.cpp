#include <wx/evtloop.h>
#include <wx/slider.h>
#include <wx/wx.h>

#include <wx/listbook.h>
#include <wx/notebook.h>

#include "My2DPanel.h"
#include "My3DPanel.h"

#include "Utils.h"

class MyApp : public wxApp {
public:
  using Parent = wxApp;

  bool OnInit() override;
  int OnExit() override;
};

class MyFrame : public wxFrame {
public:
  MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size);

private:
  wxSlider *slider;
  wxButton *button;
};

bool MyApp::OnInit() {
  DEBUG_MESSAGE("%s", "MyApp::OnInit");

  MyFrame *frame =
      new MyFrame("Hello World", wxPoint(50, 50), wxSize(450, 340));
  frame->Show(true);
  return true;
}

int MyApp::OnExit() {
  DEBUG_MESSAGE("%s", "MyApp::OnExit");
  return Parent::OnExit();
}

MyFrame::MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
    : wxFrame(NULL, wxID_ANY, title, pos, size) {
  auto sizer = new wxBoxSizer(wxVERTICAL);
  auto tabs = new wxNotebook(this, wxID_HIGHEST + 1, wxDefaultPosition,
  wxDefaultSize, wxNB_TOP);

  auto page_0 = new My2DPanel(tabs);
  auto page_1 = new My3DPanel(tabs, "Red");
  // auto page_2 = new My3DPanel(tabs, "Cyan");

  tabs->AddPage(page_0, _("2D"), true);
  tabs->AddPage(page_1, _("3D/red"), false);
  // tabs->AddPage(page_2, _("page_2"), false);

  sizer->Add(tabs, 1, wxEXPAND);
  // auto label = new wxStaticText(this, wxID_ANY, "hello");

  // sizer->Add(label, 0, 0);
  // sizer->Add(page_0, 10, wxEXPAND);

  SetSizerAndFit(sizer);
  PostSizeEvent();
}

wxIMPLEMENT_APP(MyApp);
