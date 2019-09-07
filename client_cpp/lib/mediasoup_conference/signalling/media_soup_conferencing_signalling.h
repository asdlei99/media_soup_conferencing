#ifndef _MEDIA_SOUP_CONFER_CLB_H_
#define _MEDIA_SOUP_CONFER_CLB_H_

#include "signaller.h"


namespace grt {

	class parser_callback;

	/*this class is just bridge between server message and parser callback
	this class may not be required. 
	will  do refactoring later.
	*/
	class media_soup_signalling_cbk : public grt::signaller_callback{

	private:
		std::shared_ptr<grt::parser_callback> parser_;
	public:
		~media_soup_signalling_cbk() override;
		media_soup_signalling_cbk( std::shared_ptr<grt::parser_callback> parser);

		void on_message(std::string msg) override;

		void on_error(std::string error) override;

		void on_close() override;

	};

	std::unique_ptr<signaller_callback>
		get_signaller_clb( std::shared_ptr<parser_callback> parser);
	
}//namespace grt

#endif