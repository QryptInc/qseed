#include "eaas.h"
#include "hsm_adapter.h"
#include "base64.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

const uint32_t MAX_QSEED_SIZE = 512*1024;          // 512 kib
const uint32_t MAX_QSEED_PERIOD = 60*60*24*365;    // 1 year
const uint32_t DEFAULT_QSEED_SIZE = 48;
const uint32_t DEFAULT_QSEED_PERIOD = 10;

struct CommonConfig {

    std::string qryptToken;

    uint32_t qseedSize;

    uint32_t qseedPeriod;

    CommonConfig(uint32_t qseedSize, uint32_t qseedPeriod) : qseedSize(qseedSize), qseedPeriod(qseedPeriod) {}

};

CommonConfig getCommonConfig() {

    CommonConfig commonConfig(DEFAULT_QSEED_SIZE, DEFAULT_QSEED_PERIOD);

    const char* qryptToken = std::getenv("QRYPT_TOKEN");
    if (!qryptToken) {
        throw std::runtime_error("QRYPT_TOKEN environment variable is not set");
    }
    commonConfig.qryptToken = qryptToken;

    const char* qseedSizeAsStr = std::getenv("QSEED_SIZE");
    if (qseedSizeAsStr) {
        commonConfig.qseedSize = std::stoi(qseedSizeAsStr);
    }
    if (commonConfig.qseedSize == 0 || commonConfig.qseedSize > MAX_QSEED_SIZE) {
        std::string errMsg = "QSEED_SIZE must be greater than 0 and less than or equal to " + std::to_string(MAX_QSEED_SIZE) + ".";
        throw std::runtime_error(errMsg);
    }

    const char* qseedPeriodAsStr = std::getenv("QSEED_PERIOD");
    if (qseedPeriodAsStr) {
        commonConfig.qseedPeriod = std::stoi(qseedPeriodAsStr);
    }
    if (commonConfig.qseedPeriod == 0 || commonConfig.qseedPeriod > MAX_QSEED_PERIOD) {
        std::string errMsg = "QSEED_PERIOD must be greater than 0 and less than or equal to " + std::to_string(MAX_QSEED_PERIOD) + ".";
        throw std::runtime_error(errMsg);
    }

    return commonConfig;

}

CryptokiConfig getCryptokiConfig() {

    CryptokiConfig cryptokiConfig = {};

    const char* libraryFile = std::getenv("CRYPTOKI_LIB");
    if (!libraryFile) {
        throw std::runtime_error("CRYPTOKI_LIB environment variable is not set");
    }
    cryptokiConfig.libraryFile = libraryFile;

    const char* slotIDAsStr = std::getenv("CRYPTOKI_SLOT_ID");
    if (!slotIDAsStr) {
        throw std::runtime_error("CRYPTOKI_SLOT_ID environment variable is not set");
    }
    cryptokiConfig.slotID = std::stoi(slotIDAsStr);

    std::string userPIN;
    const char* userPINAsStr = std::getenv("CRYPTOKI_USER_PIN");
    if (userPINAsStr) {
        userPIN = userPINAsStr;
    }
    cryptokiConfig.pin = userPIN;

    return cryptokiConfig;

}

std::string getTimestamp() {

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch() % std::chrono::seconds(1)).count();

    std::stringstream ss;
    ss << std::put_time(gmtime(&time_t_now), "%FT%T") << '.' << std::setfill('0') << std::setw(3) << millis << 'Z';
    return ss.str();

}

int main() {

    CommonConfig commonConfig = getCommonConfig();

    // Construct EaaS client
    EaaS eaasClient(commonConfig.qryptToken);

    // Construct HSM adapter
    CryptokiConfig cryptokiConfig = getCryptokiConfig();
    CryptokiAdapter cryptokiAdapter(cryptokiConfig);

    while(1) {

        // Download quantum random
        uint32_t sizeInKibs = (commonConfig.qseedSize + 1023) / 1024;
        std::vector<uint8_t> downloadedRandom = eaasClient.requestEntropy(sizeInKibs);

        // Inject quantum random into HSM
        std::vector<uint8_t> random(downloadedRandom.begin(), downloadedRandom.begin() + commonConfig.qseedSize);
        cryptokiAdapter.injectSeedRandom(random);

        printf("[%s] Pushed %d bytes of quantum seed material to the HSM.\n", getTimestamp().c_str(), commonConfig.qseedSize);

        // Sleep until next interval
        std::this_thread::sleep_for(std::chrono::seconds(commonConfig.qseedPeriod));

    }

    return 0;
    
}