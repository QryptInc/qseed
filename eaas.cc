#include "eaas.h"
#include <curl/curl.h>

#include <iostream>

const uint32_t MIN_REQUEST = 1;
const uint32_t MAX_REQUEST = 512;
const std::string EAAS_FQDN = "https://api-eus.qrypt.com/api/v1/quantum-entropy";

static size_t curlWriteCallback(char* data, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(data, totalSize);
    return totalSize;
}

std::string EaaS::curlRequest(const std::string& fqdn, const std::string& filename, const std::vector<std::string>& headers) {

    CURL *curl;
    std::string serverResponse;
    curl = curl_easy_init();
    if(curl) {
        
        // set standard options
        curl_easy_setopt(curl, CURLOPT_URL, fqdn.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        // define where response is saved
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &serverResponse);

        // set the multipart/form-data request
        curl_mime* mime = curl_mime_init(curl);
        curl_mimepart* part = curl_mime_addpart(mime);
        if (!filename.empty()) {
            curl_mime_name(part, "file");
            curl_mime_filedata(part, filename.c_str());
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        }
        
        // set headers
        struct curl_slist *list = NULL;
        if (headers.size() > 0) {
            for (auto& header : headers) {
                list = curl_slist_append(list, header.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        }

        // execute
        CURLcode res = curl_easy_perform(curl);

        // process response
        if (res == CURLE_OK) {
            long http_response_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_response_code);
            if (http_response_code == 200) {
                std::cout << serverResponse << std::endl;
            } else if (http_response_code != 200) {
                throw std::runtime_error("Unexpected HTTP response:\n" + serverResponse);
            }
        } else {
            throw std::runtime_error(std::string(curl_easy_strerror(res)));
        }
     
        // cleanup
        curl_easy_cleanup(curl);
        if (mime != NULL) {
            curl_mime_free(mime);
        }
        if (list != NULL) {
            curl_slist_free_all(list);
        }
    }
    else {
        throw std::runtime_error("Failed to initialize libcurl");
    }

    return serverResponse;
}

// Request
//template <typename OptValueType>
std::string EaaS::requestEntropy(uint32_t size) {

    // check size
    if (size < MIN_REQUEST || size > MAX_REQUEST) {
        throw std::runtime_error("Entropy request size is not within the acceptable range");
    }

    //fqdn
    std::string url = EAAS_FQDN + std::string("?size=") + std::to_string(size);

    //headers
    std::vector<std::string> headers;
    headers.push_back("accept: application/json");
    std::string authBearer = "Authorization: Bearer " + _token;
    headers.push_back(authBearer);

    // file to send
    std::string empty("");

    // send request
    std::string response = curlRequest(url, empty, headers);

    return response;
}