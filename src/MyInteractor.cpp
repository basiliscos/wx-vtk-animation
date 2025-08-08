#include "MyInteractor.h"
#include "Utils.h"
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindow.h>

MyInteractor::MyInteractor(wxPanel *panel_) : panel{panel_} {
  // this->RenderWindow->Delete();
  DEBUG_MESSAGE("%s", "MyInteractor::MyInteractor()");
}

MyInteractor::~MyInteractor() {
  DEBUG_MESSAGE("%s", "MyInteractor::~MyInteractor");
}

void MyInteractor::Initialize() {
  DEBUG_MESSAGE("%s", "MyInteractor::Initialize()");
  Parent::Initialize();


  vtkNew<vtkInteractorStyleTrackballCamera> style;
  SetInteractorStyle(style);

  RenderWindow->SetWindowId(utils::NativeHandle(panel));
  RenderWindow->SetParentId(utils::NativeHandle(panel->GetParent()));
  RenderWindow->SetDisplayId(RenderWindow->GetGenericDisplayId());

  int *size = RenderWindow->GetSize();
  DEBUG_MESSAGE("MyInteractor::Initialize(), sz = %u x %u", size[0], size[1]);

  Size[0] = size[0];
  Size[1] = size[1];
}

void MyInteractor::UpdateSize(int w, int h) {
  if (w != Size[0] || h != Size[1]) {
    DEBUG_MESSAGE("MyInteractor::UpdateSize(), sz = %u x %u", w, h);
    Size[0] = w;
    Size[1] = h;
    RenderWindow->SetSize(w, h);
    panel->Refresh();
  }
}
