#ifndef __UI_COMMUNICATION_HANDLER_H_
#define __UI_COMMUNICATION_HANDLER_H_
#include <string>
#include "json_parser.h"
#include <memory>

namespace grt {
	class room;

	class ui_communication_handler : public parser_callback {
	private:
		std::shared_ptr<room> room_;

	public:
		~ui_communication_handler();
		void start(unsigned short port, int threads);
		void on_ui_message(std::string m);
		void message_for_ui(std::string m);
		//parser_callback
		void on_message(message_type, absl::any msg) override;
	};
}//namespace grt

#endif//__UI_COMMUNICATION_HANDLER_H_