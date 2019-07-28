#ifndef _VIDEO_TRACK_RECEIVER__H_
#define _VIDEO_TRACK_RECEIVER__H_
#include "api/media_stream_interface.h"
#include "api/video/video_frame.h"
#include "video_track_receiver.h"


namespace grt {
	//todo: peer_connection itself may be made to have this
	class video_track_receiver_impl : public  rtc::VideoSinkInterface<webrtc::VideoFrame>, public video_track_receiver {
		
	public:
		video_track_receiver_impl(webrtc::VideoTrackInterface* track_interface);
		~video_track_receiver_impl();
		//video_track_receiver interfaces

		void register_callback(std::unique_ptr<video_frame_callback> callback) override;

		//rtc::VideoSinkInterface
		void OnFrame(const webrtc::VideoFrame& frame) override;
		void OnDiscardedFrame() override;

	private:
		rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
		std::unique_ptr<video_frame_callback> callback_;
	};

	std::unique_ptr< video_track_receiver>
		get_receiver(webrtc::MediaStreamTrackInterface*);

}//namespace grt


#endif//_VIDEO_TRACK_RECEIVER__H_