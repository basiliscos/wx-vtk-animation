#pragma once

#include <stdio.h>
#include <wx/wx.h>

#define DEBUG_MESSAGE(format, ...) fprintf(stderr, format "\n", __VA_ARGS__)

namespace utils {

void *NativeHandle(wxWindow *window);

}
