#ifndef __MEDIA_SOUP_CONFERENCE_HANDLER_H__
#define __MEDIA_SOUP_CONFERENCE_HANDLER_H__
#include <mediasoup/include/Device.hpp>
#include <mediasoup/include/mediasoupclient.hpp>
#include "json_parser.h"

namespace grt {
	class signaller;

	class consumer_handler : public mediasoupclient::RecvTransport::Listener
		, public mediasoupclient::Consumer::Listener {
	private:
		std::unique_ptr<mediasoupclient::RecvTransport> transport_;
		std::unique_ptr<mediasoupclient::Consumer> audioConsumer_;
		std::unique_ptr<mediasoupclient::Consumer> videoConsumer_;
		signaller* signaller_{ nullptr };
	public:
		consumer_handler(grt::signaller* signaller);

		void set_transport(mediasoupclient::RecvTransport* transport);

		void consumer(json const& data);
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