#include "ui_communication_handler.h"
#include <cassert>
#include "websocket_server/websocket_server.h"
#include "mediasoup_conference/mediasoup_conference.h"
#include "spdlog/spdlog.h"
#include "server_communication_util/util.h"

//#define UNIT_TEST //todo undef this when real media soup server running and actual connection is inteneded

namespace grt {


	ui_communication_handler::~ui_communication_handler() = default;

	void ui_communication_handler::start(unsigned short port, int threads) {
		auto *func_thread = util::get_func_thread();
		func_thread->register_id(
			REC_ID,
			std::bind(&ui_communication_handler::on_ui_message, this, std::placeholders::_1)
		);
		grt::start_server_block(port, 1, util::get_func_thread(), REC_ID);
	}

	void ui_communication_handler::message_for_ui(std::string m) {
		auto *func_thread = util::get_func_thread();
		func_thread->dispatch(UI_SERVER_ID, m);
	}

	void ui_communication_handler::on_ui_message(std::string m) {
		spdlog::info("on_ui_msg = {}", m);
		async_parse_message(m, this);
	}

	void ui_communication_handler::on_message(message_type type, absl::any msg) {
		switch (type) {
		case message_type::room_join_req:
		{
			spdlog::info("room_join_req from ui");

#ifndef UNIT_TEST
			assert(room_.get() == nullptr);
			const auto room_info = absl::any_cast<room_connection_credential>(msg);
			auto room_fut = grt::async_join_room(room_info.user_name_,
				room_info.room_id_, room_info.ip_, room_info.port_);
			auto room = room_fut.get();

			if (room) {
				room->enter();
				assert(room_.get() == nullptr);
				assert(room.use_count() == 1);
				room_ = room;
				assert(room.use_count() == 2);
			}

#endif//UNIT_TEST
			const auto res = make_room_join_req_res(room.get() != nullptr);
			auto *func_thread = util::get_func_thread();
			func_thread->dispatch(UI_SERVER_ID, res);

		}
			break;
		case message_type::room_leave_req:
		{
			spdlog::info("room leave req, room object count {} ", room_.use_count());
			assert(room_);
			room_->leave();
			assert(room_.unique());
			assert(room_.use_count() == 1);

			room_.reset();
			assert(room_.get() == nullptr);
			assert(room_.use_count() == 0);

			const auto m = make_session_leave_notification();
			message_for_ui(m);
		}
			break;
		case message_type::room_open_req:
		{
			const json m = absl::any_cast<json>(msg);
			spdlog::info("room open req from ui {}", m.dump());

			const std::string room_name = m["name"];
			const std::string ip = m["ip"];
			const std::string port = m["port"];
#ifndef UNIT_TEST
		    auto result =	grt::async_create_room(room_name, ip, port);
			absl::optional<std::string> id = result.get();
#else
			const std::string id = "test";//result.get();
#endif//
			const auto res = make_room_create_req_res(id.has_value(), id.value_or(""));
			auto *func_thread = util::get_func_thread();
			func_thread->dispatch(UI_SERVER_ID, res);
			
		}
			break;
		case message_type::req_room_info:
			//const std::vector<room_info> info = absl::any_cast<std::vector<room_info>>(msg);
			const json m = absl::any_cast<json>(msg);
			const std::string ip = m["ip"];
			const std::string port = m["port"];
			spdlog::info("req_room_info, msg ip = {}, port = {}", ip, port);
#ifndef UNIT_TEST
			auto result = grt::async_get_room_infos(ip, port);
			const auto rooms_info = result.get();
#else
			std::vector<room_info> const rooms_info{ {"test", "test"}, {"test1", "test1"} };
			
#endif//
			const auto ui_msg = convert_to_json(rooms_info.has_value(), *rooms_info);
			auto *func_thread = util::get_func_thread();
			func_thread->dispatch(UI_SERVER_ID, ui_msg);
			break;
		}
	}

	
}//namespace grt