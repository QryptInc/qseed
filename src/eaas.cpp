#include "eaas.h"
#include "base64.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

const uint32_t MIN_REQUEST = 1;
const uint32_t MAX_REQUEST = 512;
const std::string EAAS_FQDN = "https://api-eus.qrypt.com";

std::vector<std::string> parseJsonResponse(std::string jsonResponse) {

    // Parse json response
    ::rapidjson::Document restJson;
    restJson.Parse(jsonResponse.c_str());

    // Check for invalid json schema
    if (restJson.HasParseError()) {
        throw std::runtime_error(::rapidjson::GetParseError_En(restJson.GetParseError()));
    }
    if (!restJson.IsObject()) {
        throw std::runtime_error("JSON document is not an object.");
    }
    if (!restJson.HasMember("size")) {
        throw std::runtime_error("Missing size from JSON document.");
    }
    if (!restJson.HasMember("random")) {
        throw std::runtime_error("Missing random from JSON document.");
    }

    // Select base64 encoded random blocks
    std::vector<std::string> randomBlocks;
    const ::rapidjson::Value &jsonRandomBlocks = restJson["random"];
    if (!jsonRandomBlocks.IsArray()) {
        throw std::runtime_error("random in JSON document is not an array.");
    }
    for (rapidjson::SizeType i = 0; i < jsonRandomBlocks.Size(); i++) {
        randomBlocks.push_back(jsonRandomBlocks[i].GetString());
    }

    return randomBlocks;

}

std::vector<uint8_t> EaaS::requestEntropy(uint32_t size) {

    // check size
    if (size < MIN_REQUEST || size > MAX_REQUEST) {
        throw std::runtime_error("Entropy request size is not within the acceptable range");
    }

    // Set HTTP request details
    HttpRequest request = {};
    request.fqdn = EAAS_FQDN;
    request.method = "GET";
    request.path = std::string("/api/v1/quantum-entropy") + std::string("?size=") + std::to_string(size);

    // Set HTTP headers
    std::vector<std::string> headers;
    headers.push_back("accept: application/json");
    std::string authBearer = "Authorization: Bearer " + _token;
    headers.push_back(authBearer);
    request.headers = headers;

    // Perform HTTP request
    HttpResponse response = _httpClient->send(request);

    // Parse and decode response
    std::vector<uint8_t> random;
    std::vector<std::string> base64randomBlocks = parseJsonResponse(response.body);
    for (const auto &base64randomBlock : base64randomBlocks) {
        std::string randomBlockAsStr = base64_decode(base64randomBlock);
        std::vector<uint8_t> randomBlock((uint8_t *)randomBlockAsStr.c_str(), (uint8_t *)randomBlockAsStr.c_str() + randomBlockAsStr.size());
        random.insert(random.end(), randomBlock.begin(), randomBlock.end());
    }

    return random;
    
}