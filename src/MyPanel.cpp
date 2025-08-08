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


BEGIN_EVENT_TABLE(MyPanel, wxPanel)
EVT_PAINT(MyPanel::OnRender)
EVT_SIZE(MyPanel::OnResize)
END_EVENT_TABLE()

MyPanel::MyPanel(wxWindow *parent, const char *sphere_color)
    : Parent(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
             wxFULL_REPAINT_ON_RESIZE) {
  SetBackgroundStyle(wxBG_STYLE_CUSTOM);

  interactor = new MyInteractor(this);
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

  parent->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &MyPanel::OnTabChange, this);
  parent->Bind(wxEVT_LISTBOOK_PAGE_CHANGED, &MyPanel::OnTabChange, this);
}

/*****************************************************************************/

MyPanel::~MyPanel() {
  DEBUG_MESSAGE("%s", "MyPanel::~MyPanel");
  renderer->Delete();
  interactor->GetRenderWindow()->Delete();
  interactor->Delete();
}

/*****************************************************************************/

void MyPanel::OnRender(wxPaintEvent &event) {
  DEBUG_MESSAGE("%s", "MyPanel::OnRender");
  interactor->GetRenderWindow()->Render();
}

/*****************************************************************************/

void MyPanel::OnResize(wxSizeEvent &evt) {
  auto w = evt.GetSize().GetWidth();
  auto h = evt.GetSize().GetHeight();
  DEBUG_MESSAGE("MyPanel::OnResize %d x %d", w, h);
  interactor->UpdateSize(w, h);
  Refresh();
}

/*****************************************************************************/

void MyPanel::OnTabChange(wxBookCtrlEvent &event) {
  DEBUG_MESSAGE("%s %p", "MyFrame::OnTabChange", this);
  // interactor->GetRenderWindow()->Initialize();
  // interactor->Initialize();
  // interactor->GetRenderWindow()->WindowRemap();
}
