#include "conference_handler.h"
#include "websocket_signaller.h"
#include <iostream>
#include "peer_connection/peerConnectionUtils.hpp"

namespace util {
	//windows util 
#define WNDCLASS_NAME L"Sample Window Class"
#define WND_NAME L"Learn to Program Windows"

	HWND get_window_handle() {
		//do {
			auto hwd = FindWindow(
				WNDCLASS_NAME,
				WND_NAME
			);
			assert(hwd);
			return hwd;
		/*	if (hwd) break;
			std::this_thread::sleep_for(std::chrono::seconds(10));
		} while (true);

		assert(false);*/
	}

}

namespace grt {
	std::unique_ptr< video_frame_callback>
		get_frame_receiver(HWND hwnd, std::unique_ptr< renderer>&& render) {
		return std::make_unique< video_receiver>(hwnd, std::move(render));
	}
	media_soup_conference_handler::media_soup_conference_handler(grt::signaller* signaller)
		:signaller_{ signaller } {
		assert(signaller_);
	}

	void media_soup_conference_handler::on_message(grt::message_type type, absl::any msg){

		if (grt::message_type::router_capablity == type) {
			const auto cap = absl::any_cast<json>(msg);
			std::cout << "\n\nrouter capablity " << cap << '\n';
			assert(!device_.IsLoaded());
			device_.Load(cap);
			assert(device_.IsLoaded());
			assert(device_.CanProduce("video"));
			assert(device_.CanProduce("audio"));

			//crate producer transport on server

			const auto& rtp = device_.GetRtpCapabilities();
			const auto m = grt::make_producer_transport_creation_req(false, rtp);
			signaller_->send(m);

			//create consumer transport on server
			{
				const auto m = grt::make_consumer_transport_creation_req(false);
				signaller_->send(m);
			}


		}

		else if (grt::message_type::producer_transport_res == type) {
			TransportRemoteParameters = absl::any_cast<json>(msg);
			const std::string id = TransportRemoteParameters["id"];
			const auto iceParameters = TransportRemoteParameters["iceParameters"];
			const auto iceCandidates = TransportRemoteParameters["iceCandidates"];
			const auto dtlsParameters = TransportRemoteParameters["dtlsParameters"];

			std::cout << "producer transport id " << id << '\n';
			std::cout << "iceparameters " << iceParameters << '\n';
			std::cout << "iceCandidates " << iceCandidates << '\n';
			std::cout << "dtls parameters " << dtlsParameters << '\n';
			assert(send_transport_.get() == nullptr);
			send_transport_.reset(device_.CreateSendTransport(this, id, iceParameters,
				iceCandidates, dtlsParameters));
			assert(send_transport_.get());

			assert(send_transport_->GetId() == id);
			std::cout << "connection state in transport " << send_transport_->GetConnectionState() << '\n';
			assert(send_transport_->IsClosed() == false);

			assert(videoTrack_.get() == nullptr);
			videoTrack_ = createVideoTrack("video-track-id");
			assert(videoTrack_.get());


			std::vector<webrtc::RtpEncodingParameters> encodings;
			encodings.emplace_back(webrtc::RtpEncodingParameters());
			encodings.emplace_back(webrtc::RtpEncodingParameters());
			encodings.emplace_back(webrtc::RtpEncodingParameters());

			assert(videoProducer_.get() == nullptr);
			videoProducer_.reset(
				send_transport_->Produce(this, videoTrack_, &encodings, nullptr));

		}
		else if (grt::message_type::produce_res == type) {
			const auto id_j = absl::any_cast<json>(msg);
			producer_response_.set_value(id_j["id"]);
		}
		else if (grt::message_type::consumer_create_res == type) {
			const auto param = absl::any_cast<json>(msg);
			const std::string id = param["id"];
			const auto iceParameters = param["iceParameters"];
			const auto iceCandidates = param["iceCandidates"];
			const auto dtlsParameters = param["dtlsParameters"];
			auto transport = device_.CreateRecvTransport(consumer_handler_.get(), id,
				iceParameters, iceCandidates, dtlsParameters);
			consumer_handler_->set_transport(transport);

		}
		else if (grt::message_type::peer_add == type) {
			const auto peer_id = absl::any_cast<std::string>(msg);
			//todo: check if consumer is already created for this peer id.
			const auto m = grt::make_consume_req(peer_id, device_.GetRtpCapabilities());
			signaller_->send(m);
		}
		else if (grt::message_type::consumer_res == type) {
			const auto m = absl::any_cast<json>(msg);
			assert(consumer_handler_);
			consumer_handler_->consumer(m);
			//assert(false);

		}
		else if (grt::message_type::consumer_connect_res == type) {
			assert(consumer_handler_);
			consumer_handler_->consumer_connect_res(true);
		}
		else assert(false);
	}

