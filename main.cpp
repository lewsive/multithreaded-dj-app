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
#include <chrono>
#include <condition_variable>
#include <queue>

namespace fs = std::filesystem;

std::mutex outputMutex;
std::mutex bpmMutex;
std::condition_variable cv;
bool done = false;

struct Task {
    std::string filepath;
    float bpm = 0.0f;
};

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

float detectBpm(const std::string& filepath) {
    if (!fs::exists(filepath) || (filepath.substr(filepath.find_last_of(".")) != ".wav" && filepath.substr(filepath.find_last_of(".")) != ".mp3")) {
        return 0.0f;
    }

    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filepath.c_str(), SFM_READ, &sfinfo);

    if (!file) {
        return 0.0f;
    }

    std::vector<float> samples(sfinfo.frames * sfinfo.channels);
    if (sf_readf_float(file, samples.data(), sfinfo.frames) != sfinfo.frames) {
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

    return calculateBpm(peaks, sfinfo.samplerate) / 35;
}

void playSong(const std::string& filepath) {
    std::lock_guard<std::mutex> guard(outputMutex);
    std::cout << "Playing: " << filepath << std::endl;
}

void workerThread(std::queue<std::string>& taskQueue, std::vector<Task>& results) {
    while (true) {
        std::string filepath;
        {
            std::lock_guard<std::mutex> guard(bpmMutex);
            if (taskQueue.empty()) {
                break;
            }
            filepath = taskQueue.front();
            taskQueue.pop();
        }

        float bpm = detectBpm(filepath);
        if (bpm > 0.0f) {
            std::lock_guard<std::mutex> guard(bpmMutex);
            results.push_back({filepath, bpm});
        }
    }
}

int main() {
    const char* folder = tinyfd_selectFolderDialog("Select Folder", "");
    if (folder == nullptr) {
        std::cerr << "No folder selected." << std::endl;
        return 1;
    }
    std::string folderPath = folder;
    auto start = std::chrono::high_resolution_clock::now();

    std::queue<std::string> taskQueue;
    std::vector<Task> bpmResults;
    int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        std::string extension = entry.path().extension().string();
        if (extension == ".wav" || extension == ".mp3") {
            taskQueue.push(entry.path().string());
        }
    }

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(workerThread, std::ref(taskQueue), std::ref(bpmResults));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::sort(bpmResults.begin(), bpmResults.end(), [](const Task& a, const Task& b) {
        return a.bpm < b.bpm;
    });

    for (size_t i = 0; i < bpmResults.size(); ++i) {
        if (i + 1 < bpmResults.size()) {
            std::cout << "Next song: " << bpmResults[i + 1].filepath << " - BPM: " << bpmResults[i + 1].bpm << std::endl;
        }
        playSong(bpmResults[i].filepath);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Program runtime: " << duration.count() << " seconds" << std::endl;

    return 0;
}
