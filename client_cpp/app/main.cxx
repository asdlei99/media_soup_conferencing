#include <iostream>
#include <mediasoup/include/Device.hpp>
#include <cassert>
#include <memory>
#include <json.hpp>
#include "websocket_signaller.h"
#include "json_parser.h"
#include <future>
#include "util.h"

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

constexpr const char* signalling_port = "8081";
constexpr const char* signalling_add = "52.14.119.40";


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
	util::async_close_room(room_id, signalling_add, signalling_port,
		[](const std::string okay) {
		std::cout << "room close response " << okay << '\n';
	});
	//grt::websocket_signaller signaller;
	//signalling_callback clb{ &signaller };

	//signaller.connect(signalling_add, signalling_port, &clb);

	//auto device = std::make_unique<mediasoupclient::Device>();
	//assert(device.get());
	//assert(false == device->IsLoaded());
	////todo: next task is to get rtpCapablitities of router 
	//// therefore signalling support needs to be given
	//async_get_router_capablity([ptr = device.get()](const json capabity){
	//	ptr->Load(capabity);
	//	assert(ptr->IsLoaded());
	//	std::cout << "device is loaded\n";
	//});

	//const auto& capablity=	device->GetRtpCapabilities();
	std::cout << "done\n";
	int i;
	std::cin >> i;
}