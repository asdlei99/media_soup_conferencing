#ifndef __MEDIA_SOUP_CONFERENCE_HANDLER_H__
#define __MEDIA_SOUP_CONFERENCE_HANDLER_H__
#include <mediasoup/include/Device.hpp>
#include <mediasoup/include/mediasoupclient.hpp>
#include "json_parser.h"
#include "media_receiver/video_receiver/video_track_receiver.h"
#include "video_render/renderer.h"
namespace grt {
	class signaller;

	class video_receiver : public video_frame_callback {
	private:
		std::shared_ptr<renderer> renderer_;
		HWND hwnd_;
	public:
		video_receiver(HWND hwnd, std::unique_ptr< renderer>&& render)
			:renderer_{ std::move(render) }, hwnd_{ hwnd }{
			auto r = SetWindowPos(hwnd_, HWND_TOPMOST,
				0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
			assert(r != 0);
		}

		~video_receiver() {

			auto r = ShowWindow(hwnd_, SW_HIDE);
			assert(r != 0);//it shows it was previously shown
		}
		void on_frame(yuv_frame frame) override {
			auto frame_info = grt::make_frame_info(
				frame.y_, frame.u_, frame.v_, frame.stride_y_,
				frame.stride_u_, frame.stride_v_, frame.w_, frame.h_);
			renderer_->render_frame(hwnd_, frame_info);
			grt::clean(frame_info);
		}
	};

	class consumer_handler : public mediasoupclient::RecvTransport::Listener
		, public mediasoupclient::Consumer::Listener {
	private:
		std::unique_ptr<mediasoupclient::RecvTransport> transport_;
		std::unique_ptr<mediasoupclient::Consumer> audioConsumer_;
		std::unique_ptr<mediasoupclient::Consumer> videoConsumer_;
		signaller* signaller_{ nullptr };
		std::promise<void> consumer_transport_connect_response_;
		std::unique_ptr< video_track_receiver> video_receiver_;
	private:
		void set_video_renderer();
	public:
		consumer_handler(grt::signaller* signaller);

		void set_transport(mediasoupclient::RecvTransport* transport);

		void consumer(json const& data);

		void consumer_connect_res(bool status);

		//RecvTransport::Listener
		std::future<void>
			OnConnect(mediasoupclient::Transport* transport,
				const nlohmann::json& dtlsParameters) override;
		void
			OnConnectionStateChange(mediasoupclient::Transport* transport,
				const std::string& connectionState) override;

		//consumer::Listener interfaces
		void OnTransportClose(mediasoupclient::Consumer* consumer) override;
	};

	//todo: this class may be  broken into producer and parser callback.
	//will do later.
	//then object of both can be kept in room object
	//room can be made as parser callback 
	class media_soup_conference_handler : public grt::parser_callback,
		public mediasoupclient::SendTransport::Listener, public mediasoupclient::Producer::Listener {
	private:
		mediasoupclient::Device device_;
		grt::signaller* signaller_{ nullptr };
		json TransportRemoteParameters;
		std::unique_ptr<mediasoupclient::SendTransport> send_transport_{ nullptr };
		std::unique_ptr<mediasoupclient::Producer> videoProducer_;
		rtc::scoped_refptr<webrtc::VideoTrackInterface> videoTrack_;
		std::promise<std::string> producer_response_;

		std::unique_ptr< consumer_handler> consumer_handler_
			= std::make_unique< consumer_handler>(signaller_);


	public:
		media_soup_conference_handler(grt::signaller* signaller);

		void on_message(grt::message_type type, absl::any msg) override;

		///mediasoupclient::SendTransport::Listener interface
		std::future<std::string> OnProduce(
			const std::string& kind, nlohmann::json rtpParameters, const nlohmann::json& appData) override;

		std::future<void>
			OnConnect(mediasoupclient::Transport* transport,
				const nlohmann::json& dtlsParameters) override;

		void
			OnConnectionStateChange(mediasoupclient::Transport* transport,
				const std::string& connectionState) override;

		//Producer::Listener interfaces
		void OnTransportClose(mediasoupclient::Producer* producer) override;
	};

}//namespace grt


#endif//__MEDIA_SOUP_CONFERENCE_HANDLER_H__