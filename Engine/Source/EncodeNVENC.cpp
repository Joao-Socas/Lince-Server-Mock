#include "EncodeNVENC.hpp"

EncodeNVENC::EncodeNVENC(unsigned int cuda_device_id, unsigned int gl_pixel_buffer, unsigned int width, unsigned int height, const char* output_file_path) :
    cuda_device_id(cuda_device_id), gl_pixel_buffer(gl_pixel_buffer), width(width), height(height)
{
    // For now, hardcoded GUIDS
    GUID encoder_GUID = NV_ENC_CODEC_H264_GUID;
    GUID preset_GUID = NV_ENC_PRESET_P3_GUID;
    NV_ENC_BUFFER_FORMAT buffer_format = NV_ENC_BUFFER_FORMAT_IYUV;
    NV_ENC_TUNING_INFO tuning_info = NV_ENC_TUNING_INFO_LOW_LATENCY;
    // --------------------------------------------- CUDA AND GUIDs VALIDATION START --------------------------------------------- //
    if (cuInit(0) < 0)
    {
        std::cout << "CUDA driver initializing error!" << std::endl;
        return;
    }
    int n_devices = 0;
    if (cuDeviceGetCount(&n_devices) < 0)
    {
        std::cout << "Could not assert the number of CUDA devices!" << std::endl;
        return;
    }
    if (cuda_device_id < 0 || cuda_device_id >= n_devices)
    {
        std::cout << "Requested device ID not found. The device ID must be between 0 and " << n_devices - 1 << std::endl;
        return;
    }
    if (cuDeviceGet(&cuda_device, cuda_device_id) < 0)
    {
        std::cout << "Error getting the device with following ID: " << cuda_device_id << std::endl;
        return;
    }
    if (cuCtxCreate(&cuda_context, 0, cuda_device) < 0)
    {
        std::cout << "Error on CUDA context creation!" << std::endl;
        return;
    }
    cuda_encoder = new NvEncoderCuda(cuda_context, width, height, buffer_format);
    unsigned int encode_GUID_count = cuda_encoder->GetEncodeGUIDCount();
    if (encode_GUID_count < 1)
    {
        std::cout << "No compatible encode GUIDs with this device" << std::endl;
        return;
    }
    std::vector<GUID> encode_GUID_list;
    encode_GUID_list.resize(encode_GUID_count);
    cuda_encoder->GetEncodeList(encode_GUID_list.data(), encode_GUID_count, &encode_GUID_count);
    if (std::find(encode_GUID_list.begin(), encode_GUID_list.end(), encoder_GUID) == encode_GUID_list.end())
    {
        std::cout << "Selected encoder GUID not compatible with this device" << std::endl;
        return;
    }
    unsigned int encode_preset_GUID_count = cuda_encoder->GetEncodePresetCount(encoder_GUID);
    if (encode_preset_GUID_count < 1)
    {
        std::cout << "No presets were found for this encode GUID!" << std::endl;
        return;
    }
    std::vector<GUID> encode_preset_GUID_list;
    encode_preset_GUID_list.resize(encode_preset_GUID_count);
    cuda_encoder->GetEncodePresetList(encoder_GUID, encode_preset_GUID_list.data(), encode_preset_GUID_count, &encode_preset_GUID_count);
    if (std::find(encode_preset_GUID_list.begin(), encode_preset_GUID_list.end(), preset_GUID) == encode_preset_GUID_list.end())
    {
        std::cout << "Selected preset not found!" << std::endl;
        return;
    }
    unsigned int input_format_count = cuda_encoder->GetInputFormatCount(encoder_GUID);
    if (input_format_count < 1)
    {
        std::cout << "Error getting compatible input formats!" << std::endl;
        return;
    }
    std::vector<NV_ENC_BUFFER_FORMAT> input_format_list;
    input_format_list.resize(encode_preset_GUID_count);
    cuda_encoder->GetInputFormats(encoder_GUID, input_format_list.data(), input_format_count, &input_format_count);
    if (std::find(input_format_list.begin(), input_format_list.end(), buffer_format) == input_format_list.end())
    {
        std::cout << "Selected input format not found!" << std::endl;
        return;
    }
    // --------------------------------------------- CUDA AND GUIDs VALIDATION END --------------------------------------------- //
    // Encoder configuration
    NV_ENC_INITIALIZE_PARAMS initialize_params = { NV_ENC_INITIALIZE_PARAMS_VER };
    NV_ENC_CONFIG encode_config = { NV_ENC_CONFIG_VER };
    NvEncoderInitParam encode_options;
    initialize_params.encodeConfig = &encode_config;
    encode_options.SetInitParams(&initialize_params, buffer_format);
    cuda_encoder->CreateDefaultEncoderParams(&initialize_params, encoder_GUID, preset_GUID, tuning_info);
    //encode_config.gopLength = 1;
    //encode_config.frameIntervalP = 0;
    //initialize_params.enableEncodeAsync = false;
    //encode_config.encodeCodecConfig.h264Config.sliceMode = 0;
    //encode_config.encodeCodecConfig.h264Config.sliceModeData = 0;
    cuda_encoder->CreateEncoder(&initialize_params);
    // OpenGL pixel buffer registration as cuda resource
    cudaError_t result = cudaGraphicsGLRegisterBuffer(&cuda_pixel_buffer, gl_pixel_buffer, cudaGraphicsRegisterFlagsReadOnly);
    if (result != cudaError::cudaSuccess)
    {
        std::cout << "Failed to register OpenGL pixel buffer as a cuda resource!" << std::endl;
    }
    // File oppening
    output_file.open(output_file_path, std::ios::out, std::ios::binary);
}

