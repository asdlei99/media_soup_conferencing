
#include "dirextx_render.h"
#include <cassert>
using namespace std;

namespace grt {
		
	bool directx_render::validate_dx_device(const int width, const int  height, frame_type frame_format) {
		if (d3d9_ && (width_ == width) && (height_ == height) && type_frame_ == frame_format)
			return true;
		assert(frame_type::UNDEFINED != frame_format);

		wrapper<IDirect3D9> d3d9_temp(Direct3DCreate9(D3D_SDK_VERSION));
		D3DDISPLAYMODE d3ddm;
		//Retrieves the current display mode of the adapter.
		HRESULT hr = d3d9_temp->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
		if (hr != S_OK)
			return false;
		/*Determines whether a surface format is available as a specified resource
		type and can be used as a texture, depth-stencil buffer, or render target,
		or any combination of the three, on a device representing this adapter.*/
		hr = d3d9_temp->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16);
		if (hr != S_OK)
			return false;

		D3DCAPS9 d3dCaps;
		//The GetDeviceCaps function retrieves device-specific information for the specified device.
		hr = d3d9_temp->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps);
		if (hr != S_OK)
			return false;

		D3DFORMAT data_format_ = d3ddm.Format;
		if (frame_format == frame_type::YUV420) {
			data_format_ = D3DFMT_I420;
			//Conversion from DirectX default format to YUV420
			hr = d3d9_temp->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, 0, D3DRTYPE_SURFACE, data_format_);
			if (hr != S_OK)
				return false;
			hr = d3d9_temp->CheckDeviceFormatConversion(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, data_format_, d3ddm.Format);
			if (hr != S_OK)
				return false;
		}

		DWORD dwBehaviorFlags = 0;
		if (d3dCaps.VertexProcessingCaps != 0)
			dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else
			dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

		D3DPRESENT_PARAMETERS D3DParameter;
		ZeroMemory(&D3DParameter, sizeof(D3DParameter));                       // clear out the struct for use
		D3DParameter.Windowed = true;                                // program fullscreen, not windowed
		D3DParameter.SwapEffect = D3DSWAPEFFECT_DISCARD;               // discard old frames
		D3DParameter.BackBufferFormat = d3ddm.Format;
		D3DParameter.BackBufferWidth = width;                        // set the width of the buffer
		D3DParameter.BackBufferHeight = height;                      // set the height of the buffer
		D3DParameter.BackBufferCount = 1;                              //We only need a single back buffer
		D3DParameter.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT; //Default Refresh Rate
		D3DParameter.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;   //Default Presentation rate

		IDirect3DDevice9* d3_device_temp;
		//Creates a device to represent the display adapter.
		hr = d3d9_temp->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, /*hwnd*/NULL, dwBehaviorFlags, &D3DParameter, &d3_device_temp);
		if (hr != S_OK)
			return false;
		wrapper<IDirect3DDevice9> d3_device_wrap{ d3_device_temp };

		IDirect3DSurface9* surface_temp;
		//Create an off-screen surface.
		hr = d3_device_wrap->CreateOffscreenPlainSurface(width, height, data_format_, D3DPOOL_DEFAULT, &surface_temp, NULL);
		if (hr != S_OK)
			return false;
		wrapper<IDirect3DSurface9> surface_wrap{ surface_temp };

		if (frame_format == frame_type::RGB) {
			copy_data_to_surface = &directx_render::copy_rgb_data_to_surface;
			buff_size_ = width * height * 4;
		}
		else {
			copy_data_to_surface = &directx_render::copy_yuv_data_to_surface;
			buff_size_ = (width * height * 3) / 2;
		}

		width_ = width;
		height_ = height;
		type_frame_ = frame_format;
		surface_ = move(surface_wrap);
		d3_device_ = move(d3_device_wrap);
		d3d9_ = move(d3d9_temp);
		return true;

	}

	void directx_render::render_frame(void* hwnd, const frame_info frame) {
		HWND win_h = static_cast<HWND>(hwnd);
		if (validate_dx_device(frame.width_, frame.height_, frame.type_)) {
			if ((this->*copy_data_to_surface)(frame.frame_data_)) {
				render(win_h);
			}
		}else {// todo: if it will failed again and again then what to do.
			assert(false);
		}
	}

	bool directx_render::copy_rgb_data_to_surface(const unsigned char* rgb_buffer) {
		HRESULT hr = d3_device_->TestCooperativeLevel();
		if (hr != S_OK)
		{
			d3_device_.reset(nullptr);
			return false;
		}
		D3DLOCKED_RECT lock;
		//Locks a rectangle on a surface.
		hr = surface_->LockRect(&lock, 0, D3DLOCK_DISCARD);
		if (hr != S_OK)
			return false;

		unsigned char *data = (unsigned char*)lock.pBits;
		/*int pitch = lock.Pitch;

		unsigned char* temp = const_cast<unsigned char*>(rgb_buffer);
				
		for (int k = 0; k < 4; k++)
			for (int i = 0; i < height_; i++) {
				memcpy(data, temp, width_);
				data += width_;
				temp += (width_);
			}*/
		memcpy(data, rgb_buffer, buff_size_); //todo : This will be removed when frame will come with it's stride/pitch info.
		surface_->UnlockRect();

		return true;

	}

	//todo : This will be used when frame will come with it's stride/pitch info.
	inline unsigned char* copy_to_memory( 
		const unsigned char* source, int source_width_to_copy, int height_till_copy,
		unsigned char* dstLocation, int dist_pitch, int source_stride) {

		for (int i = 0; i < height_till_copy; i++)
		{
			memcpy(dstLocation, source, source_width_to_copy);
			dstLocation += dist_pitch;
			source += source_stride;
		}
		return dstLocation;
	}

	bool directx_render::copy_yuv_data_to_surface(const unsigned char* yuv_buffer)
	{
		HRESULT hr = d3_device_->TestCooperativeLevel();
		if (hr != S_OK)
		{
			d3_device_.reset(nullptr);
			return false;
		}
		D3DLOCKED_RECT lock;
		//Locks a rectangle on a surface.
		hr = surface_->LockRect(&lock, 0, D3DLOCK_DISCARD);
		if (hr != S_OK)
			return false;

		unsigned char *data = (unsigned char*)lock.pBits;
		//int pitch = lock.Pitch;

		memcpy(data, yuv_buffer, buff_size_);	//todo : This will be removed when frame will come with it's stride/pitch info.
		//data = copy_to_memory(yuv_buffer, width_, height_, data, pitch, width_);
		/*const auto half_width{ width_ >> 1 };
		const auto half_height{ height_ >> 1 };
		const auto half_pitch{ pitch >> 1 };
		
		data = copy_to_memory(yuv_buffer, width_, height_, data, pitch, width_);
		data = copy_to_memory(yuv_buffer, half_width, half_height, data, half_pitch, half_width);
		data = copy_to_memory(yuv_buffer, half_width, half_height, data, half_pitch, half_width);*/

		surface_->UnlockRect();
		return true;
	}

	void directx_render::render(const HWND hwnd) {
		/*Applications must call IDirect3DDevice9::BeginScene before performing
		any rendering and must call IDirect3DDevice9::EndScene when rendering is
		complete and before calling IDirect3DDevice9::BeginScene again.*/
		if (SUCCEEDED(d3_device_->BeginScene()))
		{
			LPDIRECT3DSURFACE9 BackBuffer{ 0 };
			//Retrieves a back buffer from the device's swap chain.
			if (FAILED(d3_device_->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer)))
				MessageBox(NULL, TEXT("GetBackBuffer"), TEXT("Error"), MB_OK);	//todo: this messagebox has to be removed with throw.


			//Copies rectangular subsets of pixels from one surface to another.
			if (FAILED(d3_device_->StretchRect(surface_.get(), NULL, BackBuffer, NULL, D3DTEXF_LINEAR)))
				MessageBox(NULL, TEXT("StretchRect"), TEXT("Error"), MB_OK);//todo: this messagebox has to be removed with throw.

			BackBuffer->Release();
			d3_device_->EndScene();
		}
		d3_device_->Present(0, 0, hwnd, 0);
	}

}// grt