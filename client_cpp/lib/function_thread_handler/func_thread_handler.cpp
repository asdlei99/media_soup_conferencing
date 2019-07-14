#include "func_thread_handler.h"

namespace util {

	void func_thread_handler::dispatch(std::string id, std::string m) {
		std::lock_guard<std::mutex> l{ table_mutex_ };
		assert(func_thread_table_.find(id) != func_thread_table_.end());
		auto iter = func_thread_table_.find(id);
		if (iter != func_thread_table_.end()) {
			iter->second.push_message(m);
		}

	}
}//namespace util