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
		room_join_res,
		router_capablity,
		producer_transport_res,
		produce_res,
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

}//namespace grt

#endif//__COMMON_DEF_H__