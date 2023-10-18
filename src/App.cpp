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

#include <wx/wx.h>
#include <wx/slider.h>
#include <wx/evtloop.h>

#if !defined(__WXMSW__)
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#endif

#include <bit>

// adopted from https://examples.vtk.org/site/Cxx/Utilities/AnimationScene/

/*****************************************************************************/

namespace utils {

// there is a need to get native window id to let it be OpenGL'ized by vtk

void* NativeHandle(wxWindow* window)
{
	auto handle = window->GetHandle();
#if defined(__WXMSW__)
	return reinterpret_cast<void*>(handle);
#elif defined(__WXGTK20__)
	gtk_widget_realize(handle); // otherwise xwindow id will can be null
	//gtk_widget_set_double_buffered(handle, false);
	GdkWindow* gdk = gtk_widget_get_window(GTK_WIDGET(handle));
	return reinterpret_cast<void*>(GDK_WINDOW_XID(gdk));
#else
#error "TODO: add platform support"
#endif
}

/*****************************************************************************/

} // namespace

/*****************************************************************************/

#define DEBUG_MESSAGE(format, ...) fprintf(stderr, format "\n", __VA_ARGS__)

/*****************************************************************************/

wxDEFINE_EVENT(ANIMATION_FINISHED, wxCommandEvent);
wxDEFINE_EVENT(ANIMATION_TICK, wxCommandEvent);

/*****************************************************************************/

class CueAnimator
{
public:

/*****************************************************************************/

  CueAnimator(wxWindow* window)
  {
	this->SphereSource = 0;
	this->Mapper = 0;
	this->Actor = 0;
	this->widget = window;
  }

/*****************************************************************************/

  ~CueAnimator()
  {
	this->Cleanup();
  }

/*****************************************************************************/

  void StartCue(vtkAnimationCue::AnimationCueInfo* vtkNotUsed(info),
				vtkRenderer* ren)
  {
	vtkNew<vtkNamedColors> colors;

	this->SphereSource = vtkSphereSource::New();
	this->SphereSource->SetRadius(0.5);

	this->Mapper = vtkPolyDataMapper::New();
	this->Mapper->SetInputConnection(this->SphereSource->GetOutputPort());

	this->Actor = vtkActor::New();
	this->Actor->SetMapper(this->Mapper);
	this->Actor->GetProperty()->SetSpecular(0.6);
	this->Actor->GetProperty()->SetSpecularPower(30);
	this->Actor->GetProperty()->SetColor(
		colors->GetColor3d("Peacock").GetData());

	ren->AddActor(this->Actor);
	ren->ResetCamera();
	ren->Render();

	// process wx events
	WxTick();
  }

/*****************************************************************************/

  void Tick(vtkAnimationCue::AnimationCueInfo* info, vtkRenderer* ren)
  {
	double newradius = 0.1 +
		(static_cast<double>(info->AnimationTime - info->StartTime) /
		 static_cast<double>(info->EndTime - info->StartTime)) *
			1;
	this->SphereSource->SetRadius(newradius);
	this->SphereSource->Update();
	ren->Render();

	// notify percentage for wx
	wxCommandEvent event(ANIMATION_TICK, widget->GetId());
	double percent = static_cast<long>(100 * info->AnimationTime / (info->EndTime - info->StartTime));
	event.SetExtraLong(percent);
	widget->ProcessWindowEvent(event);

	// process wx events
	WxTick();
  }

/*****************************************************************************/

  void EndCue(vtkAnimationCue::AnimationCueInfo* vtkNotUsed(info),
			  vtkRenderer* ren)
  {
	(void)ren;
	this->Cleanup();

	// notify finishing
	wxCommandEvent event(ANIMATION_FINISHED, widget->GetId());
	widget->ProcessWindowEvent(event);

	// process wx events
	WxTick();
  }

/*****************************************************************************/

protected:

/*****************************************************************************/

  vtkSphereSource* SphereSource;
  vtkPolyDataMapper* Mapper;
  vtkActor* Actor;
  wxWindow* widget;

  // inner loop for allowing wx events to be processed during animation
  // this is needed because wx event loop is NOT REENTERABLE while
  // VTK animation is synchronous
  wxGUIEventLoop loop;

/*****************************************************************************/

  void Cleanup()
  {
	if (this->SphereSource != 0)
	{
	  this->SphereSource->Delete();
	  this->SphereSource = 0;
	}

	if (this->Mapper != 0)
	{
	  this->Mapper->Delete();
	  this->Mapper = 0;
	}
	if (this->Actor != 0)
	{
	  this->Actor->Delete();
	  this->Actor = 0;
	}
  }

/*****************************************************************************/

  void WxTick()
  {
	  // activate inner event loop
	  wxEventLoopActivator activate(&loop);
	  while (loop.Pending())
	  {
		  loop.Dispatch();
	  }
  }

/*****************************************************************************/

};

