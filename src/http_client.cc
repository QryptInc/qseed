#include "http_client.h"

#include <curl/curl.h>
#include <stdexcept>

CurlClient::CurlClient() {
    curl_global_init(CURL_GLOBAL_ALL);
}

CurlClient::~CurlClient() {
    curl_global_cleanup();
}

static size_t curlWriteCallback(char* data, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(data, totalSize);
    return totalSize;
}

HttpResponse CurlClient::send(const HttpRequest &curlRequest) {

    HttpResponse response = {};

    CURL *curlHandle = curl_easy_init();
    if (curlHandle == NULL) {
        throw std::runtime_error("Failed to initialize curl.");
    }

    // Set standard options
    std::string fullURL = curlRequest.fqdn + curlRequest.path;
    curl_easy_setopt(curlHandle, CURLOPT_URL, fullURL.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, curlRequest.method.c_str());
    if(!curlRequest.body.empty()) {
        curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDS, curlRequest.body.c_str());
    }
    curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, 10L);

    // Set headers
    struct curl_slist *curlSlist = NULL;
    for (const auto header : curlRequest.headers) {
        curlSlist = curl_slist_append(curlSlist, header.c_str());
    }
    curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, curlSlist);

    // Define where response is saved
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, &curlWriteCallback);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(curlHandle, CURLOPT_BUFFERSIZE, 102400L);

    // Set debugging options
    std::vector<char> errorBuffer(CURL_ERROR_SIZE, 0);
    curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, errorBuffer.data());
    // curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 1);

    // Get HTTP status code
    long statusCode = 0;
    double totalRequestTimeSec = 0; 
    CURLcode curlCode = curl_easy_perform(curlHandle);
    if (curlCode == CURLE_OK) {
        curlCode = curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &statusCode);
    }
    response.httpCode = statusCode;

    // Cleanup and throw if an internal curl error was encountered
    curl_slist_free_all(curlSlist);
    curl_easy_cleanup(curlHandle);
    if (curlCode) {
        throw std::runtime_error(
            "Curl encountered a client error:" +
            (errorBuffer[0]) ? std::string(errorBuffer.data()) :
                               std::string(curl_easy_strerror(curlCode))
        );
    }
    
    return response;

}
