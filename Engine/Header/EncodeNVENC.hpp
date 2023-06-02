#pragma once
// STD Includes
#include <iostream>
#include <vector>
#include <fstream>

// OpenGL Includes
#include <glad/glad.h>

// nVidia Includes
#include "NvEncoder/NvEncoderCuda.h"
#include "cuda.h"
#include "cuda_gl_interop.h"
#include "cuda_runtime.h"



class EncodeNVENC
{
public:
	EncodeNVENC(unsigned int cuda_device_id, unsigned int gl_pixel_buffer, unsigned int width, unsigned int height, const char* output_file_path);
	
	//void SetUpFrameBuffer();
	void Encode();
	void CleanupEncoder();

	CUcontext cuda_context;
	CUdevice cuda_device;
	NvEncoderCuda* cuda_encoder;
	
	std::ofstream output_file;
	
	unsigned int width, height;
	unsigned int cuda_device_id;
	unsigned int gl_pixel_buffer;
	struct cudaGraphicsResource* cuda_render_buffer;
	std::vector<std::vector<uint8_t>> drawing_buffer{};

};

