#include "audio_processing.h"
#include <iostream>
#include <filesystem>
#include <sndfile.h>
#include <vector>
#include <cmath>
#include <mutex>

namespace fs = std::filesystem;
std::mutex outputMutex;  // Mutex to synchronize console output

void listWavFiles(const std::string& folderPath) {
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".wav" || entry.path().extension() == ".mp3") {
            std::lock_guard<std::mutex> guard(outputMutex);
            std::cout << "Found: " << entry.path().filename() << std::endl;
        }
    }
}

std::vector<int> detectPeaks(const std::vector<float>& signal, float threshold, int minGap) {
    std::vector<int> peaks;
    for (size_t i = 1; i < signal.size() - 1; ++i) {
        if (signal[i] > signal[i - 1] && signal[i] > signal[i + 1] && signal[i] > threshold) {
            if (peaks.empty() || (i - peaks.back()) > minGap) {
                peaks.push_back(i);
            }
        }
    }
    return peaks;
}

float calculateBpm(const std::vector<int>& peaks, int sampleRate) {
    if (peaks.size() < 2) return 0.0f;

    float totalTimeBetweenPeaks = 0.0f;
    for (size_t i = 1; i < peaks.size(); ++i) {
        float timeBetweenPeaks = static_cast<float>(peaks[i] - peaks[i - 1]) / sampleRate;
        totalTimeBetweenPeaks += timeBetweenPeaks;
    }

    float avgTimeBetweenPeaks = totalTimeBetweenPeaks / (peaks.size() - 1);
    return 60.0f / avgTimeBetweenPeaks;
}

