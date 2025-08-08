#include "MyInteractor.h"
#include "Utils.h"
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkRenderWindow.h>

MyInteractor::MyInteractor(wxPanel *panel_, vtkRenderWindow *RenderWindow_)
    : panel{panel_} {

    this->RenderWindow = NULL;
    this->SetRenderWindow(vtkRenderWindow::New());
    this->RenderWindow->Delete();
    Enable();
  DEBUG_MESSAGE("%s", "MyInteractor::MyInteractor()");
}

MyInteractor::~MyInteractor() {
  DEBUG_MESSAGE("%s", "MyInteractor::~MyInteractor");
}

void MyInteractor::Initialize() {
  Parent::Initialize();
  Enable();

  vtkNew<vtkInteractorStyleTrackballCamera> style;
  SetInteractorStyle(style);

  auto rw = GetRenderWindow();
  auto native_handle = utils::NativeHandle(panel);
  DEBUG_MESSAGE("MyInteractor::Initialize(), rw = %p, native handle = %p", rw,
                native_handle);

  rw->SetWindowId(native_handle);
  rw->SetParentId(utils::NativeHandle(panel->GetParent()));
  rw->SetDisplayId(rw->GetGenericDisplayId());

  rw->InitializeFromCurrentContext();

  int *size = rw->GetSize();
  DEBUG_MESSAGE("MyInteractor::Initialize(), sz = %u x %u", size[0], size[1]);

  Size[0] = size[0];
  Size[1] = size[1];
}

void MyInteractor::UpdateSize(int w, int h) {
  if (w != Size[0] || h != Size[1]) {
    DEBUG_MESSAGE("MyInteractor::UpdateSize(), sz = %u x %u", w, h);
    Size[0] = w;
    Size[1] = h;
    GetRenderWindow()->SetSize(w, h);
  }
}
