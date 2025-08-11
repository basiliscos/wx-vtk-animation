#include "My2DPanel.h"
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
#include <vtkRendererCollection.h>
#include <vtkSphereSource.h>

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkAxis.h>
#include <vtkCamera.h>
#include <vtkChartXY.h>
#include <vtkCommand.h>
#include <vtkContextActor.h>
#include <vtkContextInteractorStyle.h>
#include <vtkContextMouseEvent.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkCubeSource.h>
#include <vtkFloatArray.h>
#include <vtkInteractorStyle.h>
#include <vtkMath.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPlot.h>
#include <vtkPlotLine.h>
#include <vtkPlotPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkTextProperty.h>
#include <vtkXMLTableReader.h>

My2DPanel::My2DPanel(wxWindow *parent)
    : Parent(parent, wxID_ANY, wxDefaultPosition, {200, 100},
             wxFULL_REPAINT_ON_RESIZE) {
  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
  SetMinSize(wxSize(640, 480));

  auto table = vtkSmartPointer<vtkTable>::New();

  auto array_x = vtkSmartPointer<vtkFloatArray>::New();
  array_x->SetName("X Axis");
  table->AddColumn(array_x);

  auto arr_tang = vtkSmartPointer<vtkFloatArray>::New();
  arr_tang->SetName("Tan");
  table->AddColumn(arr_tang);

  // Fill table with plot data:
  int num_points = 69;
  float inc = 7.5 / (num_points - 1.0);
  table->SetNumberOfRows(num_points);

  for (int i = 0; i < num_points; ++i) {
    table->SetValue(i, 0, i * inc);
    table->SetValue(i, 1, tan(i * inc) + 0.5);
  }

  // Allocate chart objects:
  auto chart = vtkSmartPointer<vtkChartXY>::New();
  chart->SetAutoSize(true);
  chart->GetAxis(vtkAxis::LEFT)->GetPen()->SetColor(255, 255, 255);
  chart->GetAxis(vtkAxis::BOTTOM)->GetPen()->SetColor(255, 255, 255);
  chart->GetAxis(vtkAxis::RIGHT)->GetPen()->SetColor(255, 255, 255);
  chart->GetAxis(vtkAxis::TOP)->GetPen()->SetColor(255, 255, 255);
  chart->GetAxis(vtkAxis::LEFT)->GetLabelProperties()->SetColor(1.0, 1.0, 1.0);
  chart->GetAxis(vtkAxis::BOTTOM)
      ->GetLabelProperties()
      ->SetColor(1.0, 1.0, 1.0);
  chart->GetAxis(vtkAxis::RIGHT)->GetLabelProperties()->SetColor(1.0, 1.0, 1.0);
  chart->GetAxis(vtkAxis::TOP)->GetLabelProperties()->SetColor(1.0, 1.0, 1.0);
  chart->GetAxis(vtkAxis::LEFT)->GetTitleProperties()->SetColor(1.0, 1.0, 1.0);
  chart->GetAxis(vtkAxis::BOTTOM)
      ->GetTitleProperties()
      ->SetColor(1.0, 1.0, 1.0);

  auto colors = vtkSmartPointer<vtkNamedColors>::New();
  auto color_curve = colors->GetColor3d("banana");

  vtkPlot *line = chart->AddPlot(vtkChart::LINE);
  line->SetInputData(table, 0, 1);
  line->SetColorF(color_curve.GetRed(), color_curve.GetGreen(),
                  color_curve.GetBlue());

  line->SetWidth(1.0);

  // Scene, actor:
  renderer = vtkRenderer::New();

  auto scene = vtkSmartPointer<vtkContextScene>::New();

  scene->AddItem(chart);
  scene->SetRenderer(renderer);
  scene->Modified();

  auto actor = vtkSmartPointer<vtkContextActor>::New();
  actor->SetScene(scene);

  auto style = vtkSmartPointer<vtkContextInteractorStyle>::New();
  style->SetScene(scene);

  interactor = new MyInteractor(this, nullptr, style);
  interactor->GetRenderWindow()->AddRenderer(renderer);

  renderer->SetBackground(0, 0, 0);
  renderer->AddViewProp(actor);

  interactor->GetRenderWindow()->Render();

  interactor->Initialize();
  interactor->Start();

  auto backend = interactor->GetRenderWindow()->GetRenderingBackend();
  DEBUG_MESSAGE("My2DPanel::My2DPanel, backend = %s\n", backend);

  parent->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &My2DPanel::OnTabChange, this);
  parent->Bind(wxEVT_LISTBOOK_PAGE_CHANGED, &My2DPanel::OnTabChange, this);

  Bind(wxEVT_PAINT, &My2DPanel::OnRender, this);
  Bind(wxEVT_SIZE, &My2DPanel::OnResize, this);

  Bind(wxEVT_MOTION, &My2DPanel::OnMouseMove, this);
  Bind(wxEVT_MOUSEWHEEL, &My2DPanel::OnMouseScroll, this);
  Bind(wxEVT_LEFT_DOWN, &My2DPanel::OnMouseDown, this);
  Bind(wxEVT_LEFT_UP, &My2DPanel::OnMouseUp, this);
  Bind(wxEVT_RIGHT_DOWN, &My2DPanel::OnMouseRightDown, this);
  Bind(wxEVT_RIGHT_UP, &My2DPanel::OnMouseRightUp, this);
}

