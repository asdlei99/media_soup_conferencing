

#ifndef __DX9_RENDER__
#define __DX9_RENDER__
#include <Windows.h>
#include <d3d9.h>
#include <memory>
#include "renderer.h"

#define D3DFMT_I420 (D3DFORMAT)MAKEFOURCC('I','4','2','0')
namespace grt {
	struct deleter {
		template <class T>
		void operator()(T *ob) {
			ob->Release();
		}
	};
	template <class T>
	using wrapper = std::unique_ptr<T, deleter>;


	class directx_render :public renderer {
	public:
		void render_frame(void* hwnd, const frame_info frame) override;

	private:
		bool validate_dx_device(const int width, const int  height, frame_type frame_format);
		bool copy_rgb_data_to_surface(const unsigned char* rgb_buffer);
		bool copy_yuv_data_to_surface(const unsigned char* yuv_buffer);
		bool (directx_render::*copy_data_to_surface)(const unsigned char* raw_buffer);
		void render(const HWND hwnd);

	private:
		int width_{ 0 };
		int height_{ 0 };
		size_t buff_size_{ 0 };
		frame_type type_frame_{ frame_type::UNDEFINED };
		wrapper<IDirect3D9> d3d9_{ nullptr };
		wrapper<IDirect3DDevice9>  d3_device_{ nullptr };
		wrapper<IDirect3DSurface9>  surface_{ nullptr };
	};
} //grt
#endif