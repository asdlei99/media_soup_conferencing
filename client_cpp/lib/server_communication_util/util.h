#ifndef _UTIL_H__342_
#define _UTIL_H__342_
#include <functional>
#include <string>
#include <future>
#include "common/common_def.h"
#include "absl/types//optional.h"

namespace grt {
	class signaller;
	class room;
}

namespace util {

	using id_response = std::packaged_task<absl::optional<std::string> (absl::optional<std::string>)>;

	void async_get_room_id(std::string const room_name, std::string const server,
		std::string const port, id_response&& res);

	using response = std::packaged_task<bool (std::string)>;;
	void async_close_room(std::string room_id, 
		std::string const server, std::string const port, response res);

	using room_list = std::vector<grt::room_info>;
	using room_info_res = std::packaged_task<absl::optional<room_list>(absl::optional<room_list>) >;
	void async_get_rooms_info(std::string const server, 
		std::string const port, room_info_res res);

	using signaller_handle = std::shared_ptr<grt::signaller>;
	using room_handle = std::shared_ptr<grt::room>;
	using status_type = std::packaged_task<room_handle(signaller_handle)>;
	void async_join_room(std::string const room_id, std::string const user_name, std::string const server,
		std::string const port, status_type status);

	class func_thread_handler;//forward declaration.
	func_thread_handler* get_func_thread();
}//namespace util

#endif