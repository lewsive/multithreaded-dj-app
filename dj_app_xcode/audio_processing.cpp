//#include "audio_processing.h"
//#include <iostream>
//#include <filesystem>
//#include <sndfile.h>
//#include <vector>
//#include <cmath>
//#include <mutex>
//
//namespace fs = std::filesystem;
//std::mutex outputMutex;  // Mutex to synchronize console output
//
//void detectBpm(const std::string& filepath) {
//    // Open the audio file using libsndfile
//    SF_INFO sfInfo;
//    SNDFILE* file = sf_open(filepath.c_str(), SFM_READ, &sfInfo);
//    if (!file) {
//        std::cerr << "Error opening file: " << filepath << std::endl;
//        return;
//    }
//
//    const int chunkSize = 1024;  // Number of frames to process at a time
//    std::vector<float> signal(chunkSize * sfInfo.channels);  // Buffer for chunk
//    
//    std::vector<float> allSignal;  // To accumulate all chunks
//    int framesRead = 0;
//
//    while ((framesRead = sf_readf_float(file, signal.data(), chunkSize)) > 0) {
//        // Process the chunk of audio signal
//        for (int i = 0; i < framesRead * sfInfo.channels; ++i) {
//            allSignal.push_back(signal[i]);  // Store the chunk in the allSignal vector
//        }
//    }
//
//    sf_close(file);
//
//    // Now process the accumulated signal
//    // Optionally, downmix to mono if more than one channel
//    if (sfInfo.channels > 1) {
//        std::vector<float> monoSignal(allSignal.size() / sfInfo.channels);
//        for (size_t i = 0; i < allSignal.size(); i += sfInfo.channels) {
//            float monoSample = 0.0f;
//            for (int ch = 0; ch < sfInfo.channels; ++ch) {
//                monoSample += allSignal[i + ch];
//            }
//            monoSignal[i / sfInfo.channels] = monoSample / sfInfo.channels;  // Averaging channels
//        }
//        allSignal = std::move(monoSignal);  // Use the mono signal for peak detection
//    }
//
//    // Detect peaks in the signal
//    float threshold = 0.05f;  // Adjust threshold based on your data
//    int minGap = 1000;  // Minimum gap between peaks in samples
//    std::vector<int> peaks = detectPeaks(allSignal, threshold, minGap);
//
//    // Calculate BPM from detected peaks
//    float bpm = calculateBpm(peaks, sfInfo.samplerate);
//    std::cout << "Detected BPM for " << filepath << ": " << bpm << " BPM" << std::endl;
//}
//
//
//
//void listWavFiles(const std::string& folderPath) {
//    for (const auto& entry : fs::directory_iterator(folderPath)) {
//        if (entry.path().extension() == ".wav" || entry.path().extension() == ".mp3") {
//            std::lock_guard<std::mutex> guard(outputMutex);
//            std::cout << "Found: " << entry.path().filename() << std::endl;
//        }
//    }
//}
//
//std::vector<int> detectPeaks(const std::vector<float>& signal, float threshold, int minGap) {
//    std::vector<int> peaks;
//    for (size_t i = 1; i < signal.size() - 1; ++i) {
//        if (signal[i] > signal[i - 1] && signal[i] > signal[i + 1] && signal[i] > threshold) {
//            if (peaks.empty() || (i - peaks.back()) > minGap) {
//                peaks.push_back(i);
//            }
//        }
//    }
//    return peaks;
//}
//
//float calculateBpm(const std::vector<int>& peaks, int sampleRate) {
//    if (peaks.size() < 2) return 0.0f;
//
//    float totalTimeBetweenPeaks = 0.0f;
//    for (size_t i = 1; i < peaks.size(); ++i) {
//        float timeBetweenPeaks = static_cast<float>(peaks[i] - peaks[i - 1]) / sampleRate;
//        totalTimeBetweenPeaks += timeBetweenPeaks;
//    }
//
//    float avgTimeBetweenPeaks = totalTimeBetweenPeaks / (peaks.size() - 1);
//    return 60.0f / avgTimeBetweenPeaks;
//}
//
