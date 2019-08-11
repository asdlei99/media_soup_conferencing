#ifndef _JSON_PARSER_H__
#define _JSON_PARSER_H__
#include <string>
#include <absl/types/any.h>
#include "common/common_def.h" //todo: make proper path for this file
#include <json.hpp>

using json = nlohmann::json;
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
	std::string make_room_create_req_res(bool const status, std::string room_id);
	std::string make_room_close_req(std::string room_id);
	std::string make_room_infos_req();
	std::string make_room_join_req();
	std::string convert_to_json(std::vector<room_info> const&);
	std::string make_room_join_req_res(const bool status);
	std::string create_register_user_req(std::string name);
	std::string make_router_capablity_req();
	std::string make_producer_transport_creation_req(bool force_tcp, json const& rtpCapablity);
	std::string make_producer_transport_connect_req(std::string const transport_id, json const& dtls_parameter);
	std::string make_produce_transport_req(std::string const transport_id, std::string kind,
		json const& rtp_parameters);

	std::string make_consumer_transport_creation_req(bool force_tcp);
	std::string make_consume_req(std::string peer_id, json const& rtc_capablity);
	std::string make_consumer_resume_req(std::string id);
	std::string make_consumer_trasport_connect_req(std::string transport_id,
		json const& dtls_parameters);

	std::string make_render_wnd_req(std::string const id);
	std::string make_render_wnd_req_res(bool is_ok, std::string class_name, std::string parent_wnd_name, std::string id, std::string sender_id);

	std::string make_render_wnd_close_req(std::string const id);
	std::string make_render_wnd_close_res(bool is_ok, std::string const id);

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