#ifndef __FUNC_THREAD_HANDLER_H__
#define __FUNC_THREAD_HANDLER_H__
#include "msg_loop_libraray.h"
#include <map>
#include <string>
#include <cassert>
#include <mutex>


constexpr const char* UI_SERVER_ID = "websocket_server"; //todo rename these properly
constexpr const char* UI_COMMUNICATROR_ID = "ui_id";
constexpr const char* REC_ID = "receiver_id";//this id is for getting UI messages from server.

constexpr const char* SENDER_ID = "sender";

namespace util {
	class func_thread_handler {
	private:
		std::map < std::string, thread_msg_lib<std::string>> func_thread_table_;
		std::mutex table_mutex_;
	public:
		template<typename Func>
		void register_id(std::string id, Func func) {
			std::lock_guard<std::mutex> l{ table_mutex_ };
			assert(func_thread_table_.find(id) == func_thread_table_.end());
			func_thread_table_[id].run(func);
		}

		void unregister(std::string id) {
			std::lock_guard<std::mutex> l{ table_mutex_ };
			assert(func_thread_table_.find(id) != func_thread_table_.end());
			func_thread_table_.erase(id);
		}

		void dispatch(std::string id, std::string m);
	};
}

#endif//