void EncodeNVENC::Encode()
{
    float* mapped_cuda_pixel_buffer = nullptr;
    size_t num_bytes;

    //Mapping the resource
    cudaError_t result = cudaGraphicsMapResources(1, &cuda_pixel_buffer);
    if (result != cudaError::cudaSuccess)
    {
        std::cout << "Failed to map the cuda resource" << std::endl;
        return;
    }
    result = cudaGraphicsResourceGetMappedPointer((void**)&mapped_cuda_pixel_buffer, &num_bytes, cuda_pixel_buffer);
    if (result != cudaError::cudaSuccess)
    {
        std::cout << "Failed to get mapped cuda resource pointer" << std::endl;
        return;
    }

    // Transfer data to the input frame
    const NvEncInputFrame* input_frame = cuda_encoder->GetNextInputFrame();


    RGBAFtoYUVUC(width, height, (CUdeviceptr)mapped_cuda_pixel_buffer, (CUdeviceptr)input_frame->inputPtr);

    //NvEncoderCuda::CopyToDeviceFrame(cuda_context, mapped_cuda_pixel_buffer, 0, (CUdeviceptr)input_frame->inputPtr,
    //    (int)input_frame->pitch,
    //    width,
    //    height,
    //    CU_MEMORYTYPE_DEVICE,
    //    NV_ENC_BUFFER_FORMAT_ABGR,
    //    input_frame->chromaOffsets,
    //    input_frame->numChromaPlanes);
    cudaGraphicsUnmapResources(1, &cuda_pixel_buffer);

    std::vector<std::vector<uint8_t>> output_buffer{};
    cuda_encoder->EncodeFrame(output_buffer);
    for (std::vector<uint8_t>& packet : output_buffer)
    {
        output_file.write(reinterpret_cast<char*>(packet.data()), packet.size());
    }
}
void EncodeNVENC::CleanupEncoder()
{
    std::vector<std::vector<uint8_t>> output_buffer{};
    cuda_encoder->EndEncode(output_buffer);
    for (std::vector<uint8_t>& packet : output_buffer)
    {
        output_file.write(reinterpret_cast<char*>(packet.data()), packet.size());
    }
    output_file.close();
    cuda_encoder->DestroyEncoder();
    delete cuda_encoder;
}