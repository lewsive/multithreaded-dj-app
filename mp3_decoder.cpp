#include <cstdio>
#include <iostream>

// Define the implementation before including the header.
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

// This function accepts an MP3 file path, decodes it to PCM data,
// and prints basic information (sample rate, channels, total frames).
void decodeMP3(const char* filename) {
    drmp3_config config;
    drmp3_uint64 totalPCMFrameCount = 0;
    
    // Open and decode the MP3 file to 32-bit floating point PCM data.
    float* pSampleData = drmp3_open_file_and_read_pcm_frames_f32(filename, &config, &totalPCMFrameCount, nullptr);
    if (pSampleData == nullptr) {
        std::fprintf(stderr, "Failed to open or decode MP3 file: %s\n", filename);
        return;
    }
    
    std::printf("Loaded file: %s\n", filename);
    std::printf("Sample Rate: %u Hz\n", config.sampleRate);
    std::printf("Channels: %u\n", config.channels);
    std::printf("Total PCM Frames: %llu\n", totalPCMFrameCount);
    
    // Here you can process the PCM data in pSampleData if needed.
    // For now, we just free the memory and return.
    drmp3_free(pSampleData, nullptr);
}

#ifdef TEST_MP3_DECODER
// A simple main function to test the decoder independently.
// Compile with -DTEST_MP3_DECODER if you want to run this file directly.
int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <mp3 file>\n", argv[0]);
        return 1;
    }
    decodeMP3(argv[1]);
    return 0;
}
#endif