/*****************************************************************************/

class vtkAnimationCueObserver : public vtkCommand
{
public:

/*****************************************************************************/

  static vtkAnimationCueObserver* New()
  {
	return new vtkAnimationCueObserver;
  }

/*****************************************************************************/

  virtual void Execute(vtkObject* vtkNotUsed(caller), unsigned long event,
					   void* calldata)
  {
	if (this->Animator != 0 && this->Renderer != 0)
	{
	  vtkAnimationCue::AnimationCueInfo* info =
		  static_cast<vtkAnimationCue::AnimationCueInfo*>(calldata);
	  switch (event)
	  {
	  case vtkCommand::StartAnimationCueEvent:
		this->Animator->StartCue(info, this->Renderer);
		break;
	  case vtkCommand::EndAnimationCueEvent:
		this->Animator->EndCue(info, this->Renderer);
		break;
	  case vtkCommand::AnimationCueTickEvent:
		this->Animator->Tick(info, this->Renderer);
		//wxApp().ProcessPendingEvents();
		break;
	  }
	}
	if (this->RenWin != 0)
	{
	  this->RenWin->Render();
	}
  }

/*****************************************************************************/

  vtkRenderer* Renderer;
  vtkRenderWindow* RenWin;
  CueAnimator* Animator;

protected:

/*****************************************************************************/

  vtkAnimationCueObserver()
  {
	this->Renderer = 0;
	this->Animator = 0;
	this->RenWin = 0;
  }

/*****************************************************************************/

};

/*****************************************************************************/

class MyApp: public wxApp
{
public:
	using Parent = wxApp;

	bool OnInit() override;
	int OnExit() override;
};

/*****************************************************************************/

struct MyPanel;

/*****************************************************************************/

class MyFrame: public wxFrame
{
public:
	MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	void OnClick(wxCommandEvent&);
	void OnFinish(wxCommandEvent&);
	void OnTick(wxCommandEvent&);

private:
	wxSlider* slider;
	wxButton* button;
	MyPanel* panel;
	vtkSmartPointer<vtkAnimationScene> scene;
};

/*****************************************************************************/

struct MyInteractor: vtkRenderWindowInteractor
{
	using Parent = vtkRenderWindowInteractor;

	MyInteractor(wxPanel* panel);
	~MyInteractor();
	void Initialize() override;
	void UpdateSize(int x, int y) override;

	private:
	wxPanel* panel; // wx widget to be OpenGL'ized
};

/*****************************************************************************/

// Proxies wx-events to VTK events
struct MyPanel: public wxPanel
{
	using Parent = wxPanel;

	MyPanel(wxWindow* parent);
	~MyPanel();

	void OnRender(wxPaintEvent& event);
	void OnResize(wxSizeEvent& evt);

	MyInteractor* interactor;
	vtkRenderer* renderer;

	DECLARE_EVENT_TABLE()
};

/*****************************************************************************/

BEGIN_EVENT_TABLE(MyPanel, wxPanel)
	EVT_PAINT(MyPanel::OnRender)
	EVT_SIZE(MyPanel::OnResize)
END_EVENT_TABLE()

/*****************************************************************************/

bool MyApp::OnInit()
{
	DEBUG_MESSAGE("%s", "MyApp::OnInit");

	MyFrame *frame = new MyFrame( "Hello World", wxPoint(50, 50), wxSize(450, 340) );
	frame->Show( true );
	return true;
}

/*****************************************************************************/

int MyApp::OnExit()
{
	DEBUG_MESSAGE("%s", "MyApp::OnExit");
	return Parent::OnExit();
}

/*****************************************************************************/

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
		: wxFrame(NULL, wxID_ANY, title, pos, size)
{
	auto sizer = new wxBoxSizer(wxVERTICAL);
	slider = new wxSlider(this, wxID_ANY,  0, 0, 100);
	button = new wxButton(this, wxID_ANY, "play");
	panel = new MyPanel(this);
	panel->SetBackgroundColour(*wxGREEN);

	sizer->Add(slider, 0, wxEXPAND | wxALL, 3);
	sizer->Add(button, 0, wxEXPAND | wxALL, 3);
	sizer->Add(panel, 1, wxEXPAND | wxALL, 3);

	SetSizerAndFit(sizer);

	SetSize(wxSize(640, 480));

	button->Bind(wxEVT_BUTTON, &MyFrame::OnClick, this);
	Bind(ANIMATION_FINISHED, &MyFrame::OnFinish, this);
	Bind(ANIMATION_TICK, &MyFrame::OnTick, this);

	scene = vtkSmartPointer<vtkAnimationScene>::New();
}

/*****************************************************************************/

