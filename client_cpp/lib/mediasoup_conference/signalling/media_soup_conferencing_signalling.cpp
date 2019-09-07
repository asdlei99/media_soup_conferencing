#include "media_soup_conferencing_signalling.h"
#include <iostream>
#include "json_parser.h"

namespace grt {


		media_soup_signalling_cbk::~media_soup_signalling_cbk() {
#ifdef _DEBUG
			//assert(false);
			std::cout << "mediasoup signalling cbk called\n";
#endif//_DEBUG
		}

		media_soup_signalling_cbk::media_soup_signalling_cbk(
			std::shared_ptr<grt::parser_callback> parser)
			: parser_{ parser } {}

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

		std::unique_ptr<signaller_callback>
			get_signaller_clb( std::shared_ptr<parser_callback> parser) {
			return std::make_unique< media_soup_signalling_cbk>( parser);
		}

}//namespace grt