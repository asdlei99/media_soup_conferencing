#include "websocket_signaller.h"
namespace grt {

	std::unique_ptr<signaller>
		get_signaller_handle() {
		return std::make_unique< websocket_signaller>();
	}
}
