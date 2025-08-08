#include "MyPanel.h"
#include "Utils.h"

#include <vtkAnimationCue.h>
#include <vtkAnimationScene.h>
#include <vtkCommand.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkRendererCollection.h>

MyPanel::MyPanel(wxWindow *parent, const char *sphere_color)
    : Parent(parent, wxID_ANY, wxDefaultPosition, {200, 100},
             wxFULL_REPAINT_ON_RESIZE) {
  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
  SetMinSize(wxSize(250, 250));

  // render_window = vtkSmartPointer<vtkRenderWindow>::New();

  interactor = new MyInteractor(this, nullptr);
  // interactor->SetRenderWindow(render_window);

  renderer = vtkRenderer::New();
  interactor->GetRenderWindow()->AddRenderer(renderer);

  renderer->SetBackground(0, 0, 0);
  renderer->ResetCamera();

  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->SetCenter(0.0, 0.0, 0.0);
  sphereSource->SetRadius(5.0);

  // Make the surface smooth:
  sphereSource->SetPhiResolution(100);
  sphereSource->SetThetaResolution(100);

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(sphereSource->GetOutputPort());

  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d(sphere_color).GetData());

  renderer->AddActor(actor);
  // interactor->GetRenderWindow()->Render();
  // interactor->GetRenderWindow()->Render();
  // interactor->GetRenderWindow()->Render();
  interactor->GetRenderWindow()->GetRenderers()->InitTraversal();
  interactor->GetRenderWindow()->Render();
  interactor->Start();

  auto backend = interactor->GetRenderWindow()->GetRenderingBackend();
  DEBUG_MESSAGE("MyPanel::MyPanel, backend = %s\n", backend);

  parent->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &MyPanel::OnTabChange, this);
  parent->Bind(wxEVT_LISTBOOK_PAGE_CHANGED, &MyPanel::OnTabChange, this);

  Bind(wxEVT_PAINT, &MyPanel::OnRender, this);
  Bind(wxEVT_SIZE, &MyPanel::OnResize, this);

  Bind(wxEVT_MOTION, &MyPanel::OnMouseMove, this);
  Bind(wxEVT_MOUSEWHEEL, &MyPanel::OnMouseScroll, this);
  Bind(wxEVT_LEFT_DOWN, &MyPanel::OnMouseDown, this);
  Bind(wxEVT_LEFT_UP, &MyPanel::OnMouseUp, this);
  Bind(wxEVT_RIGHT_DOWN, &MyPanel::OnMouseRightDown, this);
  Bind(wxEVT_RIGHT_UP, &MyPanel::OnMouseRightUp, this);
}

MyPanel::~MyPanel() {
  DEBUG_MESSAGE("%s", "MyPanel::~MyPanel");
  renderer->Delete();
  interactor->GetRenderWindow()->Delete();
  interactor->Delete();
}

void MyPanel::OnRender(wxPaintEvent &event) {
    wxPaintDC dc(this);

  // interactor->Render();
  auto rw = interactor->GetRenderWindow();

  DEBUG_MESSAGE("MyPanel::OnRender, rw = %p, native = %p", rw,
                rw->GetGenericWindowId());

  if (!rw->GetGenericWindowId()) {
      DEBUG_MESSAGE("%s", "MyPanel::OnRender, reinitializing...");
      auto native_handle = utils::NativeHandle(this);
      rw->SetWindowId(native_handle);
      rw->SetParentId(reinterpret_cast<void *>(utils::NativeHandle(GetParent())));

      auto sz = GetSize();
      rw->SetSize(sz.x, sz.y);
  }

  rw->WaitForCompletion();

  rw->Render();
  rw->Frame();
  // Refresh();
}

void MyPanel::OnResize(wxSizeEvent &evt) {
  auto w = evt.GetSize().GetWidth();
  auto h = evt.GetSize().GetHeight();
  DEBUG_MESSAGE("MyPanel::OnResize %d x %d", w, h);
  interactor->UpdateSize(w, h);
  Refresh();
}

void MyPanel::OnTabChange(wxBookCtrlEvent &event) {
  DEBUG_MESSAGE("%s %p", "MyFrame::OnTabChange", this);
  // interactor->GetRenderWindow()->Initialize();
  // interactor->Initialize();
  // interactor->GetRenderWindow()->WindowRemap();
}

void MyPanel::OnMouseDown(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "MyPanel::OnMouseDown", this);

  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);

  Invoke(vtkCommand::LeftButtonPressEvent);
}

void MyPanel::OnMouseUp(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "MyPanel::OnMouseUp", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::LeftButtonReleaseEvent);
}

void MyPanel::OnMouseRightDown(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "MyPanel::OnMouseRightDown", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::RightButtonPressEvent);
}

void MyPanel::OnMouseRightUp(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "MyPanel::OnMouseRightUp", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::RightButtonReleaseEvent);
}

void MyPanel::OnMouseMove(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "MyPanel::OnMouseMove", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::MouseMoveEvent);
}

void MyPanel::OnMouseScroll(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "MyPanel::OnMouseScroll", this);

  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  auto command = event.GetWheelRotation() > 0
                     ? vtkCommand::MouseWheelForwardEvent
                     : vtkCommand::MouseWheelBackwardEvent;
  Invoke(command);
}

void MyPanel::Invoke(unsigned long event, void *callData) {
  interactor->InvokeEvent(event, callData);
  Refresh();
}
