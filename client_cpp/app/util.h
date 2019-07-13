#ifndef _UTIL_H__342_
#define _UTIL_H__342_
#include <functional>
#include <string>

namespace grt {
	class signaller;
}

namespace util {

	using id_response = std::function<void(std::string)>;

	void async_get_room_id(std::string const room_name, std::string const server,
		std::string const port, id_response res);

	using response = id_response;
	void async_close_room(std::string room_id, 
		std::string const server, std::string const port, response res);

	using status_type = std::function<void(std::shared_ptr<grt::signaller>)>;
	void async_join_room(std::string const room_id, std::string const user_name, std::string const server,
		std::string const port, status_type status);


}//namespace util

#endif