My2DPanel::~My2DPanel() {
  DEBUG_MESSAGE("%s", "My2DPanel::~My2DPanel");
  renderer->Delete();
  interactor->Delete();
}

void My2DPanel::OnRender(wxPaintEvent &event) {
  auto rw = interactor->GetRenderWindow();

  DEBUG_MESSAGE("My2DPanel::OnRender, rw = %p, native = %p", rw,
                rw->GetGenericWindowId());
  rw->Render();
}

void My2DPanel::OnResize(wxSizeEvent &evt) {
  auto w = evt.GetSize().GetWidth();
  auto h = evt.GetSize().GetHeight();
  DEBUG_MESSAGE("My2DPanel::OnResize %d x %d", w, h);
  interactor->UpdateSize(w, h);
  Refresh();
}

void My2DPanel::OnTabChange(wxBookCtrlEvent &event) {
  DEBUG_MESSAGE("%s %p", "MyFrame::OnTabChange", this);
  // interactor->GetRenderWindow()->Initialize();
  // interactor->Initialize();
  // interactor->GetRenderWindow()->WindowRemap();
}

void My2DPanel::OnMouseDown(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My2DPanel::OnMouseDown", this);

  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);

  Invoke(vtkCommand::LeftButtonPressEvent);
}

void My2DPanel::OnMouseUp(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My2DPanel::OnMouseUp", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::LeftButtonReleaseEvent);
}

void My2DPanel::OnMouseRightDown(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My2DPanel::OnMouseRightDown", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::RightButtonPressEvent);
}

void My2DPanel::OnMouseRightUp(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My2DPanel::OnMouseRightUp", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::RightButtonReleaseEvent);
}

void My2DPanel::OnMouseMove(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My2DPanel::OnMouseMove", this);
  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  Invoke(vtkCommand::MouseMoveEvent);
}

void My2DPanel::OnMouseScroll(wxMouseEvent &event) {
  DEBUG_MESSAGE("%s %p", "My2DPanel::OnMouseScroll", this);

  interactor->SetEventInformationFlipY(event.GetX(), event.GetY(),
                                       event.ControlDown(), event.ShiftDown(),
                                       '\0', 0, nullptr);
  auto command = event.GetWheelRotation() > 0
                     ? vtkCommand::MouseWheelForwardEvent
                     : vtkCommand::MouseWheelBackwardEvent;
  Invoke(command);
}

void My2DPanel::Invoke(unsigned long event, void *callData) {
  interactor->InvokeEvent(event, callData);
  Refresh();
}
