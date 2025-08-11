#include "My3DPanel.h"
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
#include <vtkInteractorStyleTrackballCamera.h>

My3DPanel::My3DPanel(wxWindow *parent, const char *sphere_color)
    : Parent(parent, wxID_ANY, wxDefaultPosition, {200, 100},
             wxFULL_REPAINT_ON_RESIZE) {
  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
  SetMinSize(wxSize(250, 250));

  // render_window = vtkSmartPointer<vtkRenderWindow>::New();

  auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();

  interactor = new MyInteractor(this, nullptr, style);
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
  interactor->GetRenderWindow()->Render();
  interactor->Start();

  auto backend = interactor->GetRenderWindow()->GetRenderingBackend();
  DEBUG_MESSAGE("My3DPanel::My3DPanel, backend = %s\n", backend);

  parent->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &My3DPanel::OnTabChange, this);
  parent->Bind(wxEVT_LISTBOOK_PAGE_CHANGED, &My3DPanel::OnTabChange, this);

  Bind(wxEVT_PAINT, &My3DPanel::OnRender, this);
  Bind(wxEVT_SIZE, &My3DPanel::OnResize, this);

  Bind(wxEVT_MOTION, &My3DPanel::OnMouseMove, this);
  Bind(wxEVT_MOUSEWHEEL, &My3DPanel::OnMouseScroll, this);
  Bind(wxEVT_LEFT_DOWN, &My3DPanel::OnMouseDown, this);
  Bind(wxEVT_LEFT_UP, &My3DPanel::OnMouseUp, this);
  Bind(wxEVT_RIGHT_DOWN, &My3DPanel::OnMouseRightDown, this);
  Bind(wxEVT_RIGHT_UP, &My3DPanel::OnMouseRightUp, this);
}

My3DPanel::~My3DPanel() {
  DEBUG_MESSAGE("%s", "My3DPanel::~My3DPanel");
  renderer->Delete();
  interactor->Delete();
}

void My3DPanel::OnRender(wxPaintEvent &event) {
  auto rw = interactor->GetRenderWindow();

  DEBUG_MESSAGE("My3DPanel::OnRender, rw = %p, native = %p", rw,
                rw->GetGenericWindowId());
  rw->Render();
}

void My3DPanel::OnResize(wxSizeEvent &evt) {
  auto w = evt.GetSize().GetWidth();
  auto h = evt.GetSize().GetHeight();
  DEBUG_MESSAGE("My3DPanel::OnResize %d x %d", w, h);
  interactor->UpdateSize(w, h);
  Refresh();
}

void My3DPanel::OnTabChange(wxBookCtrlEvent &event) {
  DEBUG_MESSAGE("%s %p", "MyFrame::OnTabChange", this);
  // interactor->GetRenderWindow()->Initialize();
  // interactor->Initialize();
  // interactor->GetRenderWindow()->WindowRemap();
}

void My3DPanel::OnMouseDown(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My3DPanel::OnMouseDown", this);

  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);

  Invoke(vtkCommand::LeftButtonPressEvent);
}

void My3DPanel::OnMouseUp(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My3DPanel::OnMouseUp", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::LeftButtonReleaseEvent);
}

void My3DPanel::OnMouseRightDown(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My3DPanel::OnMouseRightDown", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::RightButtonPressEvent);
}

void My3DPanel::OnMouseRightUp(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My3DPanel::OnMouseRightUp", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::RightButtonReleaseEvent);
}

void My3DPanel::OnMouseMove(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My3DPanel::OnMouseMove", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::MouseMoveEvent);
}

void My3DPanel::OnMouseScroll(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My3DPanel::OnMouseScroll", this);

  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  auto command = event.GetWheelRotation() > 0
                     ? vtkCommand::MouseWheelForwardEvent
                     : vtkCommand::MouseWheelBackwardEvent;
  Invoke(command);
}

void My3DPanel::Invoke(unsigned long event, void *callData) {
  interactor->InvokeEvent(event, callData);
  Refresh();
}
