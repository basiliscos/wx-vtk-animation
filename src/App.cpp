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

#include <wx/evtloop.h>
#include <wx/slider.h>
#include <wx/wx.h>

#include <wx/listbook.h>
#include <wx/notebook.h>

#if !defined(__WXMSW__)
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#endif

/*****************************************************************************/

namespace utils {

// there is a need to get native window id to let it be OpenGL'ized by vtk

void *NativeHandle(wxWindow *window) {
  auto handle = window->GetHandle();
#if defined(__WXMSW__)
  return reinterpret_cast<void *>(handle);
#elif defined(__WXGTK20__)
  gtk_widget_realize(handle); // otherwise xwindow id will can be null
  // gtk_widget_set_double_buffered(handle, false);
  GdkWindow *gdk = gtk_widget_get_window(GTK_WIDGET(handle));
  return reinterpret_cast<void *>(GDK_WINDOW_XID(gdk));
#else
#error "TODO: add platform support"
#endif
}

/*****************************************************************************/

} // namespace utils

/*****************************************************************************/

#define DEBUG_MESSAGE(format, ...) fprintf(stderr, format "\n", __VA_ARGS__)

/*****************************************************************************/

class MyApp : public wxApp {
public:
  using Parent = wxApp;

  bool OnInit() override;
  int OnExit() override;
};

/*****************************************************************************/

struct MyPanel;

/*****************************************************************************/

class MyFrame : public wxFrame {
public:
  MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size);

private:
  wxSlider *slider;
  wxButton *button;
  MyPanel *panel;
  vtkSmartPointer<vtkAnimationScene> scene;
};

/*****************************************************************************/

struct MyInteractor : vtkRenderWindowInteractor {
  using Parent = vtkRenderWindowInteractor;

  MyInteractor(wxPanel *panel);
  ~MyInteractor();
  void Initialize() override;
  void UpdateSize(int x, int y) override;

private:
  wxPanel *panel; // wx widget to be OpenGL'ized
};

/*****************************************************************************/

// Proxies wx-events to VTK events
struct MyPanel : public wxPanel {
  using Parent = wxPanel;

  MyPanel(wxWindow *parent, const char *sphere_color);
  ~MyPanel();

  void OnRender(wxPaintEvent &event);
  void OnResize(wxSizeEvent &evt);
  void OnTabChange(wxBookCtrlEvent &);

  MyInteractor *interactor;
  vtkRenderer *renderer;

  DECLARE_EVENT_TABLE()
};

/*****************************************************************************/

BEGIN_EVENT_TABLE(MyPanel, wxPanel)
EVT_PAINT(MyPanel::OnRender)
EVT_SIZE(MyPanel::OnResize)
END_EVENT_TABLE()

/*****************************************************************************/

bool MyApp::OnInit() {
  DEBUG_MESSAGE("%s", "MyApp::OnInit");

  MyFrame *frame =
      new MyFrame("Hello World", wxPoint(50, 50), wxSize(450, 340));
  frame->Show(true);
  return true;
}

/*****************************************************************************/

int MyApp::OnExit() {
  DEBUG_MESSAGE("%s", "MyApp::OnExit");
  return Parent::OnExit();
}

/*****************************************************************************/

MyFrame::MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
    : wxFrame(NULL, wxID_ANY, title, pos, size) {
  auto sizer = new wxBoxSizer(wxVERTICAL);
  auto tabs = new wxListbook(this, wxID_HIGHEST + 1, wxDefaultPosition,
                             wxDefaultSize, wxNB_TOP);

  auto page_0 = new MyPanel(tabs, "Green");
  auto page_1 = new MyPanel(tabs, "Red");
  auto page_2 = new MyPanel(tabs, "Cyan");

  tabs->AddPage(page_0, _("page_0"), true);
  tabs->AddPage(page_1, _("page_1"), false);
  tabs->AddPage(page_2, _("page_2"), false);

  sizer->Add(tabs, 1, wxEXPAND);

  SetSizerAndFit(sizer);
  PostSizeEvent();
}

/*****************************************************************************/

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

/*****************************************************************************/

MyInteractor::MyInteractor(wxPanel *panel_) : panel{panel_} {
  this->RenderWindow = NULL;
  this->SetRenderWindow(vtkRenderWindow::New());
  // this->RenderWindow->Delete();
  DEBUG_MESSAGE("%s", "MyInteractor::MyInteractor()");
}

/*****************************************************************************/

MyInteractor::~MyInteractor() {
  DEBUG_MESSAGE("%s", "MyInteractor::~MyInteractor");
}

/*****************************************************************************/

void MyInteractor::Initialize() {
  DEBUG_MESSAGE("%s", "MyInteractor::Initialize()");
  Parent::Initialize();
  RenderWindow->SetWindowId(utils::NativeHandle(panel));
  RenderWindow->SetParentId(utils::NativeHandle(panel->GetParent()));
  RenderWindow->SetDisplayId(RenderWindow->GetGenericDisplayId());

  int *size = RenderWindow->GetSize();
  DEBUG_MESSAGE("MyInteractor::Initialize(), sz = %u x %u", size[0], size[1]);

  Size[0] = size[0];
  Size[1] = size[1];
}

/*****************************************************************************/

void MyInteractor::UpdateSize(int w, int h) {
  if (w != Size[0] || h != Size[1]) {
    DEBUG_MESSAGE("MyInteractor::UpdateSize(), sz = %u x %u", w, h);
    Size[0] = w;
    Size[1] = h;
    RenderWindow->SetSize(w, h);
    panel->Refresh();
  }
}

/*****************************************************************************/

wxIMPLEMENT_APP(MyApp);
