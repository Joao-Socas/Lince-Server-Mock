#include "EncodeNVENC.hpp"

EncodeNVENC::EncodeNVENC(unsigned int cuda_device_id, unsigned int gl_render_buffer, unsigned int width, unsigned int height) : 
    cuda_device_id(cuda_device_id), gl_render_buffer(gl_render_buffer), width(width), height(height)
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


    NV_ENC_BUFFER_FORMAT buffer_format = NV_ENC_BUFFER_FORMAT_ARGB10;
    cuda_encoder = new NvEncoderCuda (cuda_context, width, height, buffer_format, 0);
    NV_ENC_INITIALIZE_PARAMS initialize_params = { NV_ENC_INITIALIZE_PARAMS_VER };
    NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };

    initialize_params.encodeConfig = &encodeConfig;
    cuda_encoder->CreateDefaultEncoderParams(&initialize_params, NV_ENC_CODEC_HEVC_GUID, NV_ENC_PRESET_DEFAULT_GUID);
    initialize_params.encodeWidth = 800;
    initialize_params.encodeHeight = 600;

    encodeConfig.gopLength = NVENC_INFINITE_GOPLENGTH;
    encodeConfig.frameIntervalP = 1; // IPP frame compress reference mode
    encodeConfig.encodeCodecConfig.hevcConfig.idrPeriod = NVENC_INFINITE_GOPLENGTH;
    encodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;
    encodeConfig.rcParams.averageBitRate = (static_cast<unsigned int>(5.0f * initialize_params.encodeWidth * initialize_params.encodeHeight) / (1280 * 720)) * 10000;
    encodeConfig.rcParams.vbvBufferSize = (encodeConfig.rcParams.averageBitRate * initialize_params.frameRateDen / initialize_params.frameRateNum) * 5;
    encodeConfig.rcParams.maxBitRate = encodeConfig.rcParams.averageBitRate;
    encodeConfig.rcParams.vbvInitialDelay = encodeConfig.rcParams.vbvBufferSize;

    cuda_encoder->CreateEncoder(&initialize_params);
    cudaGraphicsGLRegisterImage(&cuda_render_buffer, gl_render_buffer, cudaGraphicsMapFlagsNone, GL_RENDERBUFFER);


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

void EncodeNVENC::Encode(std::vector<std::vector<uint8_t>> drawing_buffer)
{
    char* mapped_cuda_render_buffer;
    size_t num_bytes;
    cudaGraphicsMapResources(1, &cuda_render_buffer);
    cudaGraphicsResourceGetMappedPointer((void**)&mapped_cuda_render_buffer, &num_bytes, cuda_render_buffer);


    const NvEncInputFrame* input_frame = cuda_encoder->GetNextInputFrame(); // this is where the raw image should be drawn or copied so it can be encoded
    cuMemcpy((CUdeviceptr)input_frame->inputPtr, (CUdeviceptr)mapped_cuda_render_buffer, num_bytes);
    cuda_encoder->EncodeFrame(drawing_buffer); // Outputs inside drawing_buffer the ecoded frame that was suplied on input_frame
}

void EncodeNVENC::CleanupEncoder(std::vector<std::vector<uint8_t>> drawing_buffer)
{
    cuda_encoder->EndEncode(drawing_buffer);
    delete cuda_encoder;
}