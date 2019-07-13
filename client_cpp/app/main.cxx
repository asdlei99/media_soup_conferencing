#include <iostream>
#include <cassert>
#include <memory>
#include <future>
#include "mediasoup_conference/mediasoup_conference.h"

constexpr const char* signalling_port = "8081";
constexpr const char* room_join_port = "8888";
constexpr const char* signalling_add = "52.14.119.40";




int main() {
	std::cout << "hi mediasoup project\n";
	

	auto result = grt::async_create_room("anil_room", signalling_add, signalling_port);
	const auto room_id = result.get();

	auto room_fut = grt::async_join_room("anil", room_id, signalling_add, room_join_port);
	auto room = room_fut.get();

	assert(room);
	room->enter();

	
	//clean up
	std::cout << "going to close\n";
	int i;
	std::cin >> i;
	{
		auto result = grt::async_close_room(room_id, signalling_add, signalling_port);
		std::cout << "close result " << result.get() << '\n';
	}
	
	
	std::cout << "done\n";
	std::cin >> i;
}