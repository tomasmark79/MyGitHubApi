#ifndef __MYGITHUBAPI_H__
#define __MYGITHUBAPI_H__

#include <ctime>
#include <string>
#include <vector>

class MyGitHubApi
{
  public:
    MyGitHubApi();
    ~MyGitHubApi();

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    void          fetchEvents(std::string &receivedData);
    int           fetchLastXCommits(int numberCommits, std::vector<std::string> &commitsV);

    void fetchCommitsLastHour(
        const std::string &username,
        const std::string &token,
        std::string       &receivedData,
        std::string       &receivedRawData
    );

  private:
    std::time_t parseISO8601(const std::string &date);

    std::string gitHubClassicToken;
    int         isTokenLoaded;
    bool        getToken(std::string &token);

  private:
};

#endif // __MYGITHUBAPI_H__
