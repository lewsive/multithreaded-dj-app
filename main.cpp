#include <iostream>
#include <filesystem>
#include <sndfile.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>

namespace fs = std::filesystem;

std::mutex outputMutex; // Mutex to synchronize console output

// Function to detect peaks in the signal with a minimum threshold
std::vector<int> detectPeaks(const std::vector<float>& signal, float threshold, int minGap) {
    std::vector<int> peaks;
    for (size_t i = 1; i < signal.size() - 1; ++i) {
        // Check if this point is a local maximum and exceeds the threshold
        if (signal[i] > signal[i - 1] && signal[i] > signal[i + 1] && signal[i] > threshold) {
            if (peaks.empty() || (i - peaks.back()) > minGap) {
                peaks.push_back(i);
            }
        }
    }
    return peaks;
}

// Function to calculate BPM from peaks
float calculateBpm(const std::vector<int>& peaks, int sampleRate) {
    if (peaks.size() < 2) return 0.0f; // Not enough peaks to calculate BPM

    // Calculate average time difference between peaks
    float totalTimeBetweenPeaks = 0.0f;
    for (size_t i = 1; i < peaks.size(); ++i) {
        float timeBetweenPeaks = static_cast<float>(peaks[i] - peaks[i - 1]) / sampleRate;
        totalTimeBetweenPeaks += timeBetweenPeaks;
    }

    float avgTimeBetweenPeaks = totalTimeBetweenPeaks / (peaks.size() - 1);
    float bpm = 60.0f / avgTimeBetweenPeaks;  // BPM = 60 / average time between beats in seconds
    return bpm;
}

// Function to list files with .wav or .mp3 extensions in the folder
void listWavFiles(const std::string& folderPath) {
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".wav" || entry.path().extension() == ".mp3") {
            std::lock_guard<std::mutex> guard(outputMutex);
            std::cout << "Found: " << entry.path().filename() << std::endl;
        }
    }
}

// Function to test if the file can be opened and read properly
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

    // Read samples into a vector
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

// Function to detect BPM of a given file
void detectBpm(const std::string& filepath) {
    // Ensure the file exists before proceeding
    if (!fs::exists(filepath)) {
        std::lock_guard<std::mutex> guard(outputMutex);
        std::cerr << "File not found: " << filepath << std::endl;
        return;
    }

    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filepath.c_str(), SFM_READ, &sfinfo);

    if (!file) {
        std::lock_guard<std::mutex> guard(outputMutex);
        std::cerr << "Error opening file: " << filepath << std::endl;
        std::cerr << "Libsndfile error: " << sf_strerror(file) << std::endl;
        return;
    }

    std::cout << "Processing file: " << filepath << std::endl;

    // Check if frames or channels are invalid
    if (sfinfo.frames == 0 || sfinfo.channels == 0) {
        std::lock_guard<std::mutex> guard(outputMutex);
        std::cerr << "Invalid file: " << filepath << " (frames or channels is zero)" << std::endl;
        sf_close(file);
        return;
    }

    // Read the samples into a vector
    std::vector<float> samples(sfinfo.frames * sfinfo.channels);
    if (sf_readf_float(file, samples.data(), sfinfo.frames) != sfinfo.frames) {
        std::lock_guard<std::mutex> guard(outputMutex);
        std::cerr << "Error reading samples from " << filepath << std::endl;
        sf_close(file);
        return;
    }
    sf_close(file);

    // If stereo, average left and right channels to mono
    if (sfinfo.channels > 1) {
        std::vector<float> mono(samples.size() / sfinfo.channels);
        for (size_t i = 0; i < mono.size(); ++i) {
            mono[i] = (samples[i * sfinfo.channels] + samples[i * sfinfo.channels + 1]) / 2.0f;
        }
        samples = mono;
    }

    // Create an envelope by taking absolute value of the signal
    std::vector<float> envelope(samples.size());
    for (size_t i = 0; i < samples.size(); ++i) {
        envelope[i] = std::abs(samples[i]);
    }

    // Apply a simple low-pass filter to smooth the envelope
    const float smoothingFactor = 0.1f;
    for (size_t i = 1; i < envelope.size(); ++i) {
        envelope[i] = smoothingFactor * envelope[i] + (1.0f - smoothingFactor) * envelope[i - 1];
    }

    // Set a threshold for peak detection to ignore small fluctuations
    float threshold = 0.05f;  // Adjust this based on your data
    int minGap = 500;  // Minimum gap between peaks in samples, adjust for your needs

    // Detect peaks in the envelope signal
    std::vector<int> peaks = detectPeaks(envelope, threshold, minGap);

    // Calculate BPM from detected peaks
    float bpm = calculateBpm(peaks, sfinfo.samplerate) / 35;

    // Use a mutex to safely print the result from multiple threads
    std::lock_guard<std::mutex> guard(outputMutex);
    std::cout << "Detected BPM for " << filepath << ": " << bpm << std::endl;
}

int main() {
    std::string folder = "/Users/noahcoe/Music/gww2/test";  // Replace with the actual folder path
    listWavFiles(folder);

    std::vector<std::thread> threads;

    // Process files sequentially instead of using threads
    for (const auto& entry : fs::directory_iterator(folder)) {
        if (entry.path().extension() == ".wav" || entry.path().extension() == ".mp3") {
            detectBpm(entry.path().string());
        }
    }

    return 0;
}
