#include "media_soup_conferencing_signalling.h"
#include <iostream>
#include "json_parser.h"

namespace grt {

		media_soup_signalling_cbk::media_soup_signalling_cbk(grt::signaller* p,
			std::shared_ptr<grt::parser_callback> parser)
			:sender_{ p }, parser_{ parser } {
			assert(sender_);
		}

		void media_soup_signalling_cbk::on_message(std::string msg) {
			std::cout << "on message = " << msg << '\n';
			grt::async_parse_message(msg, parser_.get());
		}

		void media_soup_signalling_cbk::on_error(std::string error){
			std::cout << "error occur = " << error << '\n';
			assert(false);
		}

		void media_soup_signalling_cbk::on_close(){
			std::cout << "close sigannling connection\n";
		}

}//namespace grt