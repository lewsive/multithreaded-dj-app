//#include <iostream>
//#include <curl/curl.h>
//#include <json/json.h>
//#include <fstream>
//#include <sstream>
//
//// Callback function to handle the response data
//size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
//    ((std::string*)userp)->append((char*)contents, size * nmemb);
//    return size * nmemb;
//}
//
//// Function to fetch metadata from SoundCloud API
//void fetchTrackMetadata(const std::string& trackUrl, std::string& downloadUrl) {
//    std::string apiUrl = "https://api.soundcloud.com/resolve.json?url=" + trackUrl + "&client_id=kgLaDplLp4y0gzyO2V3JSEv2ZQ6Wyjbz";
//    CURL* curl;
//    CURLcode res;
//    std::string responseData;
//
//    curl_global_init(CURL_GLOBAL_DEFAULT);
//    curl = curl_easy_init();
//
//    if (curl) {
//        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
//        res = curl_easy_perform(curl);
//
//        if (res != CURLE_OK) {
//            std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
//        } else {
//            // Parse the JSON response
//            Json::Reader reader;
//            Json::Value root;
//            if (reader.parse(responseData, root)) {
//                // Extract the stream URL (download URL)
//                downloadUrl = root["stream_url"].asString();
//                std::cout << "Fetched stream URL: " << downloadUrl << std::endl;
//            } else {
//                std::cerr << "Error parsing JSON response" << std::endl;
//            }
//        }
//        curl_easy_cleanup(curl);
//    }
//
//    curl_global_cleanup();
//}
//
//// Function to download the audio file from the URL
//void downloadAudio(const std::string& downloadUrl, const std::string& outputFilename) {
//    CURL* curl;
//    CURLcode res;
//    std::ofstream outFile(outputFilename, std::ios::binary);
//
//    // Append client_id to the stream URL
//    std::string finalDownloadUrl = downloadUrl + "?client_id=kgLaDplLp4y0gzyO2V3JSEv2ZQ6Wyjbz";
//
//    curl_global_init(CURL_GLOBAL_DEFAULT);
//    curl = curl_easy_init();
//
//    if (curl) {
//        curl_easy_setopt(curl, CURLOPT_URL, finalDownloadUrl.c_str());
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
//        res = curl_easy_perform(curl);
//
//        if (res != CURLE_OK) {
//            std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
//        } else {
//            std::cout << "Audio downloaded to " << outputFilename << std::endl;
//        }
//
//        outFile.close();
//        curl_easy_cleanup(curl);
//    }
//
//    curl_global_cleanup();
//}
//
//// Function to convert audio to MP3 (if necessary)
//void convertToMp3(const std::string& inputFile, const std::string& outputFile) {
//    std::string command = "ffmpeg -i " + inputFile + " -vn -ar 44100 -ac 2 -b:a 192k " + outputFile;
//    system(command.c_str());
//}
//
//int main() {
//    std::string soundcloudTrackUrl = "https://soundcloud.com/supernatural-bass/ill-do-anything-for-a-way-out-of-my-head";
//    std::string downloadUrl;
//
//    // Fetch metadata and get the download URL
//    fetchTrackMetadata(soundcloudTrackUrl, downloadUrl);
//
//    // Download the audio file (in the original format)
//    downloadAudio(downloadUrl, "downloaded_audio.wav");
//
//    // Convert the audio to MP3 (if necessary)
//    convertToMp3("downloaded_audio.wav", "output_audio.mp3");
//
//    return 0;
//}
