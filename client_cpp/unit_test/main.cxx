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

void create_window_test(grt::signaller* signaller, std::shared_ptr<grt::rendering_server_client> client, std::string id
					, bool const passTest = true) {
	{
		std::promise<void> done;
		auto done_future = done.get_future();
		client->register_function(id, [&done, id, passTest](auto type, auto msg) {
			test_window(type, msg, id, passTest);
			done.set_value();

		});

		const auto m = grt::make_render_wnd_req(id);
		signaller->send(m);

		done_future.get();//wait for event.
		client->unregister_function(id);
	}
}

void close_window_test(grt::signaller* signaller, std::shared_ptr<grt::rendering_server_client> client, std::string id,
	const bool passTest = true) {
	std::promise<void> wait_event;
	auto wait = wait_event.get_future();
	client->register_function(id, [&wait_event, id, passTest](auto type, auto msg) {
		assert(type == grt::message_type::wnd_close_req_res);
		auto result = absl::any_cast<std::pair<bool, std::string>>(msg);
		
		REQUIRE(result.second == id);
		if(passTest) REQUIRE(result.first);
		else REQUIRE_FALSE(result.first);
		wait_event.set_value();

	});

	const auto m = grt::make_render_wnd_close_req(id);
	signaller->send(m);
	wait.get();//wait for event;
	client->unregister_function(id);
}



TEST_CASE("server connection test", "[local_rendering_server]") {
	
	grt::websocket_signaller signaller;
	auto client = std::make_shared<grt::rendering_server_client>();

	std::promise<bool> connect_event;
	auto future = connect_event.get_future();

	client->set_connect_event(std::move(connect_event));
	
	signaller.connect("localhost", "8002", client);

	auto connect_status = future.get();
	REQUIRE(connect_status);

	create_window_test(&signaller, client, "Anil");
	
	signaller.disconnect();

}

////
TEST_CASE("rendering wind multiple create", "[local_rendering_server]") {
	grt::websocket_signaller signaller;
	auto client = std::make_shared<grt::rendering_server_client>();
	
	
	std::promise<bool> connect_event;
	auto future = connect_event.get_future();

	client->set_connect_event(std::move(connect_event));

	//signaller.set_callback(client);
	signaller.connect("localhost", "8002", client);

	auto connect_status = future.get();
	REQUIRE(connect_status);

	create_window_test(&signaller, client, "Anil", false);//test failure 
	
	signaller.disconnect();
}

TEST_CASE("rendering close test", "[local_rendering_server]") {
	grt::websocket_signaller signaller;
	auto client = std::make_shared<grt::rendering_server_client>();
	
	{
		std::promise<bool> connect_event;
		auto future = connect_event.get_future();

		client->set_connect_event(std::move(connect_event));

		//signaller.set_callback(client);
		signaller.connect("localhost", "8002", client);

		future.get();//WAIT FOR EVENT
		
	}
	close_window_test(&signaller, client, "Anil");

	signaller.disconnect();
	
}
//
TEST_CASE("rendering close non existing", "[local_rendering_server]") {
	grt::websocket_signaller signaller;
	auto client = std::make_shared<grt::rendering_server_client>();
	
		std::promise<bool> connect_event;
		auto future = connect_event.get_future();
		client->set_connect_event(std::move(connect_event));
		signaller.connect("localhost", "8002", client);

		future.get();//WAIT FOR connect
	
		close_window_test(&signaller, client, "Anil", false); //failure test
		
	
	signaller.disconnect();
	
}


TEST_CASE("render window maximum possible test", "[local_rendering_server]") {
	
	grt::websocket_signaller signaller;
	auto client = std::make_shared<grt::rendering_server_client>();
	//const std::string my_id = "Anil";
	
	std::promise<bool> connect_event;
	auto future = connect_event.get_future();

	client->set_connect_event(std::move(connect_event));

	signaller.connect("localhost", "8002", client);

	auto connect_status = future.get();
	REQUIRE(connect_status);

	create_window_test(&signaller, client, "one");
	create_window_test(&signaller, client, "two");
	create_window_test(&signaller, client, "three");
	create_window_test(&signaller, client, "four");
	create_window_test(&signaller, client, "five");
	create_window_test(&signaller, client, "six");
	create_window_test(&signaller, client, "seven", false);

	close_window_test(&signaller, client, "one");
	close_window_test(&signaller, client, "two");
	close_window_test(&signaller, client, "three");
	close_window_test(&signaller, client, "four");
	close_window_test(&signaller, client, "five");
	close_window_test(&signaller, client, "six");
	close_window_test(&signaller, client, "seven", false);

	signaller.disconnect();
}
