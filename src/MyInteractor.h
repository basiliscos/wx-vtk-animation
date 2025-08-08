#pragma once

#include <vtkRenderWindowInteractor.h>
#include <vtkGenericRenderWindowInteractor.h>

#include <wx/wx.h>

struct MyInteractor : vtkGenericRenderWindowInteractor {
  using Parent = vtkGenericRenderWindowInteractor;

  MyInteractor(wxPanel *panel, vtkRenderWindow* RenderWindow);
  ~MyInteractor();
  void Initialize() override;
  void UpdateSize(int x, int y) override;

private:
  wxPanel *panel;
};
