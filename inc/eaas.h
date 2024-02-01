#ifndef EAAS_H
#define EAAS_H

#include "http_client.h"

#include <string>
#include <vector>
#include <memory>

class EaaS {
    
  public:
    EaaS(const std::string& token): _token(token), _httpClient(std::make_unique<CurlClient>()) {};
    EaaS(const EaaS &) = delete;
    EaaS &operator=(const EaaS &) = delete;
    
    std::vector<uint8_t> requestEntropy(uint32_t size = 1);

  private:
    std::string _token;
    std::unique_ptr<HttpClient> _httpClient;

};

#endif /* EAAS_H */