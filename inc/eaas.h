#ifndef EAAS_H
#define EAAS_H

#include "http_client.h"

#include <string>
#include <vector>
#include <memory>

/// <summary>
/// Client abstraction to interact with Qrypt's Entropy as a Service (EaaS)
/// </summary>
class EaaS {
    
  public:
    EaaS(const std::string& token): _token(token), _httpClient(std::make_unique<CurlClient>()) {};
    EaaS(const EaaS &) = delete;
    EaaS &operator=(const EaaS &) = delete;
    
    /// <summary>
    /// Performs the EaaS REST call and decodes the result into a byte vector
    /// </summary>
    ///
    /// <param name="size">The amount of random to download in KBs</param>
    std::vector<uint8_t> requestEntropy(uint32_t size = 1);

  private:
    std::string _token;
    std::unique_ptr<HttpClient> _httpClient;

};

#endif