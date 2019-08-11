#ifndef _VIDEO_RENDER_UTIL_H__
#define _VIDEO_RENDER_UTIL_H__
#include "media_receiver/video_receiver/video_track_receiver.h"
#include <Windows.h>
#include <string>
#include <tuple>
#include <future>
namespace util {
	bool set_video_renderer(grt::video_track_receiver*, std::string const& renderer_id);
	void async_set_video_renderer(grt::video_track_receiver*, std::string const& renderer_id);
	std::future<std::tuple<std::string, std::string, std::string>>
		async_get_rendeing_window_info(std::string server_ip, std::string server_port, std::string id);
	//HWND create_rendering_window(HINSTANCE hInstance, WNDPROC wndproc);
	
}



#endif//_VIDEO_RENDER_UTIL_H__