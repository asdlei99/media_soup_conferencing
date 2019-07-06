#include "util.h"
#include <future>
#include <thread>
#include "websocket_signaller.h"
#include <memory>
#include "json_parser.h"
#include <iostream>

namespace util {

	namespace internal {

		template<typename Command, typename Handler>
		class signalling_callback : public grt::signaller_callback,
			grt::parser_callback {

		private:
			grt::signaller* sender_{ nullptr };
			Command cmd;
			Handler handler;

		public:

			signalling_callback(grt::signaller* p, Command cmd, Handler handler)
				:sender_{ p }, cmd{ cmd }, handler{ handler }{
				assert(sender_);
			}

			void on_message(std::string msg) override {
				std::cout << "on message = " << msg << '\n';
				grt::async_parse_message(msg, this);
			}

			void on_connect() override {
				std::cout << "on connect called\n";
				cmd();
			}

			void on_error(std::string error) override {
				std::cout << "error occur = " << error << '\n';
				assert(false);
			}

			void on_close() override {
				std::cout << "close sigannling connection\n";
			}

			/*parser callback*/
			void
				on_user_register_response(bool isokay, std::string id) override {
				std::cout << "user register response received\n";
				std::cout << "status = " << isokay << '\n';
				std::cout << "id = " << id << '\n';
				//	id_ = id;
			}

			void on_message(grt::message_type type, absl::any msg) override {
				handler(type, msg, this);
			}
		};

		template<typename Command, typename Handler>
		std::unique_ptr< signalling_callback< Command, Handler>>
			make_unique(grt::signaller* p, Command cmd, Handler handler) {
			return std::make_unique< signalling_callback< Command, Handler>>(p, cmd, handler);
		}

		std::string
			room_id_geter(std::string const room_name, std::string const server,
				std::string const port) {
			std::promise<std::string> promise;
			auto future = promise.get_future();

			grt::websocket_signaller signaller;
			auto signalling_callback = make_unique(&signaller, [&signaller, room_name]() {
				const auto m = grt::create_room_create_req("anil_room");
				signaller.send(m);
			}, [&promise](grt::message_type type, absl::any msg, auto ptr) {
				if (grt::message_type::create_room_res == type) {

					std::string id = absl::any_cast<std::string>(msg);
					std::cout << "received create room id " << id << '\n';
					//delete ptr;//improve this design
					promise.set_value(id);
					std::cout << "after promise set\n";
				}
			});

			signaller.connect(server, port, std::move(signalling_callback));

			const auto r = future.get();
			std::cout << "in future " << r << '\n';
			signaller.disconnect();
			std::cout << "returning value\n";
			return r;

		}

		std::string close_room_req(std::string const room_id, std::string const server,
			std::string const port) {
			std::promise<std::string> promise;
			auto future = promise.get_future();

			grt::websocket_signaller signaller;
			auto signalling_callback = make_unique(&signaller, [&signaller, room_id]() {
				const auto m = grt::make_room_close_req(room_id);
				signaller.send(m);
			}, [&promise](grt::message_type type, absl::any msg, auto ptr) {
				if (grt::message_type::close_room_res == type) {

					std::string const result = absl::any_cast<std::string>(msg);
					std::cout << "close room res" << result << '\n';
					//delete ptr;//improve this design
					promise.set_value(result);
					std::cout << "after promise set\n";
				}
			});

			signaller.connect(server, port, std::move(signalling_callback));

			const auto r = future.get();
			std::cout << "in future " << r << '\n';
			signaller.disconnect();
			std::cout << "returning value\n";
			return r;
		}



	}//namespace internal

	template<typename TaskType, typename Response, typename Task, typename... Args>
	void async_task_executor(Response res, Task task, Args... args) {
		std::packaged_task<TaskType> task_(task);
		auto f1 = task_.get_future();
		std::thread{ std::move(task_), args... }.detach();
		std::thread{ [f1 = std::move(f1), res]()mutable{
						res(f1.get());
				} }.detach();
	}

	void async_get_room_id(std::string const room_name, std::string const server,
		std::string const port, id_response res) {
		async_task_executor<
			decltype(internal::room_id_geter)
		>( res, internal::room_id_geter, room_name, server, port);

	}

	void async_close_room(std::string room_id,
		std::string const server, std::string const port, response res) {

		async_task_executor<
			decltype(internal::close_room_req)
		>(res, internal::close_room_req, room_id, server, port);
		
	}
}//namespace util