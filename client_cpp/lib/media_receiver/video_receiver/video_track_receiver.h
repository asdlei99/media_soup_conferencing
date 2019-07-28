#ifndef __VIDEO_TRACK_RECE_H_HH_
#define __VIDEO_TRACK_RECE_H_HH_
#include "video_frame_callback.h"
#include <memory>//unique_ptr

namespace grt {
	class video_track_receiver {
	public:
		virtual ~video_track_receiver() {}
		virtual void register_callback(std::unique_ptr<video_frame_callback> callback) = 0;
	};	

}//namespace grt

#endif//