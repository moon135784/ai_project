#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class OpenAIAPI {
public:
    OpenAIAPI() {
        const char* apiKeyEnv = std::getenv("OPENAI_API_KEY");
        if (apiKeyEnv) {
            apiKey = apiKeyEnv;
        } else {
            std::cerr << "Error: OPENAI_API_KEY environment variable not set." << std::endl;
        }

        curl_global_init(CURL_GLOBAL_DEFAULT);

        // headers를 초기화
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
    }

    ~OpenAIAPI() {
        curl_slist_free_all(headers); // headers 해제
        curl_global_cleanup();
    }

    json generateText(const std::string& prompt, const std::string& model = "text-davinci-003",
                      int maxTokens = 50, double temperature = 0.5) const {
        std::string apiUrl = "https://api.openai.com/v1/engines/" + model + "/completions";

        json postData = {
            {"prompt", prompt},
            {"max_tokens", maxTokens},
            {"temperature", temperature}
        };

        std::string postDataStr = postData.dump();

        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postDataStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        responseData.clear();

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Error: " << curl_easy_strerror(res) << std::endl;
            return json(); // Return an empty JSON object on error
        }

        // 예외 처리 추가
        try {
            return json::parse(responseData);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            return json(); // Return an empty JSON object on parse error
        }
    }

    std::string getApiKey() const {
        return apiKey;
    }

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t totalSize = size * nmemb;
        output->append((char*)contents, totalSize);
        return totalSize;
    }

    std::string apiKey;
    CURL* curl = curl_easy_init();
    struct curl_slist* headers = nullptr;

    mutable std::string responseData;

    OpenAIAPI(const OpenAIAPI&) = delete;
    OpenAIAPI& operator=(const OpenAIAPI&) = delete;
};

int main() {
    OpenAIAPI openai;

    if (openai.getApiKey().empty()) {
        return 1; // Exit with an error code if the API key is not set
    }

    std::cout << "Welcome to Calorie Planner!\n";

    std::cout << "Enter the total calories you want to consume in a day: ";
    int totalCalories;
    std::cin >> totalCalories;

    // 사용자 입력이 정수가 아닌 경우 프로그램 종료
    if (std::cin.fail()) {
        std::cerr << "Invalid input. Please enter a valid integer." << std::endl;
        return 1;
    }

    std::cin.ignore(); // newline 문자 제거

    std::cout << "Enter the number of meals you want to plan for: ";
    int numMeals;
    std::cin >> numMeals;

    // 사용자 입력이 정수가 아닌 경우 프로그램 종료
    if (std::cin.fail()) {
        std::cerr << "Invalid input. Please enter a valid integer." << std::endl;
        return 1;
    }

    std::cin.ignore(); // newline 문자 제거

    std::cout << "Enter the number of snacks you want to plan for: ";
    int numSnacks;
    std::cin >> numSnacks;

    // 사용자 입력이 정수가 아닌 경우 프로그램 종료
    if (std::cin.fail()) {
        std::cerr << "Invalid input. Please enter a valid integer." << std::endl;
        return 1;
    }

    std::cin.ignore(); // newline 문자 제거

    // OpenAI API에 텍스트 생성 요청 보내기
    json apiResponse = openai.generateText("Plan a daily menu with " + std::to_string(totalCalories) + " calories, "
                                           "for " + std::to_string(numMeals) + " meals and " +
                                           std::to_string(numSnacks) + " snacks을 한국어로 추천해줘 그리고 음식의 양을 g으로 표시해줘",
                                           "text-davinci-003", 500);

    // API 응답에서 추천된 식단 가져오기
    std::string dailyMenu = apiResponse["choices"][0]["text"].get<std::string>();
    std::cout << "Daily Menu: " << dailyMenu << '\n';

    // API 응답에서 영양 정보 가져오기
    if (apiResponse["choices"][0]["attributes"].contains("nutrition")) {
        json nutritionInfo = apiResponse["choices"][0]["attributes"]["nutrition"];

        // 영양 정보 출력
        std::cout << "Nutrition Information (per serving):\n";
        std::cout << "Carbohydrates: " << nutritionInfo["carbohydrates"] << " grams\n";
        std::cout << "Protein: " << nutritionInfo["protein"] << " grams\n";
        std::cout << "Fat: " << nutritionInfo["fat"] << " grams\n";
    } else {
        std::cout << "Nutrition Information not available.\n";
    }

    return 0;
}
