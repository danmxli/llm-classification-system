#include "llm_socket.h"
#include <algorithm>
#include <cctype>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using json = nlohmann::json;

static std::string trim(std::string s)
{
    auto notspace = [](int ch)
    { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notspace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notspace).base(), s.end());
    return s;
}
static std::vector<std::string> splitCSV(const std::string &csv)
{
    std::vector<std::string> out;
    std::stringstream ss(csv);
    std::string item;
    while (std::getline(ss, item, ','))
        out.push_back(trim(item));
    out.erase(std::remove_if(out.begin(), out.end(),
                             [](const std::string &s)
                             { return s.empty(); }),
              out.end());
    return out;
}
static size_t curlWrite(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}
static std::string escapeJson(const std::string &in)
{
    std::string out;
    out.reserve(in.size() + 8);
    for (char c : in)
    {
        switch (c)
        {
        case '\"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out += c;
            break;
        }
    }
    return out;
}

static std::string postJson(const std::string &url, const std::string &payload,
                            long timeout_ms = 1500, int retries = 1)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl init failed");
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string response;
    long code = 0;

    auto cleanup = [&]()
    {
        if (headers)
            curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    };

    for (int attempt = 0; attempt <= retries; ++attempt)
    {
        response.clear();
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_ms);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
            if (code == 200)
            {
                cleanup();
                return response;
            }
        }
        // backoff
        if (attempt < retries)
        {
#ifdef _WIN32
            Sleep(50);
#else
            struct timespec ts = {0, 50 * 1000 * 1000};
            nanosleep(&ts, nullptr);
#endif
        }
    }
    cleanup();
    throw std::runtime_error("classifier service unavailable");
}

std::string labelText(const std::string &textToClassify,
                      const std::string &candidateLabelsCSV)
{
    auto labels = splitCSV(candidateLabelsCSV);
    if (labels.empty())
        return "";

    // build JSON
    std::ostringstream oss;
    oss << R"({"text":")" << escapeJson(textToClassify) << R"(","labels":[)";
    for (size_t i = 0; i < labels.size(); ++i)
    {
        if (i)
            oss << ",";
        oss << "\"" << escapeJson(labels[i]) << "\"";
    }
    oss << "]}";
    const std::string payload = oss.str();

    // default to localhost
    const char *env = std::getenv("CLASSIFIER_ENDPOINT");
    const std::string url = env ? std::string(env) : "http://127.0.0.1:8088/classify";

    std::string body;
    try
    {
        body = postJson(url, payload, /*timeout_ms*/ 1500, /*retries*/ 1);
    }
    catch (...)
    {
        // fallback to first label if service is down
        return labels.front();
    }

    try
    {
        auto j = json::parse(body);
        if (j.contains("best_label") && j["best_label"].is_string())
        {
            return j["best_label"].get<std::string>();
        }
    }
    catch (...)
    {
    }

    return labels.front();
}
