#ifndef __RENDERER__
#define __RENDERER__
#include <memory>

namespace grt {
	enum class frame_type {RGB, YUV420, UNDEFINED};
	struct frame_info {
		frame_info(unsigned char* data, int w, int h, frame_type type) :frame_data_{ data }, width_{ w }, height_{ h }, type_{type} {
		}
		const frame_type type_{ frame_type::UNDEFINED };
		const unsigned char* frame_data_{ nullptr };
		const int width_{ 0 };
		const int height_{ 0 };
	};

	frame_info make_frame_info(const uint8_t* y, 
		const uint8_t* u, const uint8_t* v,
		size_t y_pitch, size_t u_pitch, size_t v_pitch, int w, int h);
	void clean(frame_info);
	class renderer {
	public:
		virtual void render_frame(void* hwnd, frame_info frame)=0;
	};

	std::unique_ptr<renderer>
		get_renderer();
}

#endif // !__RENDERER__

