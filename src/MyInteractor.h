#pragma once

#include <vtkContextInteractorStyle.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkRenderWindowInteractor.h>

#include <wx/wx.h>

struct MyInteractor : vtkRenderWindowInteractor {
  using Parent = vtkRenderWindowInteractor;

  MyInteractor(wxPanel *panel, vtkRenderWindow *RenderWindow,
               vtkContextInteractorStyle *style);
  ~MyInteractor();
  void Initialize() override;
  void UpdateSize(int x, int y) override;

private:
  wxPanel *panel;
  vtkContextInteractorStyle *style;
};
