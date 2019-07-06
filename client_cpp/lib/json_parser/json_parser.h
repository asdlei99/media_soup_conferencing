#ifndef _JSON_PARSER_H__
#define _JSON_PARSER_H__
#include <string>
#include <absl/types/any.h>
#include "common/common_def.h" //todo: make proper path for this file

constexpr const char* FORWARD_MSG_TYPE_KEY{ "forward_message" };
namespace grt {

	enum class webrtc_message_type {
		OFFER,
		ANSWER,
		ICE_CANDIDATES
	};

	struct login_res {
		bool status_;
		std::string ip_;
		std::string port_;
	};

	struct login_req {
		std::string usr_;
		std::string pwd_;
		std::string server_;
	};

	class parser_callback {
	public:
		virtual void on_user_register_req(std::string name) {}
		virtual void on_user_register_response(bool isokay, std::string id){}
		virtual void on_prsence_notification(bool isconnected, std::string id, std::string name){}
		virtual void on_ice_servers_req_res(bool isokay, std::string servers_list){}
		virtual void on_error(std::string msg, std::string err){
		}
		
		virtual void on_webrtc_signalling_msg(webrtc_message_type type, 
			std::string id, absl::any msg, std::string detail){}

		virtual void on_signalling_url(std::string ip, std::string port){}
		virtual void on_message(message_type, absl::any msg){}

	};

	void async_parse_message(std::string msg, parser_callback* caller);

	std::string get_type(std::string const& msg);


	std::string create_room_create_req(std::string room_name);
	std::string make_room_close_req(std::string room_id);
	std::string make_room_join_req();
	std::string create_register_user_req(std::string name);
	std::string get_router_capability_msg();
	std::string make_register_user_res(std::string id, bool okay);
	std::string create_ice_servers_req(std::string id);
	
	std::string make_forwarded_message(std::string to, std::string self, std::string msg);

	std::string make_webrtc_signalling_msg(webrtc_message_type type, std::string detail, std::string msg);

	std::string create_call_req(call_type type);

	std::string create_call_req_res(call_type type, bool is_accept, std::string ip, std::string port, std::string id);
	std::string make_connection_status(bool is_okay);
	std::string make_prsence_notifcation(bool is_connected, std::string id, std::string name);
	std::string make_show_message(std::string message, std::string id);
	struct ice_candidates_info; //forward declaration
	std::string stringify(ice_candidates_info info);
	std::string make_login_message(std::string user_name, std::string pwd);
	std::string make_signalling_server_message();
}//namespace grt

#endif//_JSON_PARSER_H__