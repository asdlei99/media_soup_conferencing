#include "video_render_util.h"
#include <Windows.h>
#include <cassert>
#include "video_render/renderer.h"
#include <thread>

constexpr const wchar_t* WNDCLASS_NAME = L"Sample Window Class";
constexpr const wchar_t* WND_NAME = L"Learn to Program Windows";
namespace detail {

	class video_receiver : public grt::video_frame_callback {
	private:
		std::shared_ptr<grt::renderer> renderer_;
		HWND hwnd_;
	public:
		video_receiver(HWND hwnd, std::unique_ptr< grt::renderer>&& render)
			:renderer_{ std::move(render) }, hwnd_{ hwnd }{
			auto r = SetWindowPos(hwnd_, HWND_TOPMOST,
				0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
			assert(r != 0);
		}

		~video_receiver() {

			auto r = ShowWindow(hwnd_, SW_HIDE);
			assert(r != 0);//it shows it was previously shown
		}
		void on_frame(grt::yuv_frame frame) override {
			auto frame_info = grt::make_frame_info(
				frame.y_, frame.u_, frame.v_, frame.stride_y_,
				frame.stride_u_, frame.stride_v_, frame.w_, frame.h_);
			renderer_->render_frame(hwnd_, frame_info);
			grt::clean(frame_info);
		}
	};


	std::unique_ptr< grt::video_frame_callback>
		get_frame_receiver(HWND hwnd, std::unique_ptr< grt::renderer>&& render) {
		return std::make_unique< video_receiver>(hwnd, std::move(render));
	}
}

namespace util {

	HWND find_child_window(const wchar_t* className, const wchar_t* parent_wnd_name,
		const wchar_t* child_wnd_name) {
		auto parentWnd = FindWindow(className, parent_wnd_name);
		assert(parentWnd);
		if (parentWnd == nullptr) return nullptr;
		auto wnd = FindWindowEx(parentWnd, nullptr, className, child_wnd_name);
		assert(wnd);
		return wnd;
	}

	std::wstring to_wstring(std::string v) {
		return std::wstring(v.begin(), v.end());
	}

	std::pair<std::string, std::string>
		get_parent_wnd(std::string const& render_id) {
		return std::make_pair("Sample Window Class", "Main Window");
	}


	bool set_video_renderer(grt::video_track_receiver* receiver, std::string const& renderer_id) {
		//todo: create connection with display manager and ask for creating a window.
		assert(receiver);
		const auto info = get_parent_wnd(renderer_id);
		auto hwnd = find_child_window(to_wstring(info.first).c_str(),
			to_wstring(info.second).c_str(), to_wstring(renderer_id).c_str());
		
		assert(hwnd);//rendering application with window should be running
		if (hwnd == nullptr) return false;
		auto frame_receiver = detail::get_frame_receiver(hwnd, grt::get_renderer());
		receiver->register_callback(std::move(frame_receiver));
		return true;
	}

	void async_set_video_renderer(grt::video_track_receiver* receiver, std::string const& renderer_id) {
		std::thread{ set_video_renderer, receiver, renderer_id }.detach();
	}

	HWND
		create_rendering_window(HINSTANCE hInstance, WNDPROC wndproc) {
		WNDCLASS wc = { };

		wc.lpfnWndProc = wndproc;
		wc.hInstance = hInstance;
		wc.lpszClassName = WNDCLASS_NAME;

		RegisterClass(&wc);

		// Create the window.

		HWND hwnd = CreateWindowEx(
			0,                              // Optional window styles.
			WNDCLASS_NAME,                     // Window class
			WND_NAME,    // Window text
			WS_OVERLAPPEDWINDOW,            // Window style

			// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

			NULL,       // Parent window    
			NULL,       // Menu
			hInstance,  // Instance handle
			NULL        // Additional application data
		);
		return hwnd;
	}

	
}//namespace util