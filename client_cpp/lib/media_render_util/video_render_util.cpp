#include "video_render_util.h"
#include <cassert>
#include "video_render/renderer.h"
#include <thread>
#include "server_communication_util/util.h"
#include "windows_util/windows_util.h"
#include "json_parser.h"
#include "server_communication_util/rendering_server_client.h"
#include "media_receiver/video_receiver/video_track_receiver.h"

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

	bool set_video_renderer(grt::video_track_receiver* receiver, std::string class_name, 
		std::string parent_name, std::string  renderer_id) {
		//todo: create connection with display manager and ask for creating a window.
		assert(receiver);

		auto hwnd = find_child_window(class_name, parent_name, renderer_id);
		
		assert(hwnd);//rendering application with window should be running
		if (hwnd == nullptr) return false;
		auto frame_receiver = detail::get_frame_receiver(hwnd, grt::get_renderer());
		receiver->register_callback(std::move(frame_receiver));
		return true;
	}

	void async_set_video_renderer(grt::video_track_receiver* recevier, grt::sender* sender, std::string const& id) {
		const auto m = grt::make_render_wnd_req(id);
		sender->send_to_renderer(id, m, [recevier, sender, id](grt::message_type type, absl::any msg) {
			assert(type == grt::message_type::window_create_res);
			auto wndInfo = absl::any_cast<grt::wnd_create_res>(msg);
			assert(wndInfo.status_);
			const auto className = wndInfo.class_name_;
			const auto wndName = wndInfo.parent_wnd_name_;
			const auto wndId = wndInfo.child_wnd_id_;
		
			set_video_renderer(recevier, className, wndName, wndId);
			sender->done(id);
		});
	}

	
}//namespace util