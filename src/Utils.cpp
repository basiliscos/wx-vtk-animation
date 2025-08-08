#include "Utils.h"

#if !defined(__WXMSW__)
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#endif

namespace utils {

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

} // namespace utils
