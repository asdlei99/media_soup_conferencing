#include "mediasoup_conference.h"
#include "server_communication_util/util.h"
#include <iostream>
#include <cassert>
#include "conference_handler.h"
#include "signalling/media_soup_conferencing_signalling.h"
#include "room_impl.h"
#include "spdlog/spdlog.h"

namespace grt {
	std::future<absl::optional<std::string>>
		async_create_room(std::string const room_name, std::string const server,
			std::string port) {

		std::packaged_task<absl::optional<std::string>(absl::optional<std::string> const id)> task(
			[](absl::optional<std::string> id){

			spdlog::info("create room success = {}", id.has_value());
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
			spdlog::info("room close respnse result = {}", okay);
			return okay == "success";
		} };
		auto result = task.get_future();
		util::async_close_room(room_id, server, port, std::move(task));

		return result;
	}
	std::future< absl::optional<util::room_list>>
		async_get_room_infos(std::string const server, std::string const port) {
		std::packaged_task< absl::optional<util::room_list>(absl::optional<util::room_list>)> task{
			[](absl::optional<util::room_list> list) {
			spdlog::info("room info received from server is success = ", list.has_value());
#ifdef _DEBUG
			auto printer = [](const room_info info) {
					spdlog::info("room info : id = {} name = {}", info.id_, info.name_);
			};
			if(list.has_value())
				std::for_each(list->begin(), list->end(), printer);
			else {
				spdlog::error("room info not received \n");
			}
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
		> task{ [room_id](std::shared_ptr<signaller> signaller_)-> std::shared_ptr<room> {
				if (signaller_.get() == nullptr) {
					spdlog::error("room join not success no signaller recevid from room id {}", room_id);
					return nullptr;
					}
				spdlog::info("room join got siganaller for room = {}", room_id);
				auto media_soup_confernce = std::make_unique< media_soup_conference_handler>(signaller_.get());
				
				auto signaller_callback_ = get_signaller_clb(
					 std::move(media_soup_confernce));

				auto room = get_room_handle_impl(signaller_, room_id);

				signaller_->set_callback(std::move(signaller_callback_));

				return room;
				
		} };

		auto result = task.get_future();
		util::async_join_room(room_id, user_name, server, port,
			std::move(task));
		return result;
	}

}//namespace grt