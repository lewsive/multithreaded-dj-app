#include <iostream>
#include <filesystem>
#include <sndfile.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>
#include "tinyfiledialogs.h"
#include <portaudio.h>

namespace fs = std::filesystem;

std::mutex outputMutex;

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
    float bpm = 60.0f / avgTimeBetweenPeaks;
    return bpm;
}

void listWavFiles(const std::string& folderPath) {
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".wav" || entry.path().extension() == ".mp3") {
            std::lock_guard<std::mutex> guard(outputMutex);
            std::cout << "Found: " << entry.path().filename() << std::endl;
        }
    }
}

void testFileReading(const std::string& filepath) {
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filepath.c_str(), SFM_READ, &sfinfo);

    if (!file) {
        std::lock_guard<std::mutex> guard(outputMutex);
        std::cerr << "Error opening file: " << filepath << std::endl;
        std::cerr << "Libsndfile error: " << sf_strerror(file) << std::endl;
        return;
    }

    std::cout << "File opened successfully: " << filepath << std::endl;

    std::vector<float> samples(sfinfo.frames * sfinfo.channels);
    if (sf_readf_float(file, samples.data(), sfinfo.frames) != sfinfo.frames) {
        std::lock_guard<std::mutex> guard(outputMutex);
        std::cerr << "Error reading samples from " << filepath << std::endl;
        sf_close(file);
        return;
    }

    std::cout << "Samples read successfully: " << filepath << std::endl;
    sf_close(file);
}

float detectBpm(const std::string& filepath) {
    if (!fs::exists(filepath)) {
        std::lock_guard<std::mutex> guard(outputMutex);
        std::cerr << "File not found: " << filepath << std::endl;
        return 0.0f;
    }

    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filepath.c_str(), SFM_READ, &sfinfo);

    if (!file) {
        std::lock_guard<std::mutex> guard(outputMutex);
        std::cerr << "Error opening file: " << filepath << std::endl;
        std::cerr << "Libsndfile error: " << sf_strerror(file) << std::endl;
        return 0.0f;
    }

    std::cout << "Processing file: " << filepath << std::endl;

    if (sfinfo.frames == 0 || sfinfo.channels == 0) {
        std::lock_guard<std::mutex> guard(outputMutex);
        std::cerr << "Invalid file: " << filepath << " (frames or channels is zero)" << std::endl;
        sf_close(file);
        return 0.0f;
    }

    std::vector<float> samples(sfinfo.frames * sfinfo.channels);
    if (sf_readf_float(file, samples.data(), sfinfo.frames) != sfinfo.frames) {
        std::lock_guard<std::mutex> guard(outputMutex);
        std::cerr << "Error reading samples from " << filepath << std::endl;
        sf_close(file);
        return 0.0f;
    }
    sf_close(file);

    if (sfinfo.channels > 1) {
        std::vector<float> mono(samples.size() / sfinfo.channels);
        for (size_t i = 0; i < mono.size(); ++i) {
            mono[i] = (samples[i * sfinfo.channels] + samples[i * sfinfo.channels + 1]) / 2.0f;
        }
        samples = mono;
    }

    std::vector<float> envelope(samples.size());
    for (size_t i = 0; i < samples.size(); ++i) {
        envelope[i] = std::abs(samples[i]);
    }

    const float smoothingFactor = 0.1f;
    for (size_t i = 1; i < envelope.size(); ++i) {
        envelope[i] = smoothingFactor * envelope[i] + (1.0f - smoothingFactor) * envelope[i - 1];
    }

    float threshold = 0.05f;
    int minGap = 500;

    std::vector<int> peaks = detectPeaks(envelope, threshold, minGap);

    float bpm = calculateBpm(peaks, sfinfo.samplerate) / 35;

    std::lock_guard<std::mutex> guard(outputMutex);

    return bpm;
}

void playSong(const std::string& filepath) {
    std::lock_guard<std::mutex> guard(outputMutex);
    std::cout << "Playing: " << filepath << std::endl;
}

int main() {
    const char* folder = tinyfd_selectFolderDialog("Select Folder", "");

    if (folder == nullptr) {
        std::cerr << "No folder selected." << std::endl;
        return 1;
    }

    std::string folderPath = folder;

    std::vector<std::pair<std::string, float>> bpmResults;

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".wav" || entry.path().extension() == ".mp3") {
            float bpm = detectBpm(entry.path().string());
            if (bpm > 0.0f) {
                bpmResults.push_back({entry.path().string(), bpm});
            }
        }
    }

    std::sort(bpmResults.begin(), bpmResults.end(), [](const std::pair<std::string, float>& a, const std::pair<std::string, float>& b) {
        return a.second < b.second;
    });

    for (size_t i = 0; i < bpmResults.size(); ++i) {
        if (i + 1 < bpmResults.size()) {
            std::cout << "Next song: " << bpmResults[i + 1].first << " - BPM: " << bpmResults[i + 1].second << std::endl;
        }
        playSong(bpmResults[i].first);
    }

    return 0;
}
