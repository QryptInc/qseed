
#include "eaas.h"
#include "base64.h"

#include <gtest/gtest.h>

class EaaSTest : public ::testing::Test {
  protected:
    const char* _qryptToken = nullptr;

    void SetUp() override {
        _qryptToken = std::getenv("QRYPT_TOKEN");
        if (!_qryptToken) {
            throw std::runtime_error("QRYPT_TOKEN environment variable is not set");
        }
    }

};

TEST_F(EaaSTest, InvalidToken) {

    _qryptToken = "invalidtoken";
    EaaS eaasClient(_qryptToken);

    EXPECT_THROW(eaasClient.requestEntropy(1), std::runtime_error);

}

TEST_F(EaaSTest, InvalidRequestSize) {

    EaaS eaasClient(_qryptToken);

    EXPECT_THROW(eaasClient.requestEntropy(0), std::runtime_error);
    EXPECT_THROW(eaasClient.requestEntropy(513), std::runtime_error);

}

TEST_F(EaaSTest, 1KiBRequest) {

    EaaS eaasClient(_qryptToken);
    std::vector<uint8_t> random = eaasClient.requestEntropy(1);

    std::vector<uint8_t> zeroVector(1024, 0);
    EXPECT_EQ(random.size(), 1024);
    EXPECT_NE(random, zeroVector);

    std::string base64random = base64_encode(random.data(), random.size());
    printf("%s\n", base64random.c_str());

}

TEST_F(EaaSTest, 512KiBRequest) {

    EaaS eaasClient(_qryptToken);
    std::vector<uint8_t> random = eaasClient.requestEntropy(512);

    std::vector<uint8_t> zeroVector(512*1024, 0);
    EXPECT_EQ(random.size(), 512*1024);
    EXPECT_NE(random, zeroVector);

}
