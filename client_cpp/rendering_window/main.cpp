#include <Windows.h>
#include <thread>
#include <cassert>
#include "video_render_util.h"
#include "websocket_server/websocket_server.h"
#include <future>
#include "display_manager/layout_manager.h"
#include "display_manager/window_creater.h"


constexpr const wchar_t* WNDCLASS_NAME = L"Sample Window Class";


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//constexpr const char* REC_ID = "3245";
#if 1
class server_handler {
private:
	 util::func_thread_handler func_object_;
	 std::future<void> handle_server_;
	 std::unique_ptr<display::window_creator> main_wnd_;
public:
	explicit server_handler(std::unique_ptr<display::window_creator> main_wnd_);
	void start_server(unsigned short port);

	void message_from_server(std::string msg);

};

server_handler::server_handler(std::unique_ptr<display::window_creator> main_wnd_)
	:main_wnd_{ std::move(main_wnd_) } {

}

void server_handler::start_server(unsigned short port) {
	func_object_.register_id(
		REC_ID,
		std::bind(&server_handler::message_from_server, this, std::placeholders::_1)
	);
	
	handle_server_ = std::async(std::launch::async, &grt::start_server_block, port, 1, &func_object_, REC_ID);
}

void server_handler::message_from_server(std::string msg) {
	assert(false);//todo: parse messae here

}

#endif//

HWND find_child_window(const wchar_t* className, const wchar_t* parent_wnd_name,
	const wchar_t* child_wnd_name) {
	auto parentWnd = FindWindow(className, parent_wnd_name);
	assert(parentWnd);
	if (parentWnd == nullptr) return nullptr;
	auto wnd = FindWindowEx(parentWnd, nullptr, className, child_wnd_name);
	assert(wnd);
	return wnd;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = WNDCLASS_NAME;

	RegisterClass(&wc);

	auto creator = display::get_wnd_creator(hInstance, WNDCLASS_NAME);
	auto hwnd = creator->get_handle();
	
	//server_handler server_handler_{ std::move(creator) };
	if (hwnd == NULL) {
		return 0;
	}

	auto got_hwd = FindWindow(display::get_default_class_name(), display::get_default_wnd_name());
	assert(hwnd == got_hwd);

	auto p = creator->create_window( L"anil");
	assert(p);
	auto child_wnd = find_child_window(display::get_default_class_name(), display::get_default_wnd_name(), L"anil");
	assert(child_wnd);
	//auto future = std::async(std::launch::async, []() {
						/*int i = 0;
						auto parentWnd = FindWindow(display::get_default_class_name(), display::get_default_wnd_name());
						assert(parentWnd);
						while (true) {
							if (i > 20) break;
							auto wnd = FindWindowEx(parentWnd, nullptr, display::get_default_class_name(), L"anil");
							if (wnd) break;
							i++;
							std::this_thread::sleep_for(std::chrono::seconds(1));
						}
						assert(i < 20);*/
						//});
	
	//server_handler_.start_server(8088);

	ShowWindow(hwnd, nCmdShow);

	p->reposition(24, 50, 800, 600);

	// Run the message loop.

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(hwnd, &ps);
	}
	return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}