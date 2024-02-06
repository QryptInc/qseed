#include "pkcs11.h"
#include "eaas.h"
#include "hsm_adapter.h"
#include "base64.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

const unsigned long DEFAULT_QSEED_SIZE = 2;
const unsigned long DEFAULT_QSEED_PERIOD = 10;

std::string getTimestamp() {

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch() % std::chrono::seconds(1)).count();

    std::stringstream ss;
    ss << std::put_time(gmtime(&time_t_now), "%FT%T") << '.' << std::setfill('0') << std::setw(3) << millis << 'Z';
    return ss.str();

}

int main() {

    // Check environment variables
    const char* qryptToken = std::getenv("QRYPT_TOKEN");
    if (!qryptToken) {
        throw std::runtime_error("QRYPT_TOKEN environment variable is not set");
    }
    unsigned long sizeInKBs = DEFAULT_QSEED_SIZE;
    auto qseedSizeAsStr = std::getenv("QSEED_SIZE");
    if (qseedSizeAsStr) {
        sizeInKBs = std::stoi(qseedSizeAsStr);
    }
    unsigned long periodInSecs = DEFAULT_QSEED_PERIOD;
    auto qseedPeriodAsStr = std::getenv("QSEED_PERIOD");
    if (qseedPeriodAsStr) {
        periodInSecs = std::stoi(qseedPeriodAsStr);
    }
    const char* libraryFile = std::getenv("CRYPTOKI_LIB");
    if (!libraryFile) {
        throw std::runtime_error("CRYPTOKI_LIB environment variable is not set");
    }
    const char* slotIDAsStr = std::getenv("CRYPTOKI_SLOT_ID");
    if (!slotIDAsStr) {
        throw std::runtime_error("CRYPTOKI_SLOT_ID environment variable is not set");
    }
    std::string userPIN;
    const char* userPINAsStr = std::getenv("CRYPTOKI_USER_PIN");
    if (userPINAsStr) {
        userPIN = userPINAsStr;
    }

    // Construct EaaS client
    EaaS eaasClient(qryptToken);

    // Construct HSM adapter
    CryptokiConfig cryptokiConfig = {};
    cryptokiConfig.libraryFile = libraryFile;
    cryptokiConfig.slotID = std::stoi(slotIDAsStr);
    cryptokiConfig.pin = userPIN;
    CryptokiAdapter cryptokiAdapter(cryptokiConfig);

    while(1) {

        // Download quantum random
        std::vector<uint8_t> random = eaasClient.requestEntropy(sizeInKBs);

        // Inject quantum random into HSM
        cryptokiAdapter.injectSeedRandom(random);

        printf("[%s] Pushed %ld KBs of quantum seed material to the HSM.\n", getTimestamp().c_str(), sizeInKBs);

        // Sleep until next interval
        std::this_thread::sleep_for(std::chrono::seconds(periodInSecs));

    }

    return 0;
    
}