#pragma once
// STD Includes
#include <iostream>
#include <vector>
#include <fstream>
#include <map>

// OpenGL Includes
#include <glad/glad.h>

// nVidia Includes
#include "NvEncoder/NvEncoderCuda.h"
#include "cuda.h"
#include "cuda_gl_interop.h"
#include "cuda_runtime.h"
#include "NvEncoder/NvCodecUtils.h"
#include "NvEncoder/NvEncoderCLIOptions.h"
#include "NvEncoder/ColorSpace.h"



class EncodeNVENC
{
public:
	EncodeNVENC(unsigned int cuda_device_id, unsigned int gl_pixel_buffer, unsigned int width, unsigned int height, const char* output_file_path);

	void Encode();
	void CleanupEncoder();
private:
	unsigned int cuda_device_id;
	CUcontext cuda_context;
	CUdevice cuda_device;
	NvEncoderCuda* cuda_encoder;

	struct cudaGraphicsResource* cuda_pixel_buffer;
	unsigned int gl_pixel_buffer;

	std::ofstream output_file;

	unsigned int width, height;
};
