#include <iostream>
#include <mediasoup/include/Device.hpp>
#include <cassert>
#include <memory>
#include <json.hpp>
#include "websocket_signaller.h"
#include "json_parser.h"

using json = nlohmann::json;

namespace util {
	
	json generateRouterRtpCapabilities()
	{
		auto codecs = json::array();
		auto headerExtensions = json::array();
		auto fecMechanisms = json::array();

		codecs = R"(
	[
		{
			"mimeType"             : "audio/opus",
			"kind"                 : "audio",
			"clockRate"            : 48000,
			"preferredPayloadType" : 100,
			"channels"             : 2,
			"rtcpFeedback"         : [],
			"parameters"           :
			{
				"useinbandfec" : 1
			}
		},
		{
			"mimeType"             : "video/VP8",
			"kind"                 : "video",
			"clockRate"            : 90000,
			"preferredPayloadType" : 101,
			"rtcpFeedback"         :
			[
				{ "type": "nack" },
				{ "type": "nack", "parameter": "pli"  },
				{ "type": "nack", "parameter": "sli"  },
				{ "type": "nack", "parameter": "rpsi" },
				{ "type": "nack", "parameter": "app"  },
				{ "type": "ccm",  "parameter": "fir"  },
				{ "type": "goog-remb" }
			],
			"parameters" :
			{
				"x-google-start-bitrate" : "1500"
			}
		},
		{
			"mimeType"             : "video/rtx",
			"kind"                 : "video",
			"clockRate"            : 90000,
			"preferredPayloadType" : 102,
			"rtcpFeedback"         : [],
			"parameters"           :
			{
				"apt" : 101
			}
		},
		{
			"mimeType"             : "video/H264",
			"kind"                 : "video",
			"clockRate"            : 90000,
			"preferredPayloadType" : 103,
			"rtcpFeedback"         :
			[
				{ "type": "nack" },
				{ "type": "nack", "parameter": "pli"  },
				{ "type": "nack", "parameter": "sli"  },
				{ "type": "nack", "parameter": "rpsi" },
				{ "type": "nack", "parameter": "app"  },
				{ "type": "ccm",  "parameter": "fir"  },
				{ "type": "goog-remb" }
			],
			"parameters" :
			{
				"level-asymmetry-allowed" : 1,
				"packetization-mode"      : 1,
				"profile-level-id"        : "42e01f"
			}
		},
		{
			"mimeType"             : "video/rtx",
			"kind"                 : "video",
			"clockRate"            : 90000,
			"preferredPayloadType" : 104,
			"rtcpFeedback"         : [],
			"parameters"           :
			{
				"apt" : 103
			}
		}
	])"_json;

		headerExtensions = R"(
	[
		{
			"kind"             : "audio",
			"uri"              : "urn:ietf:params:rtp-hdrext:ssrc-audio-level",
			"preferredId"      : 1,
			"preferredEncrypt" : false
		},
		{
			"kind"             : "video",
			"uri"              : "urn:ietf:params:rtp-hdrext:toffset",
			"preferredId"      : 2,
			"preferredEncrypt" : false
		},
		{
			"kind"             : "audio",
			"uri"              : "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time",
			"preferredId"      : 3,
			"preferredEncrypt" : false
		},
		{
			"kind"             : "video",
			"uri"              : "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time",
			"preferredId"      : 3,
			"preferredEncrypt" : false
		},
		{
			"kind"             : "video",
			"uri"              : "urn:3gpp:video-orientation",
			"preferredId"      : 4,
			"preferredEncrypt" : false
		},
		{
			"kind"             : "audio",
			"uri"              : "urn:ietf:params:rtp-hdrext:sdes:mid",
			"preferredId"      : 5,
			"preferredEncrypt" : false
		},
		{
			"kind"             : "video",
			"uri"              : "urn:ietf:params:rtp-hdrext:sdes:mid",
			"preferredId"      : 5,
			"preferredEncrypt" : false
		},
		{
			"kind"             : "video",
			"uri"              : "urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id",
			"preferredId"      : 6,
			"preferredEncrypt" : false
		}
	])"_json;

		json capabilities =
		{
			{ "codecs",           codecs           },
			{ "headerExtensions", headerExtensions },
			{ "fecMechanisms",    fecMechanisms    }
		};

		return capabilities;
	};
}

//todo: get rtc capablity
template<typename CallBack>
void async_get_router_capablity(CallBack callback) {
	std::thread{ [callback]() {
	const auto capablity = util::generateRouterRtpCapabilities();
	callback(capablity);
	} }.detach();
}

class signalling_callback : public grt::signaller_callback, 
	grt::parser_callback {

private:
	grt::signaller* sender_{ nullptr };
public:
	std::string id_;
	signalling_callback(grt::signaller* p) :sender_{ p } {
		assert(sender_);
	}

	void on_message(std::string msg) override {
		std::cout << "on message = " << msg << '\n';
		grt::async_parse_message(msg, this);
	}

	void on_connect() override {
		std::cout << "on connect called\n";
		/*const json j2 = {
			{"type", "register_user"},
			{"name", "test"}
		};
		const std::string m = j2.dump();
		std::cout << "message " << m << '\n';
		sender_->send(m);*/
		const auto m = grt::create_register_user_req("anil");
		sender_->send(m);
	}

	void on_error(std::string error) override {
		std::cout << "error occur = " << error << '\n';
		assert(false);
	}

	void on_close() override {
		std::cout << "close sigannling connection\n";
	}

	/*parser callback*/
	void
		on_user_register_response(bool isokay, std::string id) override {
		std::cout << "user register response received\n";
		std::cout << "status = " << isokay << '\n';
		std::cout << "id = " << id << '\n';
		id_ = id;
	}
};


constexpr const char* signalling_port = "1339";
constexpr const char* signalling_add = "localhost";

int main() {
	std::cout << "hi mediasoup project\n";

	grt::websocket_signaller signaller;
	signalling_callback clb{ &signaller };

	signaller.connect(signalling_add, signalling_port, &clb);

	auto device = std::make_unique<mediasoupclient::Device>();
	assert(device.get());
	assert(false == device->IsLoaded());
	//todo: next task is to get rtpCapablitities of router 
	// therefore signalling support needs to be given
	async_get_router_capablity([ptr = device.get()](const json capabity){
		ptr->Load(capabity);
		assert(ptr->IsLoaded());
		std::cout << "device is loaded\n";
	});

	//const auto& capablity=	device->GetRtpCapabilities();
	std::cout << "done\n";
	int i;
	std::cin >> i;
}