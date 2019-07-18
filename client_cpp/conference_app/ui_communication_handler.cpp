#include "ui_communication_handler.h"
#include <cassert>
#include "websocket_server/websocket_server.h"
#include "mediasoup_conference/mediasoup_conference.h"
#include <iostream>

#define UNIT_TEST //todo undef this when real media soup server running and actual connection is inteneded

namespace grt {


	static util::func_thread_handler func_object;
	util::func_thread_handler* get_func_thread() {
		return &func_object;
	}

	ui_communication_handler::~ui_communication_handler() = default;

	void ui_communication_handler::start(unsigned short port, int threads) {
		auto *func_thread = get_func_thread();
		func_thread->register_id(
			REC_ID,
			std::bind(&ui_communication_handler::on_ui_message, this, std::placeholders::_1)
		);
		grt::start_server_block(port, 1, get_func_thread(), REC_ID);
	}

	void ui_communication_handler::message_for_ui(std::string m) {
		auto *func_thread = get_func_thread();
		func_thread->dispatch(UI_SERVER_ID, m);
	}

	void ui_communication_handler::on_ui_message(std::string m) {
		async_parse_message(m, this);
	}

	void ui_communication_handler::on_message(message_type type, absl::any msg) {
		switch (type) {
		case message_type::room_join_req:
		{
#ifdef _DEBUG
			std::cout << "room_join_req " << '\n';
#endif//_DEBUG
#ifndef UNIT_TEST
			assert(room_.get() == nullptr);
			const auto room_info = absl::any_cast<room_connection_credential>(msg);
			auto room_fut = grt::async_join_room(room_info.user_name_,
				room_info.room_id_, room_info.ip_, room_info.port_);
			auto room = room_fut.get();

			assert(room);

			room->enter();
			room_ = room;
#else
			const auto res = make_room_join_req_res(true);
			auto *func_thread = get_func_thread();
			func_thread->dispatch(UI_SERVER_ID, res);

#endif//


		}
			break;
		case message_type::room_open_req:
		{
			const json m = absl::any_cast<json>(msg);
#ifdef _DEBUG
			std::cout << " room open req from ui " << m << '\n';
#endif//_DEBUG
			const std::string room_name = m["name"];
			const std::string ip = m["ip"];
			const std::string port = m["port"];
#ifndef UNIT_TEST
		    auto result =	grt::async_create_room(room_name, ip, port);
			const auto id = result.get();
#else
			const std::string id = "test";//result.get();
#endif//
			const auto res = make_room_create_req_res(true, id);
			auto *func_thread = get_func_thread();
			func_thread->dispatch(UI_SERVER_ID, res);
			
		}
			break;
		}
	}

	
}//namespace grt