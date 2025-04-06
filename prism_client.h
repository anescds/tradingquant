#pragma once

#include <string>
#include <vector>
#include <utility>
#include <windows.h>
#include <wininet.h>
#include <sstream>
#include <stdexcept>

#pragma comment(lib, "wininet.lib")

class PrismClient {
private:
    std::string base_url;
    std::string api_key;
    const std::string api_version = "v1";  // API version
    const int timeout_ms = 30000;  // 30 second timeout

    std::string makeRequest(const std::string& endpoint, const std::string& data = "", bool isPost = false) {
        HINTERNET hInternet = InternetOpenA("PrismClient/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            throw std::runtime_error("Failed to initialize WinINet");
        }

        // Set timeouts
        InternetSetOption(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout_ms, sizeof(timeout_ms));
        InternetSetOption(hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout_ms, sizeof(timeout_ms));
        InternetSetOption(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout_ms, sizeof(timeout_ms));

        // Parse URL components
        URL_COMPONENTS urlComp = { 0 };
        urlComp.dwStructSize = sizeof(urlComp);
        char hostName[256] = { 0 };
        char urlPath[1024] = { 0 };
        urlComp.lpszHostName = hostName;
        urlComp.dwHostNameLength = sizeof(hostName);
        urlComp.lpszUrlPath = urlPath;
        urlComp.dwUrlPathLength = sizeof(urlPath);

        std::string fullUrl = base_url + api_version + "/" + endpoint;
        if (!InternetCrackUrlA(fullUrl.c_str(), fullUrl.length(), 0, &urlComp)) {
            InternetCloseHandle(hInternet);
            throw std::runtime_error("Failed to parse URL");
        }

        // Connect to the server
        HINTERNET hConnect = InternetConnectA(hInternet, 
            urlComp.lpszHostName,
            urlComp.nPort,
            NULL, NULL,
            INTERNET_SERVICE_HTTP,
            0, 0);

        if (!hConnect) {
            InternetCloseHandle(hInternet);
            throw std::runtime_error("Failed to connect to server");
        }

        // Create the request
        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
        if (urlComp.nScheme == INTERNET_SCHEME_HTTPS) {
            flags |= INTERNET_FLAG_SECURE;
        }

        HINTERNET hRequest = HttpOpenRequestA(hConnect,
            isPost ? "POST" : "GET",
            urlComp.lpszUrlPath,
            NULL,
            NULL,
            NULL,
            flags,
            0);

        if (!hRequest) {
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            throw std::runtime_error("Failed to create HTTP request");
        }

        // Add headers
        std::string headers = "Content-Type: application/json\r\n"
                            "Accept: application/json\r\n"
                            "X-API-Key: " + api_key + "\r\n";

        if (!HttpAddRequestHeadersA(hRequest, headers.c_str(), -1, 
            HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE)) {
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            throw std::runtime_error("Failed to add request headers");
        }

        // Send the request
        BOOL sendResult;
        if (isPost && !data.empty()) {
            sendResult = HttpSendRequestA(hRequest, NULL, 0, 
                (LPVOID)data.c_str(), data.length());
        } else {
            sendResult = HttpSendRequestA(hRequest, NULL, 0, NULL, 0);
        }

        if (!sendResult) {
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            throw std::runtime_error("Failed to send request");
        }

        // Check HTTP status code
        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        if (HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
            &statusCode, &statusCodeSize, NULL)) {
            if (statusCode != 200) {
                InternetCloseHandle(hRequest);
                InternetCloseHandle(hConnect);
                InternetCloseHandle(hInternet);
                throw std::runtime_error("HTTP Error: " + std::to_string(statusCode));
            }
        }

        // Read the response
        std::string response;
        char buffer[4096];
        DWORD bytesRead;
        while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            response.append(buffer, bytesRead);
        }

        // Clean up
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        // For testing when API is down, return mock responses
        if (response.empty()) {
            if (endpoint == "get_context") {
                return "Client is 35 years old with a budget of $100000. Started investing on 2020-01-01 and ended on 2023-12-31. Has a salary of $80000. Avoids Technology, Healthcare.";
            } else if (endpoint == "send_portfolio") {
                return "0.85";  // Mock score
            } else if (endpoint == "get_my_current_information") {
                return "{\"current_value\": 120000, \"profit\": 20000}";
            }
        }

        return response;
    }

public:
    PrismClient(const std::string& url = "https://api.prism.com/", const std::string& key = "") 
        : base_url(url), api_key(key) {
        // Ensure base_url ends with a slash
        if (!base_url.empty() && base_url.back() != '/') {
            base_url += '/';
        }
    }

    // Get context from API
    std::pair<bool, std::string> getContext() {
        try {
            std::string response = makeRequest("context");
            return {true, response};
        } catch (const std::exception& e) {
            return {false, std::string("API Error: ") + e.what()};
        }
    }

    // Send portfolio to API
    std::pair<bool, std::string> sendPortfolio(const std::vector<std::pair<std::string, double>>& portfolio) {
        try {
            std::stringstream ss;
            ss << "{\"api_key\":\"" << api_key << "\",\"portfolio\":[";
            
            bool first = true;
            for (const auto& [stock, amount] : portfolio) {
                if (!first) ss << ",";
                ss << "{\"symbol\":\"" << stock << "\",\"amount\":" << amount << "}";
                first = false;
            }
            
            ss << "]}";
            
            std::string response = makeRequest("portfolio", ss.str(), true);
            return {true, response};
        } catch (const std::exception& e) {
            return {false, std::string("API Error: ") + e.what()};
        }
    }

    // Get current information
    std::pair<bool, std::string> getMyCurrentInformation() {
        try {
            std::string response = makeRequest("information");
            return {true, response};
        } catch (const std::exception& e) {
            return {false, std::string("API Error: ") + e.what()};
        }
    }
}; 