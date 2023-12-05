#include <iostream>
#include <curl/curl.h>
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    s->append((char*)contents, newLength);
    return newLength;
}

struct Manager {
    struct Movie {
        std::string title;
        std::string genre;
        std::string information;
        std::string review;
        std::string rating;
    } userMovie;

    std::string genre;
    std::string apiKey = "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";
    std::string apiUrl = "https://api.openai.com/v1/engines/text-davinci-003/completions";

    bool sendRequest(const std::string& prompt, std::string& info) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cout << "cURL 초기화 실패!" << std::endl;
            return false;
        }

        std::string url = apiUrl;
        std::string data = "{\"prompt\": \"" + prompt + "\", \"max_tokens\": 500}";

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        std::string response_string;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            json response_json = json::parse(response_string);
            info = response_json["choices"][0]["text"];
        } else {
            std::cout << "cURL 요청 실패: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        return (res == CURLE_OK);
    }

    void printMovieInfo(const Movie& movie) {
        std::cout << "영화 제목:";
        for (char c : movie.title) {
            if (c == '\n' || c == '\r' || c == '\t') {  
                std::cout << ' ';
            } else {
                std::cout << c;
            }
        }
        std::cout << std::endl;

        std::cout << "영화 장르:  " << movie.genre << std::endl;
        std::cout << "영화 소개:";
        for (char c : movie.information) {
            if (c == '\n' || c == '\r' || c == '\t') {  
                std::cout << ' ';
            } else {
                std::cout << c;
            }
        }
        std::cout << std::endl;
        std::cout << "영화 리뷰:";
        for (char c : movie.review) {
            if (c == '\n' || c == '\r' || c == '\t') {  
                std::cout << ' ';
            } else {
                std::cout << c;
            }
        }
        std::cout << std::endl;
        std::cout << "영화 평점:";
        for (char c : movie.rating) {
            if (c == '\n' || c == '\r' || c == '\t') {  
                std::cout << ' ';
            } else {
                std::cout << c;
            }
        }
        std::cout << std::endl;
    }

    void query() {
        std::cout << "어떤 장르의 영화를 원하세요? (ex. action, romance, comedy, fantasy, thriller, musical, drama, documentary, biographical): ";
        std::cin >> genre;
        userMovie.genre = genre;
    }
};

int main() {
    Manager manager;
    manager.query();

    std::string prompt;
    if (manager.genre == "romance") {
        prompt = "Please write a brief review of the famous romance movie.";
    } else if (manager.genre == "action") {
        prompt = "Please write a brief review of the famous action movie.";
    } else if (manager.genre == "comedy") {
        prompt = "Please write a brief review of the famous comedy movie.";
    } else if (manager.genre == "fantasy") {
        prompt = "Please write a brief review of the famous fantasy movie.";
    } else if (manager.genre == "thriller") {
        prompt = "Please write a brief review of the famous thriller movie.";
    } else if (manager.genre == "musical") {
        prompt = "Please write a brief review of the famous musical movie.";
    } else if (manager.genre == "drama") {
        prompt = "Please write a brief review of the famous drama movie.";
    } else if (manager.genre == "documentary") {
        prompt = "Please write a brief review of the famous documentary movie.";
    } else if (manager.genre == "biographical") {
        prompt = "Please write a brief review of the famous biographical movie.";
    } 

    std::string titlePrompt = "Please write down the movie title without any other information";

    if (!manager.sendRequest(titlePrompt, manager.userMovie.title)) {
        std::cout << "영화 제목 요청 실패!" << std::endl;
        return 1;
    }

    auto f = [&](std::string s) {
        std::string ret = "";
        for (auto& x : s) {
            if (x >= 'a' && x <= 'z') ret += x;
            else if (x >= 'A' && x <= 'Z') ret += x;
            else if (x >= '0' && x <= '9') ret += x;
        }
        return ret;
    };

    std::string informationPrompt = "Please write a simple introduction to the " + f(manager.userMovie.title);
    std::string reviewPrompt = "Please write a simple review of the " + f(manager.userMovie.title);
    std::string ratingPrompt = "Please write the rating of the " + f(manager.userMovie.title) + " in numbers without any other information. For example, 1.7 or 4.2 or 8.0";

    if (!manager.sendRequest(informationPrompt, manager.userMovie.information)) {
        std::cout << "영화 소개 요청 실패!" << std::endl;
        return 1;
    }
    if (!manager.sendRequest(reviewPrompt, manager.userMovie.review)) {
        std::cout << "영화 리뷰 요청 실패!" << std::endl;
        return 1;
    }
    if (!manager.sendRequest(ratingPrompt, manager.userMovie.rating)) {
        std::cout << "영화 평점 요청 실패!" << std::endl;
        return 1;
    }

    std::cout << std::endl;
    std::cout << "==== 영화 정보 ====" << std::endl;
    manager.printMovieInfo(manager.userMovie);

    return 0;
}
