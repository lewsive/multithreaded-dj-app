#include <iostream>
#include <curl/curl.h>
#include <json/json.h>
#include <fstream>
#include <sstream>
#include <cstdlib>

// Callback function to handle the response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch metadata from SoundCloud API
void fetchTrackMetadata(const std::string& trackUrl, std::string& downloadUrl, const std::string& accessToken) {
    std::string apiUrl = "https://api.soundcloud.com/resolve.json?url=" + trackUrl;
    CURL* curl;
    CURLcode res;
    std::string responseData;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        
        // Add Authorization header with the OAuth token
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Enable automatic redirect following
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            // Print raw response data for debugging
            std::cout << "Raw response: " << responseData << std::endl;

            // Parse the JSON response
            Json::Reader reader;
            Json::Value root;
            if (reader.parse(responseData, root)) {
                // Log the complete response structure
                std::cout << "Parsed JSON response:" << std::endl;
                std::cout << root.toStyledString() << std::endl;

                // Check if 'stream_url' is available and valid
                if (root.isMember("stream_url")) {
                    downloadUrl = root["stream_url"].asString();
                    if (!downloadUrl.empty()) {
                        // Append the access token if needed
                        downloadUrl += "?client_id=kgLaDplLp4y0gzyO2V3JSEv2ZQ6Wyjbz"; // Adjust with your client ID
                        std::cout << "Fetched stream URL: " << downloadUrl << std::endl;
                    } else {
                        std::cerr << "Stream URL is empty" << std::endl;
                    }
                } else {
                    std::cerr << "Stream URL not found in the response" << std::endl;
                }
            } else {
                std::cerr << "Error parsing JSON response" << std::endl;
            }
        }

        // Clean up
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();
}

int main() {
    std::string soundcloudTrackUrl = "https://soundcloud.com/supernatural-bass/ill-do-anything-for-a-way-out-of-my-head";  // Replace with actual track URL
    std::string accessToken = "2-299528--MbXrVzDFdmYx2oJ6RoMr0si"; // Replace with your actual OAuth 2.0 token
    std::string downloadUrl;

    // Fetch metadata and get the download URL
    fetchTrackMetadata(soundcloudTrackUrl, downloadUrl, accessToken);

    if (!downloadUrl.empty()) {
        std::cout << "Download URL: " << downloadUrl << std::endl;
    } else {
        std::cerr << "Failed to retrieve download URL." << std::endl;
    }

    return 0;
}
