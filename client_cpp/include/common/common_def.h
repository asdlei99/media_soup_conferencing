#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__
#include <string>

namespace grt {
	enum class message_type {
		offer_sdp,
		answer_sdp,
		ice_candidates,
		remote_stream_attached_evt,
		connection_disconnected,
		connection_connected,
		user_registration_req,
		forward_message,
		create_peer,
		remove_peer,
		login_res,
		login_req,
		call_req,
		call_req_res,
		call_res_evt,
		signalling_serv_req_res,
		create_room_res,
		close_room_res,
		room_join_req,
		room_open_req,
		room_join_res,
		req_room_info,
		res_rooms_info,
		router_capablity,
		producer_transport_res,
		produce_res,
		consumer_create_res,
		peer_add,
		peer_remove,
		consumer_res,
		consumer_connect_res,
		wnd_create_req,
		window_create_res,
		wnd_close_req,
		wnd_close_req_res,
		invalid
	};

	struct ice_candidates_info {
		std::string mid_;
		int mline_index_;
		std::string sdp_;
	};

	struct sdp_message {
		std::string sdp;

	};

	enum class call_type {
		AUDIO,
		VIDEO
	};


	struct call_response_info {
		std::string detail_;
		std::string new_id_;
		bool is_accepted_;
		std::string url_;
		std::string status_;
		std::string from_;
		std::string json_;
	};

	struct callee_address {
		std::string url_;
		std::string id_;
	};

	struct call_req_info {
		call_type type_;
		std::string sender_id_;
		std::string json_;
	};
	
	struct call_res_event {
		call_type type_;
		std::string sender_id_;
		std::string self_id_;
		bool is_accepted_;
	};

	struct signaling_server_req_res {
		bool is_okay_;
		std::string ip_;
		std::string port_;
	};

	struct room_connection_credential {
		std::string ip_;
		std::string port_;
		std::string room_id_;
		std::string user_name_;
	};

	struct room_info {
		std::string id_;
		std::string name_;
	};

	struct wnd_create_res {
		bool status_{ false };
		std::string class_name_;
		std::string parent_wnd_name_;
		std::string child_wnd_id_;
		std::string id;
	};

}//namespace grt

#endif//__COMMON_DEF_H__