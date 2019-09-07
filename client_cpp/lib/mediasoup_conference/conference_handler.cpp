#include "conference_handler.h"
#include "websocket_signaller.h"
#include <iostream>
#include "peer_connection/peerConnectionUtils.hpp"
#include "video_receiver_helper/video_receiver_helper.h"

namespace grt {
	
	media_soup_conference_handler::media_soup_conference_handler(grt::signaller* signaller)
		:signaller_{ signaller } {
		assert(signaller_);
		auto future_ = sender_->sync_connect(RENDERING_SERVER_IP, RENDERING_SERVER_PORT);

		std::thread{ [future = std::move(future_)]()mutable{
			//todo: FIXMe this has to be fixed. and it is run time check as well for error case
			auto status = future.wait_for(std::chrono::seconds(5));
			assert(status != std::future_status::timeout); //if it crashes here, it means renderer is not running
			const auto connection_status = future.get();
			assert(connection_status);
		} }.detach();
	}



	media_soup_conference_handler::~media_soup_conference_handler() {
#ifdef _DEBUG
		//assert(false);
		std::cout << "media soup destruction called\n";

#endif//_DEBUG
		if (consumer_transport_) consumer_transport_->Close();
		if (videoProducer_) videoProducer_->Close();
		if (send_transport_) send_transport_->Close();
		//assert(false);
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

			auto transport = device_.CreateRecvTransport(this, id,
				iceParameters, iceCandidates, dtlsParameters);

			consumer_transport_.reset(transport);

		}
		else if (grt::message_type::peer_add == type) {
			const auto peer_id = absl::any_cast<std::string>(msg);
			assert(consumers_.find(peer_id) == consumers_.end());

			//todo: check if consumer is already created for this peer id.
			const auto m = grt::make_consume_req(peer_id, device_.GetRtpCapabilities());
			signaller_->send(m);
		}
		else if (grt::message_type::peer_remove == type) {
			const auto peer_id = absl::any_cast<std::string>(msg);
			consumers_.erase(peer_id);
		}
		else if (grt::message_type::consumer_res == type) {
			const auto m = absl::any_cast<json>(msg);
			//const std::string id = m["id"];
			const std::string peerId = m["peerId"];
			
			assert(consumers_.find(peerId) == consumers_.end());

			const std::string kind = m["kind"];
			assert(consumer_transport_);

			const auto r = consumers_.emplace(peerId, std::make_unique< consumer_handler>(sender_));
			assert(r.second);//insertion should have hapened

			auto* consumer = consumer_transport_->Consume(r.first->second.get(), m["id"], m["producerId"], kind,
				&m["rtpParameters"]);
			r.first->second->consumer(consumer, kind);

			{
				//now ask to server to resume track
				const auto id = consumer->GetId();
				const auto m = grt::make_consumer_resume_req(id);
				this->signaller_->send(m);
			}

		}
		else if (grt::message_type::consumer_connect_res == type) {
			consumer_transport_connect_response_.set_value();
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
		if (transport == send_transport_.get()) {
			const auto m = grt::make_producer_transport_connect_req(transport->GetId(), dtlsParameters);
			signaller_->send(m);
			//assert(false);
			std::promise<void> promise;

			promise.set_value();

			return promise.get_future();
		}
		else {
			//cosumer transport
			std::promise<void> promise;
				
			consumer_transport_connect_response_ = std::move(promise);
			const auto m = make_consumer_trasport_connect_req(transport->GetId(), dtlsParameters);
			this->signaller_->send(m);
			return consumer_transport_connect_response_.get_future();
		}
		
	}

	void
		media_soup_conference_handler::OnConnectionStateChange(mediasoupclient::Transport* transport,
			const std::string& connectionState) {
		std::cout << "connectin state change " << connectionState << " id =" << transport->GetId() << '\n';
	}

	//Producer::Listener interfaces
	void 
		media_soup_conference_handler::OnTransportClose(mediasoupclient::Producer* producer) {

	}

	consumer_handler::consumer_handler(std::shared_ptr<sender> sender)
		:sender_{ sender }{ assert(sender_); }

	consumer_handler::~consumer_handler() {
		if (audioConsumer_)
			audioConsumer_->Close();
		if (videoConsumer_)
			videoConsumer_->Close();
	}

	void consumer_handler::consumer(mediasoupclient::Consumer* consumer,
		std::string const& kind) {
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

			assert(video_track);//todo: handle this to render 
			//const auto r = util::set_video_renderer(video_receiver_.get());
			//assert(r);
			auto const id = consumer->GetId();
			std::cout << "video renderer id " << id << '\n';
			video_receiver_ = set_video_renderer(video_track, sender_, id);
		}
		else
			assert(false);
	}

	//consumer::Listener interfaces
	void consumer_handler::OnTransportClose(mediasoupclient::Consumer* consumer) {
		std::cout << "transport close " << consumer->GetId() << '\n';
	}


}//namespace grt