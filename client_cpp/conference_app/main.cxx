#include <iostream>
#include "ui_communication_handler.h"
#include "spdlog/spdlog.h"
#include "../include/spdlog/sinks/stdout_color_sinks.h" // or "../stdout_sinks.h" if no colors needed
#include "../include/spdlog/sinks/basic_file_sink.h"

constexpr unsigned short ui_server_port = 8086;

void intialize_logger() {
	try
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		//console_sink->set_level(spdlog::level::warn);
		console_sink->set_level(spdlog::level::trace);
		//console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("media_soup_conference.txt", true);
		file_sink->set_level(spdlog::level::trace);
		

		spdlog::logger logger("conference", { console_sink, file_sink });
		logger.set_level(spdlog::level::debug);
		//logger.flush_on(spdlog::level::info);
		//logger.warn("this should appear in both console and file");
		//logger.info("this message should not appear in the console, only in the file");

		// or you can even set multi_sink logger as default logger
		spdlog::set_default_logger(std::make_shared<spdlog::logger>("conference", spdlog::sinks_init_list({ console_sink, file_sink })));
		spdlog::flush_on(spdlog::level::info);
		//spdlog::flush_every(std::chrono::seconds(3));

	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log initialization failed: " << ex.what() << std::endl;
		assert(false);
		//REQUIRE(false);
	}
}

int main() {
	intialize_logger();
	spdlog::info("hi mediasoup project");
	grt::ui_communication_handler ui_handler;
	ui_handler.start(ui_server_port, 1);//this is blocking call

}