#include "rendering_server_client.h"
#include <cassert>
#include <iostream>

namespace grt {
	

	void rendering_server_client::register_function(std::string id, function_callback callback) {
		//assert(register_functions_.find(id) == register_functions_.end());
		auto r = register_functions_.emplace(id, callback);
		assert(r.second);
	}

	void rendering_server_client::unregister_function(std::string id) {
		const auto count = register_functions_.erase(id);
		assert(count == 1);
	}

	void rendering_server_client::set_connect_event(std::promise<bool> receiver) {
		connect_event_ = std::move(receiver);
	}

	void rendering_server_client::on_message(std::string msg) {
		async_parse_message(msg, this);
	}

	void rendering_server_client::on_connect() {
		std::cout << "on connect called\n";
		connect_event_.set_value(true);
	}


	void rendering_server_client::on_error(std::string error) {
		assert(false);
	}

	void rendering_server_client::on_close() {

	}

	void rendering_server_client::on_message(message_type type, absl::any msg) {
		switch (type) {
		case message_type::window_create_res:
		{
			const auto res = absl::any_cast<wnd_create_res>(msg);
			auto iter = register_functions_.find(res.id);
			assert(iter != register_functions_.end());
			iter->second(type, msg);
		}
			break;
		case message_type::wnd_close_req_res:
		{
			const auto res = absl::any_cast<std::pair<bool, std::string>>(msg);
			auto iter = register_functions_.find(res.second);
			assert(iter != register_functions_.end());
			iter->second(type, msg);
		}
			break;
		default:
			assert(false);
			break;
		}
	}

}//namespace grt
