#pragma once
// STD Includes
#include <iostream>
#include <vector>

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
	EncodeNVENC(unsigned int cuda_device_id, unsigned int gl_render_buffer, unsigned int width, unsigned int height);
	
	//void SetUpFrameBuffer();
	void Encode(std::vector<std::vector<uint8_t>> drawing_buffer);
	void CleanupEncoder(std::vector<std::vector<uint8_t>> drawing_buffer);

	CUcontext cuda_context;
	CUdevice cuda_device;
	NvEncoderCuda* cuda_encoder;
	
	
	unsigned int width, height;
	unsigned int cuda_device_id;
	unsigned int gl_render_buffer;
	struct cudaGraphicsResource* cuda_render_buffer;

};

