#ifndef _MEDIASOUP_CONFERENCE_H__
#define _MEDIASOUP_CONFERENCE_H__
#include <future>
#include <string>
#include "room.h"
namespace grt{


	///////  Room Management API /////////////////////////
	//returns room if on success, or throws exception if error
	std::future<std::string>
		async_create_room(std::string const room_name, std::string const server, 
			std::string port);

	//returns true on success
	std::future<bool>
		async_close_room(std::string const room_id, std::string const server,
			std::string port);

	///////  Room Management API /////////////////////////

	///////  Room USE API /////////////////////////
	std::future<std::shared_ptr<room>>
		async_join_room(std::string const user_name, std::string room_id,
			std::string const server, std::string port);



}//namespace grt

#endif//_MEDIASOUP_CONFERENCE_H__