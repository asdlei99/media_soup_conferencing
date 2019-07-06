#include "json_parser.h"
#include <json.hpp>
//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/json_parser.hpp>
#include <cassert>
#include <map>
#include <thread>
#include <iostream>

constexpr const char* TYPE{ "type" };
constexpr const char* REG_USR_REQ{ "register_user" };
constexpr const char* REG_USR_REQ_RES{ "register_user_response" };
constexpr const char* ID{ "id" };
constexpr const char* STATUS{ "status" };
constexpr const char* OKAY{ "ok" };
constexpr const char* OKAY_{ "okay" };
constexpr const char* SUCESS{ "success" };
constexpr const char* ERR{ "error" };
constexpr const char* NAME{ "name" };
constexpr const char* CON_STATUS{ "connection_status" };
constexpr const char* CONNECTED{ "connected" };
constexpr const char* PRESENCE_NOTIFICATION{ "presence_notification" };
constexpr const char* KEY{ "key" };

constexpr const char* REQ_ICE_CANDIDATES{ "req_ice_candidates" };
constexpr const char* ICE_CANDIDATES_REQ_RES{ "res_ice_candidates" };
constexpr const char* SIGNALLING_SERVER_URL{ "signalling_server_url" };
constexpr const char* DETAIL{ "detail" };
constexpr const char* URL{ "url" };

constexpr const char* FROM{ "from" };
constexpr const char* TO{ "to" };
constexpr const char* PEER_MSG_KEY{ "m" };
constexpr const char* PEER_OFFER{ "offer" };
constexpr const char* PEER_ANS{ "answer" };
constexpr const char* PEER_ICE{ "iceCandidates" };
constexpr const char* PEER_CALL_REQ{ "call_request" };
constexpr const char* PEER_CALL_RES{ "call_request_res" };
constexpr const char* AUDIO_CALL{ "audio" };
constexpr const char* VIDEO_CALL{ "video" };
constexpr const char* SDP{ "sdp" };
constexpr const char* CANDIDATE_SDP{ "candidate" };
constexpr const char* MID{ "sdpMid" };
constexpr const char* MINDEX{ "sdpMLineIndex" };
constexpr const char* IP{ "ip" };
constexpr const char* PORT{ "port" };
constexpr const char* CREATE_CONNECTION{ "create_peer" };
constexpr const char* REMOVE_CONNECTION{ "remove_peer" };
constexpr const char* SHOW_MSG{ "show_message" };
constexpr const char* LOGIN_RES{ "login_response" };
constexpr const char* LOGIN_REQ{ "login_req" };
constexpr const char* USR{ "usr" };
constexpr const char* PWD{ "pwd" };

constexpr const char* CALL_RES_EVNT{ "call_response_event" };
constexpr const char* SIGNALLING_SERV_REQ{ "request_signalling_server" };
constexpr const char* SIGNALLING_SERV_REQ_RES{ "response_siganalling_server" };

using json = nlohmann::json;
namespace grt {
	
	namespace detail {

		//std::string get_key_value(std::string json, std::string key) {
		//	ptree pt;
		//	boost::property_tree::json_parser::read_json(std::stringstream{ json }, pt);
		//	const auto value = pt.get_child(key).get_value<std::string>();
		//	return value;
		//}

		bool
			is_status_okay(std::string const status) {
			return (status == OKAY || status == OKAY_ || status == SUCESS);
		}

		//bool is_call_accepted(std::string status) {
		//	return (status == "yes");
		//}

		//std::string to_string(bool is_accept) {
		//	return is_accept?"yes" : "no";
		//}

		//bool is_connected_msg(std::string status) {
		//	return (status == CONNECTED);
		//}

		//std::string to_connection_status_str(bool status) {
		//	return status ? CONNECTED : ERR;
		//}

		//std::string
		//	get_type(std::string json) {
		//	const auto type = get_key_value(json, TYPE);
		//	return type;
		//}

		//std::string get_register_user_name(std::string json) {
		//	const auto name = get_key_value(json, NAME);
		//	return name;
		//}

		//call_type to_call_type(std::string const detail) {
		//	if (detail == VIDEO_CALL) return call_type::VIDEO;
		//	if (detail == AUDIO_CALL) return call_type::AUDIO;
		//	assert(false);
		//};

		//std::string to_string(call_type type) {
		//	switch (type) {
		//	case call_type::AUDIO: return AUDIO_CALL;
		//	case call_type::VIDEO: return VIDEO_CALL;
		//		assert(false);
		//	}
		//}

