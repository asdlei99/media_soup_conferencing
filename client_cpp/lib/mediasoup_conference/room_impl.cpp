#include "room_impl.h"
#include <cassert>
#include "json_parser.h"
#include <iostream>

namespace grt {
	room_impl::room_impl(std::shared_ptr<signaller> signaler_, std::string const& id)
		:signaller_{ signaler_ }, 
		id_{ id }{
		assert(this->signaller_);
		
	}

	room_impl::~room_impl() 
	{
#ifdef _DEBUG
		std::cout << "  called\n";
#endif//_DEBUG
	}

	void room_impl::enter() {
			const auto m = grt::make_router_capablity_req();
			signaller_->send(m);
	}

	void room_impl::leave() {
		const auto m = grt::make_room_leave_req(id_);
		signaller_->send(m);
#ifdef _DEBUG
		assert(signaller_.unique());
#endif//_DEBUG
		signaller_->disconnect();
	}

	std::unique_ptr<room> get_room_handle_impl(std::shared_ptr<signaller>
		signaller_, const std::string& id) {
		return std::make_unique<room_impl>(signaller_, id);
	}
}//namespace grt

