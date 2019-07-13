#include "room_impl.h"
#include <cassert>
#include "json_parser.h"

namespace grt {
	room_impl::room_impl(std::shared_ptr<signaller> signaler_)
		:signaller_{ signaler_ }{
		assert(this->signaller_);
		
	}

	void room_impl::enter() {
			const auto m = grt::make_router_capablity_req();
			signaller_->send(m);
	}

	std::unique_ptr<room> get_room_handle_impl(std::shared_ptr<signaller>
		signaller_) {
		return std::make_unique<room_impl>(signaller_);
	}
}//namespace grt