		//void _parse_forwarded(std::string id, std::string forwarded_msg, parser_callback* caller) {
		//	assert(caller);
		//	const auto type = detail::get_type(forwarded_msg);
		//	if (type == PEER_OFFER || type == PEER_ANS || type == PEER_ICE) {
		//		auto to_webrtc_signalling_msg = [](const std::string type) {
		//			if (type == PEER_OFFER) return webrtc_message_type::OFFER;
		//			if (type == PEER_ANS) return webrtc_message_type::ANSWER;
		//			if (type == PEER_ICE) return webrtc_message_type::ICE_CANDIDATES;
		//			assert(false);
		//		};
		//		const auto msg_type = to_webrtc_signalling_msg(type);
		//		const auto offer = get_key_value(forwarded_msg, PEER_MSG_KEY);
		//		const auto detail = get_key_value(forwarded_msg, DETAIL);
		//		if (type == PEER_ICE) {
		//			const int mline_index = std::stoi(get_key_value(offer, MINDEX));
		//			const auto mid = get_key_value(offer, MID);
		//			const auto sdp = get_key_value(offer, CANDIDATE_SDP);
		//			const ice_candidates_info info{ mid, mline_index, sdp };
		//			caller->on_webrtc_signalling_msg(msg_type, id, info, detail);
		//		}
		//		else {
		//			const auto sdp = get_key_value(offer, SDP);
		//			caller->on_webrtc_signalling_msg(msg_type, id, sdp, detail);
		//		}
		//			
		//	}
		//	else if (type == PEER_CALL_REQ) {
		//		const auto detail = get_key_value(forwarded_msg, DETAIL);
		//		caller->on_message(message_type::call_req,
		//			call_req_info{ to_call_type(detail) , id, forwarded_msg });
		//		//caller->on_call_req(to_call_type(detail), id);
		//	}
		//	else if (type == PEER_CALL_RES) {
		//		const auto detail = get_key_value(forwarded_msg, DETAIL);
		//		const auto status = get_key_value(forwarded_msg, STATUS);
		//		const auto is_accepted = is_call_accepted(status);
		//		const auto remote_id = get_key_value(forwarded_msg, ID);
		//		const auto url = get_key_value(forwarded_msg, URL);
		//		caller->on_message(
		//			message_type::call_req_res,
		//			call_response_info{ detail, 
		//			remote_id, is_accepted, url, status, id, forwarded_msg }
		//		);
		//	}
		//}

