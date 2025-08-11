#pragma once

#include "MyInteractor.h"

#include <wx/bookctrl.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/wx.h>

struct My3DPanel : public wxPanel {
  using Parent = wxPanel;

  My3DPanel(wxWindow *parent, const char *sphere_color);
  ~My3DPanel();

  void OnRender(wxPaintEvent &event);
  void OnResize(wxSizeEvent &evt);
  void OnTabChange(wxBookCtrlEvent &);

  void OnMouseDown(wxMouseEvent &event);
  void OnMouseUp(wxMouseEvent &event);
  void OnMouseRightDown(wxMouseEvent &event);
  void OnMouseRightUp(wxMouseEvent &event);
  void OnMouseMove(wxMouseEvent &event);
  void OnMouseScroll(wxMouseEvent &event);

  void Invoke(unsigned long event, void *callData = nullptr);

  MyInteractor *interactor;
  vtkRenderer *renderer;
  // vtkSmartPointer<vtkRenderWindow> render_window;
};
