#ifndef _MEDIA_SOUP_CONFER_CLB_H_
#define _MEDIA_SOUP_CONFER_CLB_H_

#include "signaller.h"


namespace grt {

	class parser_callback;

	class media_soup_signalling_cbk : public grt::signaller_callback{

	private:
		grt::signaller* sender_{ nullptr };
		std::shared_ptr<grt::parser_callback> parser_;
	public:

		media_soup_signalling_cbk(grt::signaller* p, std::shared_ptr<grt::parser_callback> parser);

		void on_message(std::string msg) override;

		void on_error(std::string error) override;

		void on_close() override;

		/*parser callback*/

		//void on_message(grt::message_type type, absl::any msg) override;
	};

}//namespace grt

#endif