	///mediasoupclient::SendTransport::Listener interface
	std::future<std::string> 
		media_soup_conference_handler::OnProduce(
		const std::string& kind, nlohmann::json rtpParameters, const nlohmann::json& appData){

		const auto m = grt::make_produce_transport_req(send_transport_->GetId(), kind, rtpParameters);
		signaller_->send(m);

		producer_response_ = std::promise<std::string>{};
		return producer_response_.get_future();
	}

	std::future<void>
		media_soup_conference_handler::OnConnect(mediasoupclient::Transport* transport,
			const nlohmann::json& dtlsParameters){

		const auto m = grt::make_producer_transport_connect_req(transport->GetId(), dtlsParameters);
		signaller_->send(m);
		//assert(false);
		std::promise<void> promise;

		promise.set_value();

		return promise.get_future();
	}

	void
		media_soup_conference_handler::OnConnectionStateChange(mediasoupclient::Transport* transport,
			const std::string& connectionState) {

	}

	//Producer::Listener interfaces
	void 
		media_soup_conference_handler::OnTransportClose(mediasoupclient::Producer* producer) {

	}


	consumer_handler::consumer_handler(grt::signaller* signaller)
		:signaller_{ signaller } {
		assert(signaller_);
	}

	void consumer_handler::set_transport(mediasoupclient::RecvTransport* transport) {
		assert(transport_.get() == nullptr);
		transport_.reset(transport);
		assert(transport_);
	}

	void consumer_handler::set_video_renderer() {
		assert(video_receiver_);
		auto hwnd = util::get_window_handle();
		//auto renderer = get_renderer();
		auto frame_receiver = get_frame_receiver(hwnd, get_renderer());
		video_receiver_->register_callback(std::move(frame_receiver));
	}

	void consumer_handler::consumer(json const& data) {
		assert(transport_);
		const std::string kind = data["kind"];

		auto* consumer = transport_->Consume(this, data["id"], data["producerId"], kind,
			&data["rtpParameters"]);
		assert(consumer);
		if (kind == "audio") {
			assert(!audioConsumer_);
			audioConsumer_.reset(consumer);
			auto* audio_track = audioConsumer_->GetTrack();
			assert(audio_track);//todo: handle this 
		}
		else if (kind == "video") {
			assert(!videoConsumer_);
			videoConsumer_.reset(consumer);
			auto* video_track = videoConsumer_->GetTrack();
			assert(video_receiver_.get() == nullptr);
			video_receiver_ = get_receiver(video_track);
			
			assert(video_track);//todo: handle this to render 
			this->set_video_renderer();
		}
		else
			assert(false);

		//now ask to server to resume track
		const auto id = consumer->GetId();
		const auto m = grt::make_consumer_resume_req(id);
		this->signaller_->send(m);

	}

	void consumer_handler::consumer_connect_res(bool status) {
		assert(status);
		consumer_transport_connect_response_.set_value();
	}

	//RecvTransport::Listener
	std::future<void>
		consumer_handler::OnConnect(mediasoupclient::Transport* transport,
			const nlohmann::json& dtlsParameters)
	{
		std::promise<void> promise;
		
		consumer_transport_connect_response_ = std::move(promise);
		const auto m = make_consumer_trasport_connect_req(transport->GetId(), dtlsParameters);
		this->signaller_->send(m);
	
		

		//promise.set_value();

		return consumer_transport_connect_response_.get_future();
	}

	void
		consumer_handler::OnConnectionStateChange(mediasoupclient::Transport* transport,
			const std::string& connectionState) {
		//assert(false);

	}

	//consumer::Listener interfaces
	void consumer_handler::OnTransportClose(mediasoupclient::Consumer* consumer) {}


}//namespace grt