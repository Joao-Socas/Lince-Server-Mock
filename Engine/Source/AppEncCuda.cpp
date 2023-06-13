/*
* Copyright 2017-2023 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/

/**
*  This sample application illustrates encoding of frames in CUDA device buffers.
*  The application reads the image data from file and loads it to CUDA input
*  buffers obtained from the encoder using NvEncoder::GetNextInputFrame().
*  The encoder subsequently maps the CUDA buffers for encoder using NvEncodeAPI
*  and submits them to NVENC hardware for encoding as part of EncodeFrame() function.
*  The NVENC hardware output is written in system memory for this case.
*
*  This sample application also illustrates the use of video memory buffer allocated
*  by the application to get the NVENC hardware output. This feature can be used
*  for H264 ME-only mode, H264 encode and HEVC encode. This application copies the NVENC output
*  from video memory buffer to host memory buffer in order to dump to a file, but this
*  is not needed if application choose to use it in some other way.
*
*  Since, encoding may involve CUDA pre-processing on the input and post-processing on
*  output, use of CUDA streams is also illustrated to pipeline the CUDA pre-processing
*  and post-processing tasks, for output in video memory case.
*
*  CUDA streams can be used for H.264 ME-only, HEVC ME-only, H264 encode, HEVC encode and AV1 encode.
*/

#include <fstream>
#include <iostream>
#include <memory>
#include <cuda.h>
#include "NvEncoder/NvCodecUtils.h"
#include "NvEncoder/NvEncoderCuda.h"
#include "NvEncoder/NvEncoderOutputInVidMemCuda.h"
#include "NvEncoder/NvEncoderCLIOptions.h"

template<class EncoderClass>
void InitializeEncoder(EncoderClass &pEnc, NvEncoderInitParam encodeCLIOptions, NV_ENC_BUFFER_FORMAT buffer_format)
{
	NV_ENC_INITIALIZE_PARAMS initializeParams = { NV_ENC_INITIALIZE_PARAMS_VER };
	NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };

	initializeParams.encodeConfig = &encodeConfig;
	pEnc->CreateDefaultEncoderParams(&initializeParams, encodeCLIOptions.GetEncodeGUID(), encodeCLIOptions.GetPresetGUID(), encodeCLIOptions.GetTuningInfo());
	encodeCLIOptions.SetInitParams(&initializeParams, buffer_format);

	pEnc->CreateEncoder(&initializeParams);
}

void EncodeCuda(int nWidth, int nHeight, NV_ENC_BUFFER_FORMAT buffer_format, NvEncoderInitParam encodeCLIOptions, CUcontext cuContext, std::ifstream &fpIn, std::ofstream &fpOut)
{
	std::unique_ptr<NvEncoderCuda> pEnc(new NvEncoderCuda(cuContext, nWidth, nHeight, buffer_format)); 

	InitializeEncoder(pEnc, encodeCLIOptions, buffer_format);

	int nFrameSize = pEnc->GetFrameSize();

	std::unique_ptr<uint8_t[]> pHostFrame(new uint8_t[nFrameSize]);
	int nFrame = 0;
	while (true)
	{
		// Load the next frame from disk
		std::streamsize nRead = fpIn.read(reinterpret_cast<char*>(pHostFrame.get()), nFrameSize).gcount();
		// For receiving encoded packets
		std::vector<std::vector<uint8_t>> vPacket;
		if (nRead == nFrameSize)
		{
			const NvEncInputFrame* encoderInputFrame = pEnc->GetNextInputFrame();
			NvEncoderCuda::CopyToDeviceFrame(cuContext, pHostFrame.get(), 0, (CUdeviceptr)encoderInputFrame->inputPtr,
				(int)encoderInputFrame->pitch,
				pEnc->GetEncodeWidth(),
				pEnc->GetEncodeHeight(),
				CU_MEMORYTYPE_HOST,
				encoderInputFrame->bufferFormat,
				encoderInputFrame->chromaOffsets,
				encoderInputFrame->numChromaPlanes);

			pEnc->EncodeFrame(vPacket);
		}
		else
		{
			pEnc->EndEncode(vPacket);
		}
		nFrame += (int)vPacket.size();
		for (std::vector<uint8_t> &packet : vPacket)
		{
			// For each encoded packet
			fpOut.write(reinterpret_cast<char*>(packet.data()), packet.size());
		}

		if (nRead != nFrameSize) break;
	}

	pEnc->DestroyEncoder();

	std::cout << "Total frames encoded: " << nFrame << std::endl;
}

int notmain()
{
	char szInFilePath[256] = "D:\\Projetos\\Lince Server Mock\\sample.yuv";
	char szOutFilePath[256] = "D:\\Projetos\\Lince Server Mock\\sampleout.h264";
	int nWidth = 176, nHeight = 144;
	NV_ENC_BUFFER_FORMAT buffer_format = NV_ENC_BUFFER_FORMAT_IYUV;
	int iGpu = 0;
	try
	{
		NvEncoderInitParam encodeCLIOptions;
		int cuStreamType = -1;
		bool bOutputInVideoMem = false;

		CheckInputFile(szInFilePath);
		ValidateResolution(nWidth, nHeight);

		if (!*szOutFilePath)
		{
			sprintf_s(szOutFilePath, std::strlen(szOutFilePath), encodeCLIOptions.IsCodecH264() ? "out.h264" : "out.hevc");
		}

		ck(cuInit(0));
		int nGpu = 0;
		ck(cuDeviceGetCount(&nGpu));
		if (iGpu < 0 || iGpu >= nGpu)
		{
			std::cout << "GPU ordinal out of range. Should be within [" << 0 << ", " << nGpu - 1 << "]" << std::endl;
			return 1;
		}
		CUdevice cuDevice = 0;
		ck(cuDeviceGet(&cuDevice, iGpu));
		char szDeviceName[80];
		ck(cuDeviceGetName(szDeviceName, sizeof(szDeviceName), cuDevice));
		std::cout << "GPU in use: " << szDeviceName << std::endl;
		CUcontext cuContext = NULL;
		ck(cuCtxCreate(&cuContext, 0, cuDevice));

		// Open input file
		std::ifstream fpIn(szInFilePath, std::ifstream::in | std::ifstream::binary);
		if (!fpIn)
		{
			std::ostringstream err;
			err << "Unable to open input file: " << szInFilePath << std::endl;
			throw std::invalid_argument(err.str());
		}

		// Open output file
		std::ofstream fpOut(szOutFilePath, std::ios::out | std::ios::binary);
		if (!fpOut)
		{
			std::ostringstream err;
			err << "Unable to open output file: " << szOutFilePath << std::endl;
			throw std::invalid_argument(err.str());
		}
		EncodeCuda(nWidth, nHeight, buffer_format, encodeCLIOptions, cuContext, fpIn, fpOut);

		fpOut.close();
		fpIn.close();

		std::cout << "Bitstream saved in file " << szOutFilePath << std::endl;
	}
	catch (const std::exception &ex)
	{
		std::cout << ex.what();
		return 1;
	}
	return 0;
}
