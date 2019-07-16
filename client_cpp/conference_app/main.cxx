#include <iostream>
#include "ui_communication_handler.h"

constexpr unsigned short ui_server_port = 8086;

int main() {
	std::cout << "hi mediasoup project\n";
	
	grt::ui_communication_handler ui_handler;
	ui_handler.start(ui_server_port, 1);//this is blocking call

}