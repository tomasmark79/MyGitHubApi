#include "MyGitHubApi/MyGitHubApi.hpp"

// #include <EmojiTools/EmojiTools.hpp> // yet dissabled
#include <iostream>
#include <mygithubapi/version.h>

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

#define GITHUB_OAUTH_TOKEN_FILE (std::string) "/home/tomas/.github_oauth.key"
#define GITHUB_USERNAME         (std::string) "tomasmark79"

#define DEBUG 1

MyGitHubApi::MyGitHubApi()
{
    std::cout << "--- MyGitHubApi v." << MYGITHUBAPI_VERSION << " instantiated ---" << std::endl;

    isTokenLoaded = getToken(gitHubClassicToken);
    if (!isTokenLoaded)
    {
        std::cerr << "Error: Could not load GitHub token" << std::endl;
    }
}

MyGitHubApi::~MyGitHubApi()
{
    std::cout << "--- MyGitHubApi uninstantiated ---" << std::endl;
}

// Funkce pro ukládání odpovědi do řetězce
size_t MyGitHubApi::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// Převod ISO8601 na std::time_t
std::time_t MyGitHubApi::parseISO8601(const std::string &date)
{
    struct std::tm     tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return std::mktime(&tm);
}

void MyGitHubApi::fetchEvents(std::string &receivedData)
{
    std::cout << "Fetching events ..." << std::endl;

    CURL       *curl;
    CURLcode    res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl)
    {
        // Nastavení URL
        std::string url = "https://api.github.com/users/" + GITHUB_USERNAME + "/events";

        // Přidání autentizace
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(
            headers, ("Authorization: Bearer " + this->gitHubClassicToken).c_str()
        );
        headers = curl_slist_append(headers, "User-Agent: C++ App");

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Provést požadavek
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else
        {
            receivedData = readBuffer;
            if (DEBUG)
                std::cout << "Received data: " << receivedData << std::endl;
        }

        // Uvolnění zdrojů
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

int MyGitHubApi::fetchLastXCommits(int numberCommits, std::vector<std::string> &commitsV)
{
    std::cout << "Fetching last " << numberCommits << " commits ..." << std::endl;

    int totalCommits = 0;

    std::string receivedData;
    this->fetchEvents(receivedData);

    try
    {
        json response = json::parse(receivedData);
        if (!response.is_array())
        {
            std::cerr << "Expected JSON array but got something else." << std::endl;
            return -1;
        }

        for (const auto &event : response)
        {
            if (event["type"] == "PushEvent")
            {
                std::string repoName = event["repo"]["name"];
                std::string message = event["payload"]["commits"][0]["message"];
                std::string commitDate = event["created_at"];
                std::time_t commitTime = parseISO8601(commitDate);

                std::stringstream ss;
                ss << "Repo: " << repoName << std::endl;
                ss << "Message: " << message << std::endl;
                ss << "Commit time: " << commitDate << std::endl;

                commitsV.insert(commitsV.begin(), ss.str());

                if (DEBUG)
                    std::cout << commitsV.back() << std::endl;

                totalCommits++;
                if (totalCommits >= numberCommits)
                {
                    break;
                }
            }
        }
    }
    catch (json::parse_error &e)
    {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return -1;
    }
    catch (json::type_error &e)
    {
        std::cerr << "JSON type error: " << e.what() << std::endl;
        return -1;
    }

    return totalCommits;
}

// Hlavní metoda pro získání seznamu commitů
void MyGitHubApi::fetchCommitsLastHour(
    const std::string &username,
    const std::string &token,
    std::string       &receivedData,
    std::string       &receivedRawData
)
{
    // CURL       *curl;
    // CURLcode    res;
    // std::string readBuffer;

    // curl = curl_easy_init();
    // if (curl)
    // {
    //     // Nastavení URL
    //     std::string url = "https://api.github.com/users/" + username + "/events";

    //     // Přidání autentizace
    //     struct curl_slist *headers = nullptr;
    //     headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    //     headers = curl_slist_append(headers, "User-Agent: C++ App");

    //     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    //     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    //     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    //     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    //     // Provést požadavek
    //     res = curl_easy_perform(curl);
    //     if (res != CURLE_OK)
    //     {
    //         std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    //     }
    //     else
    //     {
    //         try
    //         {
    //             // Print the raw response
    //             std::cout << "Raw response: " << readBuffer << std::endl;

    //             receivedRawData = readBuffer;

    //             // Parsování JSON odpovědi
    //             json response = json::parse(readBuffer);

    //             // Check if response is an array
    //             if (!response.is_array())
    //             {
    //                 std::cerr << "Expected JSON array but got something else." << std::endl;
    //                 return;
    //             }

    //             // Získání času před hodinou
    //             std::time_t now = std::time(nullptr);
    //             std::time_t oneHourAgo = now - 3600;

    //             for (const auto &event : response)
    //             {
    //                 if (event["type"] == "PushEvent")
    //                 {
    //                     // Process the event
    //                     std::string repoName = event["repo"]["name"];
    //                     std::string message = event["payload"]["commits"][0]["message"];
    //                     std::string commitDate = event["created_at"];
    //                     std::time_t commitTime = parseISO8601(commitDate);

    //                     std::cout << "Repo: " << repoName << std::endl;
    //                     std::cout << "Message: " << message << std::endl;
    //                     std::cout << "Commit time: " << commitDate << std::endl;
    //                     std::cout << "Commit time (parsed): "
    //                               << std::asctime(std::gmtime(&commitTime));
    //                     std::cout << "Commit time (local): "
    //                               << std::asctime(std::localtime(&commitTime));

    //                     receivedData = "Repo: " + repoName + "\n";
    //                     receivedData += "Message: " + message + "\n";
    //                     receivedData += "Commit time: " + commitDate + "\n";
    //                     receivedData += "Commit time (parsed): " +
    //                         std::string(std::asctime(std::gmtime(&commitTime)));
    //                     receivedData += "Commit time (local): " +
    //                         std::string(std::asctime(std::localtime(&commitTime)));
    //                 }
    //             }
    //         }
    //         catch (json::parse_error &e)
    //         {
    //             std::cerr << "JSON parse error: " << e.what() << std::endl;
    //         }
    //         catch (json::type_error &e)
    //         {
    //             std::cerr << "JSON type error: " << e.what() << std::endl;
    //         }
    //     }
    //     // Uvolnění zdrojů
    //     curl_slist_free_all(headers);
    //     curl_easy_cleanup(curl);
    // }
}

bool MyGitHubApi::getToken(std::string &token)
{
    std::ifstream file(GITHUB_OAUTH_TOKEN_FILE);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << GITHUB_OAUTH_TOKEN_FILE << std::endl;
        return false;
    }

    std::getline(file, token);

    file.close();
    if (token.empty())
    {
        std::cerr << "Error: Token is empty" << std::endl;
        return false;
    }

    return true;
}