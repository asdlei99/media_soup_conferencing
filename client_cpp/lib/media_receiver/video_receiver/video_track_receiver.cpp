#include "video_track_receiver.h"


namespace util {
	grt::yuv_frame convert_to_yuv_frame(const webrtc::VideoFrame& frame) {
		const auto width = frame.width();
		const auto height = frame.height();
		auto buffer = frame.video_frame_buffer();
		auto yuv = buffer->GetI420();
		const auto* y = yuv->DataY();
		const auto*u = yuv->DataU();
		const auto* v = yuv->DataV();
		const auto stridey = yuv->StrideY();
		const auto strideu = yuv->StrideU();
		const auto stridev = yuv->StrideV();
		return grt::yuv_frame{ y, u, v, stridey, strideu, stridev, width, height };
	}
}

namespace grt {

	video_track_receiver::video_track_receiver(webrtc::VideoTrackInterface* track_interface)
		:rendered_track_{ track_interface } {
		rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants{});
	}

	video_track_receiver::~video_track_receiver() {
		rendered_track_->RemoveSink(this);
	}

	void video_track_receiver::register_callback(
		std::unique_ptr<video_frame_callback> callback) {
		callback_ = std::move(callback);
	}

	void video_track_receiver::OnFrame(const webrtc::VideoFrame& frame) {
		if (callback_) {
			callback_->on_frame(util::convert_to_yuv_frame(frame));
		}
			
	}

	void video_track_receiver::OnDiscardedFrame() {

	}

	std::unique_ptr< video_track_receiver>
		get_receiver(webrtc::MediaStreamTrackInterface* stream_track) {
		assert(stream_track);
		return std::make_unique<video_track_receiver>(
			static_cast<webrtc::VideoTrackInterface*>(stream_track));
	}

}//namespace grt