		void _parse(std::string msg, parser_callback* caller) {
			assert(caller);
			const auto json_msg = json::parse(msg);
			const auto type = json_msg[TYPE];
			if (type == REG_USR_REQ) {
				assert(false);
				/*const auto name = detail::get_register_user_name(msg);
				caller->on_user_register_req(name);*/
			}
			else if (type == REG_USR_REQ_RES) {
				const auto status = json_msg[STATUS]; //get_key_value(msg, STATUS);
				if (status == OKAY) {
					const auto id = json_msg[ID];// get_key_value(msg, ID);
					caller->on_user_register_response(true, id);
				}
				else {
					assert(status == ERR);
					caller->on_user_register_response(false, std::string{});
				}
			}
			else if (type == ICE_CANDIDATES_REQ_RES) {
				assert(false);
				/*const auto status = get_key_value(msg, STATUS);
				if (is_status_okay(status)) {
					const auto detail = get_key_value(msg, DETAIL);
					caller->on_ice_servers_req_res(true, detail);
				}
				else {
					caller->on_ice_servers_req_res(false, std::string{});
				}*/
			}
			else if (type == PRESENCE_NOTIFICATION) {
				assert(false);
				/*const auto notify_msg = get_key_value(msg, PEER_MSG_KEY);
				const auto prsence_type = get_key_value(notify_msg, TYPE);
				const auto id = get_key_value(notify_msg, KEY);
				const auto name = get_key_value(notify_msg, NAME);
				caller->on_prsence_notification(is_connected_msg(prsence_type), id, name);*/
			}
			else if (type == FORWARD_MSG_TYPE_KEY) {
				assert(false);
			/*	const auto from = get_key_value(msg, FROM);
				const auto forwarded_msg = get_key_value(msg, PEER_MSG_KEY);
				_parse_forwarded(from, forwarded_msg, caller);*/
			}
			else if (type == SIGNALLING_SERVER_URL) {
				assert(false);
			/*	const auto ip = get_key_value(msg, IP);
				const auto port = get_key_value(msg, PORT);
				caller->on_signalling_url(ip, port);*/
			}
			else if (type == CREATE_CONNECTION) {
				assert(false);
				/*const auto id = get_key_value(msg, ID);
				const auto url = get_key_value(msg, URL);
				caller->on_message(grt::message_type::create_peer, callee_address{ url, id });*/
			}
			else if (type == REMOVE_CONNECTION) {
				assert(false);
				/*const auto id = get_key_value(msg, ID);
				caller->on_message(grt::message_type::remove_peer, id);*/
			}
			else if (type == LOGIN_RES) {
				std::cout << "login response receviced\n";
				/*const auto status = get_key_value(msg, STATUS);
				assert(is_status_okay(status));
				const auto ip = get_key_value(msg, IP);
				const auto port = get_key_value(msg, PORT);
				caller->on_message(message_type::login_res,
					login_res{ is_status_okay(status), ip, port });*/
				assert(false);
			}
			else if (type == LOGIN_REQ) {
				assert(false);
				/*const auto ip = get_key_value(msg, IP);
				const auto usr = get_key_value(msg, USR);
				const auto pwd = get_key_value(msg, PWD);
				caller->on_message(message_type::login_req,
					login_req{ usr, pwd, ip });*/
			}
			else if (type == CALL_RES_EVNT) {
				assert(false);
				//const auto 
		/*	const auto status =	get_key_value(msg, STATUS);
			const bool is_accepted = is_call_accepted(status);
			const auto remote_id = get_key_value(msg, FROM);
			const auto self_id = get_key_value(msg, ID);
			const auto detail = get_key_value(msg, DETAIL);
			caller->on_message(message_type::call_res_evt, 
				call_res_event{to_call_type(detail),remote_id, self_id, is_accepted });*/

			}
			else if (type == SIGNALLING_SERV_REQ_RES) {
				assert(false);
			/*	const auto status = get_key_value(msg, STATUS);
				const auto is_ok = is_status_okay(status);
				const auto ip = is_ok ? get_key_value(msg, IP) : std::string{};
				const auto port = is_ok ? get_key_value(msg, PORT) : std::string{};
				caller->on_message(message_type::signalling_serv_req_res,
					signaling_server_req_res{is_ok, ip, port});*/
			}
			else if (type == "room_open_response") {
				const auto status = json_msg[STATUS];
				assert(detail::is_status_okay(status));
				const std::string id = json_msg[ID];
				caller->on_message(message_type::create_room_res, id);
			}
			else if (type == "room_close_response") {
				const std::string status = json_msg[STATUS];
				//assert(detail::is_status_okay(status));
				caller->on_message(message_type::close_room_res, status);
			}
			else if (type == "room_join_response") {
				const std::string status = json_msg[STATUS];
				caller->on_message(message_type::room_join_res, status);
			}
			else if (type == "response_router_capablity") {
				const auto m = json_msg[PEER_MSG_KEY];
				caller->on_message(message_type::router_capablity, m);
			}
			else {
				std::cout << "not supported msg = " << msg << "\n";
				caller->on_error(msg, "not supported msg");
				assert(false);
			}
		}
		
	}//namespace detail

	//void add_ptree(ptree& pt) {
	//	return;
	//}

	//template<typename T, typename... Args>
	//void add_ptree(ptree& pt, T&& t, Args&&... args) {
	//	pt.put(t.first, t.second);
	//	add_ptree(pt, std::forward<Args>(args)...);
	//}

	//template<typename... Args>
	//std::string 
	//	make_json_string(Args&&... args) {
	//	ptree pt;
	//	add_ptree(pt, std::forward<Args>(args)...);
	//	std::stringstream ss;
	//	boost::property_tree::json_parser::write_json(ss, pt);

	//	return ss.str();
	//}

	std::string 
		create_room_create_req(std::string room_name) {
		const json j2 = {
			{TYPE, "request_room_open"},
			{NAME, room_name}
		};
		return j2.dump();
		
	}

	std::string 
		make_room_close_req(std::string room_id) {
		const json j2 = {
			{TYPE, "request_room_close"},
		{ID, room_id}
		};
		return j2.dump();
	}

	std::string 
		make_room_join_req() {
		const json j2 = {
			{TYPE, "request_room_join"}
		};
		return j2.dump();
	}

	std::string 
		create_register_user_req(std::string name) {
		const json j2 = {
			{TYPE, REG_USR_REQ},
			{NAME, name}
		};
		return j2.dump();
	}

	std::string make_router_capablity_req() {
	
		const json j2 = {
			{TYPE, "get_router_capability"}
		};
		return j2.dump();
	}
	//std::string 
	//	make_register_user_res(std::string id, bool okay) {
	//	return
	//		make_json_string(
	//			std::make_pair(TYPE, REG_USR_REQ_RES),
	//			std::make_pair(STATUS, (okay ? OKAY : ERR)),
	//			std::make_pair(ID, id)
	//		);
	//}

