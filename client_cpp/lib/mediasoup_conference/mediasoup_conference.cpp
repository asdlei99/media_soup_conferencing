#include "mediasoup_conference.h"
#include "util.h"
#include <iostream>
#include <cassert>
#include "conference_handler.h"
#include "signalling/media_soup_conferencing_signalling.h"
#include "room_impl.h"

namespace grt {
	std::future<std::string>
		async_create_room(std::string const room_name, std::string const server,
			std::string port) {

		std::packaged_task<std::string (std::string const id)> task(
			[](const std::string id){
#ifdef _DEBUG
			std::cout << "id received " << id << '\n';
#endif
			return id;
		});
		auto result  = task.get_future();

		util::async_get_room_id(room_name, server, port, std::move(task));
		
		return result;
	}

	std::future<bool>
		async_close_room(std::string const room_id, std::string const server,
			std::string port) {

		std::packaged_task<bool(std::string const)> task{ [](std::string const okay){
#ifdef _DEBUG
			std::cout << "room close response " << okay << '\n';
#endif//
			return okay == "success";
		} };
		auto result = task.get_future();
		util::async_close_room(room_id, server, port, std::move(task));

		return result;
	}
	std::future< std::vector<room_info>>
		async_get_room_infos(std::string const server, std::string const port) {
		std::packaged_task<util::room_list(util::room_list)> task{
			[](util::room_list list) {
#ifdef _DEBUG
			auto printer = [](const room_info info) {
					std::cout << info.id_ << " name = " << info.name_ << '\n';
			};
			std::for_each(list.begin(), list.end(), printer);
#endif//_DEBUG

			return list;
			} };
			auto result = task.get_future();
			util::async_get_rooms_info(server, port, std::move(task));
			return result;

		}

	std::future<std::shared_ptr<room>>
		async_join_room(std::string const user_name, std::string room_id,
			std::string const server, std::string port) {

		std::packaged_task<
			std::shared_ptr<room>(std::shared_ptr<signaller>)
		> task{ [](std::shared_ptr<signaller> signaller_)-> std::shared_ptr<room> {
				if (signaller_.get() == nullptr) return nullptr;
#ifdef _DEBUG
				std::cout << "got server handle to server room" << '\n';
#endif//
				auto media_soup_confernce = std::make_unique< media_soup_conference_handler>(signaller_.get());
				
				auto signaller_callback_ = get_signaller_clb(
					 std::move(media_soup_confernce));

				auto room = get_room_handle_impl(signaller_);

				signaller_->set_callback(std::move(signaller_callback_));

				return room;
				
		} };

		auto result = task.get_future();
		util::async_join_room(room_id, user_name, server, port,
			std::move(task));
		return result;
	}

}//namespace grt