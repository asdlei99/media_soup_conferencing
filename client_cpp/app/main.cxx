#include <iostream>
#include <mediasoup/include/Device.hpp>
#include <cassert>

int main() {
	std::cout << "hi mediasoup project\n";
	auto* device = new mediasoupclient::Device();
	assert(device);
	assert(false == device->IsLoaded());
	std::cout << "done\n";
}