void MyFrame::OnClick(wxCommandEvent&)
{
	DEBUG_MESSAGE("%s", "MyFrame::OnClick, enter");
	if (!scene->IsInPlay())
	{

		button->SetLabel("Stop");

		scene->SetModeToRealTime();

		scene->SetLoop(0);
		scene->SetFrameRate(5);
		scene->SetStartTime(0);
		scene->SetEndTime(3);

		// Create an Animation Cue.
		vtkNew<vtkAnimationCue> cue1;
		cue1->SetStartTime(0);
		cue1->SetEndTime(3);
		scene->AddCue(cue1);

		// Create cue animator;
		CueAnimator animator(this);

		// Create Cue observer.
		vtkNew<vtkAnimationCueObserver> observer;
		observer->Renderer = panel->renderer;
		observer->Animator = &animator;
		observer->RenWin = panel->interactor->GetRenderWindow();
		//observer->event_loop = GetEventHandler();

		cue1->AddObserver(vtkCommand::StartAnimationCueEvent, observer);
		cue1->AddObserver(vtkCommand::EndAnimationCueEvent, observer);
		cue1->AddObserver(vtkCommand::AnimationCueTickEvent, observer);

		//scene->SetModeToSequence();
		DEBUG_MESSAGE("%s", "MyFrame::OnClick, play");
		scene->Play();

		DEBUG_MESSAGE("%s", "MyFrame::OnClick, stop");
		scene->Stop();

		panel->interactor->Start();
	}
	else
	{
		scene->Stop();
	}
	DEBUG_MESSAGE("%s", "MyFrame::OnClick, leave");
}

/*****************************************************************************/

void MyFrame::OnFinish(wxCommandEvent&)
{
	DEBUG_MESSAGE("%s", "MyFrame::OnFinish");
	button->SetLabel("Play");
	panel->renderer->Clear();
}

/*****************************************************************************/

void MyFrame::OnTick(wxCommandEvent& event)
{
	DEBUG_MESSAGE("%s", "MyFrame::OnTick");
	slider->SetValue(static_cast<int>(event.GetExtraLong()));
}

/*****************************************************************************/

MyPanel::MyPanel(wxWindow* parent): Parent(parent)
{
	interactor = new MyInteractor(this);
	interactor->SetRenderWindow(vtkRenderWindow::New());
	renderer = vtkRenderer::New();
	interactor->GetRenderWindow()->AddRenderer(renderer);

	renderer->SetBackground(219 / 255.0, 222 / 255.0, 248 / 255.0);
	renderer->ResetCamera();
}

/*****************************************************************************/

MyPanel::~MyPanel()
{
	DEBUG_MESSAGE("%s", "MyPanel::~MyPanel");
	renderer->Delete();
	interactor->GetRenderWindow()->Delete();
	interactor->Delete();
}

/*****************************************************************************/

void MyPanel::OnRender(wxPaintEvent& event)
{
	wxPaintDC pDC(this);
//	interactor->Render();
	interactor->GetRenderWindow()->Render();
}

/*****************************************************************************/

void MyPanel::OnResize(wxSizeEvent& evt)
{
	auto w = evt.GetSize().GetWidth();
	auto h = evt.GetSize().GetHeight();
	interactor->UpdateSize(w, h);
}

/*****************************************************************************/

MyInteractor::MyInteractor(wxPanel* panel_): panel{panel_}
{
	DEBUG_MESSAGE("%s", "MyInteractor::MyInteractor()");
}

/*****************************************************************************/

MyInteractor::~MyInteractor()
{
	DEBUG_MESSAGE("%s", "MyInteractor::~MyInteractor");
}

/*****************************************************************************/

void MyInteractor::Initialize()
{
	DEBUG_MESSAGE("%s", "MyInteractor::Initialize()");
	Parent::Initialize();
	auto handle = utils::NativeHandle(panel);
	RenderWindow->SetWindowId(reinterpret_cast<void *>(handle));
	RenderWindow->SetParentId(utils::NativeHandle(panel->GetParent()));
	RenderWindow->SetDisplayId(RenderWindow->GetGenericDisplayId());

	int *size = RenderWindow->GetSize();
	DEBUG_MESSAGE("MyInteractor::Initialize(), sz = %u x %u", size[0], size[1]);

	Size[0] = size[0];
	Size[1] = size[1];
}

/*****************************************************************************/

void MyInteractor::UpdateSize(int w, int h)
{
	if (w != Size[0] || h != Size[1])
	{
		DEBUG_MESSAGE("MyInteractor::UpdateSize(), sz = %u x %u", w, h);
		Size[0] = w;
		Size[1] = h;
		RenderWindow->SetSize(w, h);
		panel->Refresh();
	}
}

/*****************************************************************************/

wxIMPLEMENT_APP(MyApp);
