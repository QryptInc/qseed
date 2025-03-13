#include "eaas.h"
#include "base64.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <vector>
#include <string>
#include <sstream>
#include <numeric>

const uint32_t EAAS_MAX_REQUEST = 512;
const std::string EAAS_FQDN = "https://api-eus.qrypt.com";

std::vector<uint8_t> parseAndFlattenEntropy(const std::string& jsonResponse) {
    rapidjson::Document doc;
    doc.Parse(jsonResponse.c_str());
    if (!doc.IsObject() || !doc.HasMember("entropy") || !doc["entropy"].IsArray()) {
        throw std::runtime_error("Invalid JSON response format");
    }

    std::ostringstream concatenatedEntropy;
    for (const auto& item: doc["entropy"].GetArray()) {
        if (!item.IsString()) {
            throw std::runtime_error("Invalid entropy block");
            }
            concatenatedEntropy << base64_decode(item.GetString());
        }

    std::string str = concatenatedEntropy.str();

    // Convert the string to a vector<uint8_t>
    std::vector<uint8_t> result(str.begin(), str.end());
    
    return result;
}

std::vector<uint8_t> EaaS::requestEntropy(uint32_t size) {

    // Check size
    if (size == 0 || size > EAAS_MAX_REQUEST) {
        throw std::runtime_error("Entropy request size is not within the acceptable range");
    }

    // Set HTTP request details
    HttpRequest request = {};
    request.fqdn = EAAS_FQDN;
    request.method = "POST";
    request.path = std::string("/api/v1/entropy");

    // Set HTTP headers
    std::vector<std::string> headers;
    headers.push_back("accept: application/json");
    std::string authBearer = "Authorization: Bearer " + _token;
    headers.push_back(authBearer);
    request.headers = headers;

    // Calculate the number of blocks needed
    // The max block size is 512 bytes.
    uint64_t num_packets_u64 = (size / 512) + std::min(static_cast<unsigned int>(1), size % 512);
    size_t block_count = static_cast<size_t>(num_packets_u64);

    size_t block_size = 1;

    // if we need more than one block, use max size
    if (block_count > 1){
        block_size = 512;
    }

    // Write body
    rapidjson::StringBuffer jsonBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(jsonBuffer);
    writer.StartObject();
    writer.Key("block_size");
    writer.Int(block_size);
    writer.Key("block_count");
    writer.Int(block_count);
    writer.EndObject();
    request.body = jsonBuffer.GetString();

    // Perform HTTP request
    HttpResponse response = _httpClient->send(request);
    if (response.httpCode != 200) {
        std::string errMsg = "Entropy request returned status code " + std::to_string(response.httpCode) + ". " + response.body;
        throw std::runtime_error(errMsg);
    }

    // Parse and decode response
    std::vector<uint8_t> random = parseAndFlattenEntropy(response.body);

    return random;  
}
