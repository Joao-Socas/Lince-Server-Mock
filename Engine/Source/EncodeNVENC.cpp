#include "EncodeNVENC.hpp"

EncodeNVENC::EncodeNVENC(unsigned int cuda_device_id, unsigned int gl_pixel_buffer, unsigned int width, unsigned int height, const char* output_file_path) :
    cuda_device_id(cuda_device_id), gl_pixel_buffer(gl_pixel_buffer), width(width), height(height)
{
    if (cuInit(0) < 0)
    {
        std::cout << "Erro ao inciar o cuda!" << std::endl;
        return;
    }

    int n_devices = 0;
    if (cuDeviceGetCount(&n_devices) < 0)
    {
        std::cout << "Erro ao requisitar o número de dispositivos cuda" << std::endl;
        return;
    }
   
    if (cuda_device_id < 0 || cuda_device_id >= n_devices)
    {
        std::cout << "ID do dispositivo não encontrado. O id do dispositivo é um número entre 0 e " << n_devices - 1 << std::endl;
        return;
    }

    if (cuDeviceGet(&cuda_device, cuda_device_id) < 0)
    {
        std::cout << "Erro ao obter o dispositivo de ID: " << cuda_device_id << std::endl;
        return;
    }

    if (cuCtxCreate(&cuda_context, 0, cuda_device) < 0)
    {
        std::cout << "Erro na geração de contexto cuda" << std::endl;
        return;
    }


    NV_ENC_BUFFER_FORMAT buffer_format = NV_ENC_BUFFER_FORMAT_ARGB;
    cuda_encoder = new NvEncoderCuda (cuda_context, width, height, buffer_format);
    NV_ENC_INITIALIZE_PARAMS initialize_params = { NV_ENC_INITIALIZE_PARAMS_VER };
    NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };
    initialize_params.encodeConfig = &encodeConfig;
    cuda_encoder->CreateDefaultEncoderParams(&initialize_params, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_P3_GUID, NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY);
    initialize_params.bufferFormat = NV_ENC_BUFFER_FORMAT_ARGB;
    //initialize_params.outputStatsLevel = NV_ENC_OUTPUT_STATS_BLOCK_LEVEL;
    initialize_params.encodeConfig->gopLength = 2;
    //initialize_params.encodeConfig->profileGUID = NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID;
    //initialize_params.enableEncodeAsync = 0;
     //initialize_params.frameRateNum = 30;
    //initialize_params.frameRateDen = 1;
    //initialize_params.enableReconFrameOutput = 1;
    //auto& config = encodeConfig.encodeCodecConfig.h264Config;
    //config.repeatSPSPPS = 1;
    //config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_CAVLC;
    //config.maxNumRefFrames = 50;

    //encodeConfig.rcParams.lowDelayKeyFrameScale = UINT32_MAX / 10;
    //encodeConfig.rcParams.vbvBufferSize = UINT32_MAX/10;
    //encodeConfig.rcParams.vbvInitialDelay = 0;
    //encodeConfig.rcParams.maxBitRate = UINT32_MAX / 10;
    //encodeConfig.rcParams.averageBitRate = UINT32_MAX / 10;

    cuda_encoder->CreateEncoder(&initialize_params);
    cudaError_t result = cudaGraphicsGLRegisterBuffer(&cuda_render_buffer, gl_pixel_buffer, cudaGraphicsMapFlagsNone);
    if (result != cudaError::cudaSuccess)
    {
        std::cout << "Falha ao linkar o buffer OpenGl com o buffer cuda";
    }

    output_file.open(output_file_path, std::ios::out, std::ios::binary);
}

//void EncodeNVENC::SetUpFrameBuffer()
//{
//    struct cudaGraphicsResource* data_buffer_cuda;
//    
//    const NvEncInputFrame* input_frame = cuda_encoder->GetNextInputFrame();
//    NV_ENC_INPUT_RESOURCE_OPENGL_TEX* gl_resource = (NV_ENC_INPUT_RESOURCE_OPENGL_TEX*)input_frame->inputPtr;
//
//    glBindTexture(gl_resource->target, gl_resource->texture);
//    glTexImage2D(gl_resource->target, 0, GL_RGB10_A2, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl_resource->texture, 0);
//}

void EncodeNVENC::Encode()
{
    char* pixel_test = new char[width * height * 4];
    char* mapped_cuda_render_buffer;
    size_t num_bytes;
    cudaError_t result = cudaGraphicsMapResources(1, &cuda_render_buffer);
    if (result != cudaError::cudaSuccess)
    {
        std::cout << "Falha ao mapear o resource do cuda";
    }
    result = cudaGraphicsResourceGetMappedPointer((void**)&mapped_cuda_render_buffer, &num_bytes, cuda_render_buffer);
    if (result != cudaError::cudaSuccess)
    {
        std::cout << "Falha ao conseguir o ponteiro mapeado";
    }


    const NvEncInputFrame* input_frame = cuda_encoder->GetNextInputFrame(); // this is where the raw image should be drawn or copied so it can be encoded


    //glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixel_test);
    //NvEncoderCuda::CopyToDeviceFrame(cuda_context, pixel_test, 0, (CUdeviceptr)input_frame->inputPtr,
    //    (int)input_frame->pitch,
    //    cuda_encoder->GetEncodeWidth(),
    //    cuda_encoder->GetEncodeHeight(),
    //    CU_MEMORYTYPE_HOST,
    //    input_frame->bufferFormat,
    //    input_frame->chromaOffsets,
    //    input_frame->numChromaPlanes);
 
    CUresult cu_result = cuMemcpy((CUdeviceptr)input_frame->inputPtr, (CUdeviceptr)mapped_cuda_render_buffer, num_bytes);

    if (cu_result != cudaError::cudaSuccess)
    {
        std::cout << "Falha ao copiar do buffer cuda para o buffer gl";
    }

    cuda_encoder->EncodeFrame(drawing_buffer); // Outputs inside drawing_buffer the ecoded frame that was suplied on input_frame

    for (std::vector<uint8_t>& packet : drawing_buffer)
    {
        output_file.write(reinterpret_cast<char*>(packet.data()), packet.size());
    }
    cudaGraphicsUnmapResources(1, &cuda_render_buffer);
    delete[] pixel_test;
}

void EncodeNVENC::CleanupEncoder()
{
    cuda_encoder->EndEncode(drawing_buffer);
    output_file.close();
    cuda_encoder->DestroyEncoder();
    delete cuda_encoder;
}