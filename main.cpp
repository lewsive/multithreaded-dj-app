#include <iostream>
#include <cpr/cpr.h>

int main() {
    std::string client_id = "kgLaDplLp4y0gzyO2V3JSEv2ZQ6Wyjbz";
    std::string track_id = "123456";  // Replace with actual track ID
    std::string url = "https://api.soundcloud.com/tracks/" + track_id + "?client_id=" + client_id;

    auto response = cpr::Get(cpr::Url{url});

    if (response.status_code == 200) {
        std::cout << "Track Data: " << response.text << std::endl;
    } else {
        std::cout << "Failed to fetch track! Status Code: " << response.status_code << std::endl;
    }
    return 0;
}
