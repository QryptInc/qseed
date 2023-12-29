#ifndef EAAS_H
#define EAAS_H

#include <string>
#include <vector>

class EaaS {
public:
    EaaS(const std::string& token): _token{token} {};
    std::string requestEntropy(uint32_t size = 1);

private:
    std::string _token;
    std::string curlRequest(const std::string& fqdn, const std::string& filename, const std::vector<std::string>& headers);
};

#endif /* EAAS_H */