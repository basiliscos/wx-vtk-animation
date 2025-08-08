#pragma once

#include "MyInteractor.h"

#include <wx/wx.h>
#include <wx/bookctrl.h>
#include <wx/notebook.h>
#include <wx/listbook.h>

struct MyPanel : public wxPanel {
  using Parent = wxPanel;

  MyPanel(wxWindow *parent, const char *sphere_color);
  ~MyPanel();

  void OnRender(wxPaintEvent &event);
  void OnResize(wxSizeEvent &evt);
  void OnTabChange(wxBookCtrlEvent &);

  MyInteractor *interactor;
  vtkRenderer *renderer;

  DECLARE_EVENT_TABLE()
};
