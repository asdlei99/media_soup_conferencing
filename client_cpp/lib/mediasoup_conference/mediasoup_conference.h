#ifndef _MEDIASOUP_CONFERENCE_H__
#define _MEDIASOUP_CONFERENCE_H__
#include <future>
#include <string>
#include "room.h"
#include "common/common_def.h"
#include "absl/types/optional.h"
#include "server_communication_util/util.h"

namespace grt{


	///////  Room Management API /////////////////////////
	//returns room if on success, or throws exception if error
	std::future<absl::optional<std::string>>
		async_create_room(std::string const room_name, std::string const server, 
			std::string port);

	//returns true on success
	std::future<bool>
		async_close_room(std::string const room_id, std::string const server,
			std::string port);

	std::future< absl::optional<util::room_list>>
		async_get_room_infos(std::string const server, std::string const port);
	///////  Room Management API /////////////////////////

	///////  Room USE API /////////////////////////
	std::future<std::shared_ptr<room>>
		async_join_room(std::string const user_name, std::string room_id,
			std::string const server, std::string port);



}//namespace grt

#endif//_MEDIASOUP_CONFERENCE_H__