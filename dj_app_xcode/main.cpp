#include <iostream>
#include <filesystem>
#include <thread>
#include <vector>
#include "audio_processing.h"  // Include the header file

namespace fs = std::filesystem;

int main() {
    std::string folder = "/Users/noahcoe/Music/gww2/test";  // Replace with your actual folder path
    listWavFiles(folder);

    // Process files sequentially
    for (const auto& entry : fs::directory_iterator(folder)) {
        if (entry.path().extension() == ".wav" || entry.path().extension() == ".mp3") {
            detectBpm(entry.path().string());
        }
    }

    return 0;
}

