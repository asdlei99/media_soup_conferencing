#ifndef _VIDEO_RENDER_UTIL_H__
#define _VIDEO_RENDER_UTIL_H__
#include "media_receiver/video_receiver/video_track_receiver.h"
#include <Windows.h>
namespace util {
	bool set_video_renderer(grt::video_track_receiver*);
	HWND create_rendering_window(HINSTANCE hInstance, WNDPROC wndproc);
}



#endif//_VIDEO_RENDER_UTIL_H__