#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../lib/display_manager/win32_window.h"
#include "../lib/server_communication_util/util.h"
#include "../lib/windows_util/windows_util.h"
#include "websocket_signaller.h"
#include "server_communication_util/rendering_server_client.h"

TEST_CASE("window resolution", "[wnd_resolution]") {
	const auto w_h = display::get_desktop_width_height();
	REQUIRE(w_h.first > 0);
	REQUIRE(w_h.second > 0);
	std::cout << "Please enter width of desktop\n";
	int width = 0; 
	std::cin >> width;
	std::cout << "\nPlease enter height of desktop\n";
	int height = 0;
	std::cin >> height;
	REQUIRE(w_h.first == width);
	REQUIRE(w_h.second == height);
	{
		const auto h = display::get_desktop_height();
		REQUIRE(h == height);
	}

	{
		const auto w = display::get_desktop_width();
		REQUIRE(w == width);
	}
}


void test_window(grt::message_type type, absl::any msg, std::string command_id, bool passTest) {
	assert(type == grt::message_type::window_create_res);
	auto wndInfo = absl::any_cast<grt::wnd_create_res>(msg);
	const auto className = wndInfo.class_name_;
	const auto wndName = wndInfo.parent_wnd_name_;
	const auto id = wndInfo.child_wnd_id_;
	REQUIRE(wndInfo.id == command_id);
	if (passTest) {
	
		REQUIRE(wndInfo.status_);
		REQUIRE_FALSE(className.empty());
		REQUIRE_FALSE(wndName.empty());
		REQUIRE_FALSE(id.empty());
		auto wnd = util::find_child_window(className, wndName, id);
		REQUIRE(wnd);
	}
	else {
		//fail test
		REQUIRE_FALSE(wndInfo.status_);
		REQUIRE(className.empty());
		REQUIRE(wndName.empty());
		REQUIRE(id.empty());
	}
	
	//res();
}

void create_window_test(grt::sender* sender, std::string id
	, bool const passTest = true) {
	const auto m = grt::make_render_wnd_req(id);
	std::promise<void> done;
	auto done_future = done.get_future();
	sender->send_to_renderer(id, m, [&done, id, passTest](auto type, auto msg){
		test_window(type, msg, id, passTest);
		done.set_value();
	});

	done_future.get();//wait for event.
	sender->done(id);
}

void close_window_test(grt::sender* sender, std::string id
	, bool const passTest = true) {
	const auto m = grt::make_render_wnd_close_req(id);
	std::promise<void> done;
	auto wait = done.get_future();
	sender->send_to_renderer(id, m, [&done, id, passTest](auto type, auto msg) {
		assert(type == grt::message_type::wnd_close_req_res);
		auto result = absl::any_cast<std::pair<bool, std::string>>(msg);

		REQUIRE(result.second == id);
		if (passTest) REQUIRE(result.first);
		else REQUIRE_FALSE(result.first);
		done.set_value();
	});

	wait.get();//wait for event.
	sender->done(id);
}

TEST_CASE("server connection test", "[local_rendering_server]") {

	grt::sender sender;
	auto future = sender.sync_connect("localhost", "8002");
	auto is_connected = future.get();
	REQUIRE(is_connected);
	create_window_test(&sender, "Anil");
}

////
TEST_CASE("rendering wind multiple create", "[local_rendering_server]") {
	grt::sender sender;
	auto future = sender.sync_connect("localhost", "8002");
	auto is_connected = future.get();
	REQUIRE(is_connected);

	create_window_test(&sender, "Anil", false);//failure test
}

TEST_CASE("rendering close test", "[local_rendering_server]") {
	grt::sender sender;
	auto future = sender.sync_connect("localhost", "8002");
	auto is_connected = future.get();
	REQUIRE(is_connected);
	close_window_test(&sender, "Anil");
}
//
TEST_CASE("rendering close non existing", "[local_rendering_server]") {
	grt::sender sender;
	auto future = sender.sync_connect("localhost", "8002");
	auto is_connected = future.get();
	REQUIRE(is_connected);
	close_window_test(&sender, "Anil", false);//test failure
}
//
//
TEST_CASE("render window maximum possible test", "[local_rendering_server]") {

	grt::sender sender;
	auto future = sender.sync_connect("localhost", "8002");
	auto is_connected = future.get();
	REQUIRE(is_connected);

	create_window_test(&sender, "one");
	create_window_test(&sender, "two");
	create_window_test(&sender, "three");
	create_window_test(&sender, "four");
	create_window_test(&sender, "five");
	create_window_test(&sender, "six");
	create_window_test(&sender, "seven", false);

	close_window_test(&sender, "one");
	close_window_test(&sender, "two");
	close_window_test(&sender, "three");
	close_window_test(&sender, "four");
	close_window_test(&sender, "five");
	close_window_test(&sender, "six");
	close_window_test(&sender, "seven", false);

}
