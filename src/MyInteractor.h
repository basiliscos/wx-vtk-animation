#pragma once

#include <vtkRenderWindowInteractor.h>
#include <wx/wx.h>

struct MyInteractor : vtkRenderWindowInteractor {
  using Parent = vtkRenderWindowInteractor;

  MyInteractor(wxPanel *panel);
  ~MyInteractor();
  void Initialize() override;
  void UpdateSize(int x, int y) override;

private:
  wxPanel *panel;
};

