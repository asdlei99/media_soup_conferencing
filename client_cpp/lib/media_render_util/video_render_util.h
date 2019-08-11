#ifndef _VIDEO_RENDER_UTIL_H__
#define _VIDEO_RENDER_UTIL_H__
#include <string>

namespace grt {
	class sender;
	class video_track_receiver;
}
namespace util {
	void async_set_video_renderer(grt::video_track_receiver*, grt::sender* sender, std::string const& id);
}



#endif//_VIDEO_RENDER_UTIL_H__