#ifndef _ROOM_IMPL_H__
#define _ROOM_IMPL_H__
#include "room.h"
#include "signalling/media_soup_conferencing_signalling.h"

namespace grt {

	class room_impl : public room {
	private:
		std::shared_ptr<signaller> signaller_;
		const std::string id_;
	public:
		room_impl(std::shared_ptr<signaller> signaler_, std::string const& id);
		~room_impl() override;
		void enter() override;
		void leave() override;
	};

	std::unique_ptr<room> get_room_handle_impl(std::shared_ptr<signaller>, std::string const&);

}//namespace grt


#endif//_ROOM_IMPL_H__