#ifndef __RENDERING_SERVER_CLIENT_H__
#define __RENDERING_SERVER_CLIENT_H__
#include "signaller.h"
#include "json_parser.h"
#include <functional>
#include <map>
#include <future>

namespace grt {

	class rendering_server_client : public signaller_callback , 
		public parser_callback{

	private:
		using function_callback = std::function<void(message_type, absl::any msg)>;
		std::map< std::string, function_callback > register_functions_;
		std::promise<bool> connect_event_;
	public:

		void register_function(std::string id, function_callback callback);
		void unregister_function(std::string id);
		void set_connect_event(std::promise<bool>);
		/***signaller_callback interfaces*/
		void on_message(std::string msg) override;
		void on_connect() override;
		void on_error(std::string error) override;
		void on_close() override;

		//parser_callback interface implementation
		void on_message(message_type, absl::any msg) override;

	};

}//namespace grt


#endif//__RENDERING_SERVER_CLIENT_H__