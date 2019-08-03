#ifndef _VIDEO_RENDER_UTIL_H__
#define _VIDEO_RENDER_UTIL_H__
#include "media_receiver/video_receiver/video_track_receiver.h"
//#include <Windows.h>
#include <string>

namespace util {
	bool set_video_renderer(grt::video_track_receiver*, std::string const& renderer_id);
	void async_set_video_renderer(grt::video_track_receiver*, std::string const& renderer_id);
	//HWND create_rendering_window(HINSTANCE hInstance, WNDPROC wndproc);
	//HWND find_child_window(const wchar_t* className, const wchar_t* parent_wnd_name, const wchar_t* child_wnd_name);
}



#endif//_VIDEO_RENDER_UTIL_H__