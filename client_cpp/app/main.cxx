#include <iostream>
#include <mediasoup/include/Device.hpp>
#include <cassert>
#include <memory>
#include <json.hpp>
#include "websocket_signaller.h"
#include "json_parser.h"
#include <future>
#include "util.h"
#include "media_soup_conferencing_signalling.h"

using json = nlohmann::json;

constexpr const char* signalling_port = "8081";
constexpr const char* room_join_port = "8888";
constexpr const char* signalling_add = "52.14.119.40";

class media_soup_conference_handler : public grt::parser_callback {
private:
	mediasoupclient::Device device_;
public:
	void on_message(grt::message_type type, absl::any msg) override {
		
		if (grt::message_type::router_capablity == type) {
			const auto cap = absl::any_cast<json>(msg);
			std::cout << "\n\nrouter capablity " << cap << '\n';
			assert(!device_.IsLoaded());
			device_.Load(cap);
			assert(device_.IsLoaded());
		}
		else
		assert(false);
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
	auto media_soup_confernce = std::make_unique< media_soup_conference_handler>();
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