	//std::string 
	//	create_ice_servers_req(std::string id) {
	//	return make_json_string(
	//		std::make_pair(TYPE, REQ_ICE_CANDIDATES),
	//		std::make_pair(ID, id)
	//	);
	//}

	//std::string 
	//	make_forwarded_message(std::string to, std::string self, std::string msg) {
	//	return make_json_string(
	//		std::make_pair(TYPE, FORWARD_MSG_TYPE_KEY),
	//		std::make_pair(TO, to),
	//		std::make_pair(FROM, self),
	//		std::make_pair(PEER_MSG_KEY, msg)
	//	);
	//}

	//std::string 
	//	make_webrtc_signalling_msg(webrtc_message_type type, std::string detail, std::string msg) {
	//	using message_type = webrtc_message_type;
	//	auto convert_string = [](message_type type) {
	//		switch (type) {
	//		case message_type::ANSWER:return PEER_ANS;
	//		case message_type::ICE_CANDIDATES: return PEER_ICE;
	//		case message_type::OFFER: return PEER_OFFER;
	//		assert(false);
	//		}
	//	};

	//	const auto type_str = convert_string(type);
	//	
	//	if (type == message_type::ANSWER || type == message_type::OFFER) {
	//		const auto forwarded_msg = make_json_string(
	//			std::make_pair(TYPE, type_str),
	//			std::make_pair(SDP, msg)
	//		);
	//		return make_json_string(
	//			std::make_pair(TYPE, type_str),
	//			std::make_pair(DETAIL, detail),
	//			std::make_pair(PEER_MSG_KEY, forwarded_msg)
	//		);
	//	}
	//
	//	return make_json_string(
	//		std::make_pair(TYPE, type_str),
	//		std::make_pair(DETAIL, detail),
	//		std::make_pair(PEER_MSG_KEY, msg)
	//	);
	//}

	//std::string create_call_req(call_type type) {
	//	return make_json_string(
	//		std::make_pair(TYPE, PEER_CALL_REQ),
	//		std::make_pair(DETAIL, detail::to_string(type))
	//	);
	//}

	////todo: remove this function dependency asap
	//std::string _to_url(std::string ip, std::string port) {
	//	return std::string{ "ws://" } +ip + ':' + port;
	//}

	//std::string 
	//	create_call_req_res(call_type type, bool is_accept, std::string ip, std::string port, std::string id) {
	//	return make_json_string(
	//		std::make_pair(TYPE, PEER_CALL_RES),
	//		std::make_pair(DETAIL, detail::to_string(type)),
	//		std::make_pair(STATUS, detail::to_string(is_accept)),
	//		std::make_pair(URL, _to_url(ip, port)),
	//		std::make_pair(ID, id)
	//	);
	//}

	//std::string 
	//	make_connection_status(bool is_okay) {
	//	return make_json_string(
	//		std::make_pair(TYPE, CON_STATUS),
	//		std::make_pair(STATUS, detail::to_connection_status_str(is_okay))
	//	);
	//}

	//std::string 
	//	make_prsence_notifcation(bool is_connected, std::string id, std::string name) {
	//	const auto m = make_json_string(
	//		std::make_pair(TYPE, detail::to_connection_status_str(is_connected)),
	//		std::make_pair(KEY, id),
	//		std::make_pair(NAME, name)

	//	);
	//	return make_json_string(
	//		std::make_pair(TYPE, PRESENCE_NOTIFICATION),
	//		std::make_pair(PEER_MSG_KEY, m)
	//	);
	//}

	//std::string 
	//make_show_message(std::string message, std::string id) {
	//	return make_json_string(
	//		std::make_pair(TYPE, SHOW_MSG),
	//		std::make_pair(PEER_MSG_KEY, message),
	//		std::make_pair(ID, id)
	//	);
	//}

	//std::string 
	//	stringify(ice_candidates_info info) {
	//	return make_json_string(
	//		std::make_pair(MID, info.mid_),
	//		std::make_pair(MINDEX, std::to_string(info.mline_index_)),
	//		std::make_pair(CANDIDATE_SDP, info.sdp_)
	//	);
	//}

	//std::string
	//	make_login_message(std::string user_name, std::string pwd) {
	//	return std::string{ "/login_req?" } +"user=" + user_name + "&pwd=" + pwd;
	//}

	//std::string make_signalling_server_message() {
	//	return make_json_string(
	//			std::make_pair(TYPE, SIGNALLING_SERV_REQ)
	//	);
	//}

	void async_parse_message(std::string msg, parser_callback* caller) {

		std::thread{
			detail::_parse, msg, caller
		}.detach();
		
	}

	//std::string get_type(std::string const& msg) {
	//	return detail::get_type(msg);
	//}

	//
}//namespace grt