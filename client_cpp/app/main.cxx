#include <iostream>
#include <mediasoup/include/Device.hpp>
#include <mediasoup/include/mediasoupclient.hpp>
#include <cassert>
#include <memory>
#include <json.hpp>
#include "websocket_signaller.h"
#include "json_parser.h"
#include <future>
#include "util.h"
#include "media_soup_conferencing_signalling.h"
#include "peer_connection/peerConnectionUtils.hpp"

using json = nlohmann::json;

constexpr const char* signalling_port = "8081";
constexpr const char* room_join_port = "8888";
constexpr const char* signalling_add = "52.14.119.40";


class consumer_handler : public mediasoupclient::RecvTransport::Listener
, public mediasoupclient::Consumer::Listener{
private:
	std::unique_ptr<mediasoupclient::RecvTransport> transport_;
	std::unique_ptr<mediasoupclient::Consumer> audioConsumer_;
	std::unique_ptr<mediasoupclient::Consumer> videoConsumer_;
	grt::signaller* signaller_{ nullptr };
public:
	consumer_handler(grt::signaller* signaller)
		:signaller_{ signaller } {
		assert(signaller_);
	}

	void set_transport(mediasoupclient::RecvTransport* transport) {
		assert(transport_.get() == nullptr);
		transport_.reset(transport);
		assert(transport_);
	}

	void consumer(json const& data) {
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
			assert(video_track);//todo: handle this to render 
		}
		else
			assert(false);

		//now ask to server to resume track
		const auto id = consumer->GetId();
		const auto m = grt::make_consumer_resume_req(id);
		this->signaller_->send(m);

	}
	//RecvTransport::Listener
	std::future<void>
		OnConnect(mediasoupclient::Transport* transport,
			const nlohmann::json& dtlsParameters) override
	{
		assert(false);

		//const auto m = grt::make_producer_transport_connect_req(transport->GetId(), dtlsParameters);
		//signaller_->send(m);
		////assert(false);
		std::promise<void> promise;

		promise.set_value();

		return promise.get_future();
	}

	void
		OnConnectionStateChange(mediasoupclient::Transport* transport,
			const std::string& connectionState) override {
		assert(false);

	}

	//consumer::Listener interfaces
	void OnTransportClose(mediasoupclient::Consumer* consumer) override{

	}

};

class media_soup_conference_handler : public grt::parser_callback, 
	public mediasoupclient::SendTransport::Listener, public mediasoupclient::Producer::Listener{
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
	media_soup_conference_handler(grt::signaller* signaller)
		:signaller_{ signaller } {
		assert(signaller_);
	}

	void on_message(grt::message_type type, absl::any msg) override {
		
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

		}
		else assert(false);
	}

	///mediasoupclient::SendTransport::Listener interface
	std::future<std::string> OnProduce(
		const std::string& kind, nlohmann::json rtpParameters, const nlohmann::json& appData) override {

		const auto m = grt::make_produce_transport_req(send_transport_->GetId(), kind, rtpParameters);
		signaller_->send(m);
		
		producer_response_ = std::promise<std::string>{};
		return producer_response_.get_future();
	}

	std::future<void> 
		OnConnect(mediasoupclient::Transport* transport,
			const nlohmann::json& dtlsParameters) override {

		const auto m = grt::make_producer_transport_connect_req(transport->GetId(), dtlsParameters);
		signaller_->send(m);
		//assert(false);
		std::promise<void> promise;

		promise.set_value();

		return promise.get_future();
	}

	void 
		OnConnectionStateChange(mediasoupclient::Transport* transport,
			const std::string& connectionState) override {

	}

	//Producer::Listener interfaces
	void OnTransportClose(mediasoupclient::Producer* producer) override {

	}
};


int main() {
	std::cout << "hi mediasoup project\n";
	
	std::promise<std::string> room_id_getter;
	auto result = room_id_getter.get_future();

	util::async_get_room_id("anil_room", signalling_add, signalling_port, 
		[&room_id_getter](const std::string id) {
		std::cout << "id received " << id << '\n';
		room_id_getter.set_value(id);
	});

	const auto room_id = result.get();

	std::promise<std::shared_ptr<grt::signaller>> media_soup_room_signaller_getter;
	auto signaller_room = media_soup_room_signaller_getter.get_future();
	util::async_join_room(room_id, "anil", signalling_add, room_join_port,
		[&media_soup_room_signaller_getter](std::shared_ptr<grt::signaller> status){
		std::cout << "room join response\n";
		assert(status);
		media_soup_room_signaller_getter.set_value(status);
		});
	auto room_signaller = signaller_room.get();

	//make 
	auto media_soup_confernce = std::make_unique< media_soup_conference_handler>(room_signaller.get());
	auto signaller_callback = std::make_unique<grt::media_soup_signalling_cbk>(
		room_signaller.get(), std::move(media_soup_confernce));
	room_signaller->set_callback(std::move(signaller_callback));
	{
		const auto m = grt::make_router_capablity_req();
		room_signaller->send(m);
	}

	//clean up
	std::cout << "going to close\n";
	int i;
	std::cin >> i;
	util::async_close_room(room_id, signalling_add, signalling_port,
		[](const std::string okay) {
		std::cout << "room close response " << okay << '\n';
	});
	
	std::cout << "done\n";
	std::cin >> i;
}