#include "common.h"
#include "eaas.h"
#include "hsm_adapter.h"
#include "base64.h"

#include <chrono>
#include <thread>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <ryml.hpp>
#include <ryml_std.hpp>

const std::string DEFAULT_CONFIG_FILE = "/etc/qseed/qseed_config.yml";

const uint32_t MAX_QSEED_SIZE = 64*1024;            // 64 KiB
const uint32_t MAX_QSEED_PERIOD = 60*60*24*365;     // 1 year
const uint32_t DEFAULT_QSEED_SIZE = 48;
const uint32_t DEFAULT_QSEED_PERIOD = 10;

struct CommonConfig {

    std::string qryptToken;

    uint32_t qseedSize;

    uint32_t qseedPeriod;

    CommonConfig(uint32_t qseedSize, uint32_t qseedPeriod) : qseedSize(qseedSize), qseedPeriod(qseedPeriod) {}

};

CommonConfig getCommonConfig(const std::string& yamlContents) {

    CommonConfig commonConfig(DEFAULT_QSEED_SIZE, DEFAULT_QSEED_PERIOD);
    ryml::Tree tree = ryml::parse_in_place((char*)yamlContents.c_str());

    const char* qryptTokenKey = "qrypt_token";
    if (tree[qryptTokenKey].has_key() && tree[qryptTokenKey].has_val()) {
        ryml::from_chars(tree[qryptTokenKey].val(), &commonConfig.qryptToken);
    } 
    else {
        throw std::runtime_error(std::string(qryptTokenKey) + " is not set in configuration file.");
    }

    const char* qseedSizeKey = "size";
    if (tree[qseedSizeKey].has_key() && tree[qseedSizeKey].has_val()) {
        ryml::from_chars(tree[qseedSizeKey].val(), &commonConfig.qseedSize);
    }
    if (commonConfig.qseedSize == 0 || commonConfig.qseedSize > MAX_QSEED_SIZE) {
        std::string errMsg = "Qseed size must be greater than 0 and less than or equal to " + std::to_string(MAX_QSEED_SIZE) + ".";
        throw std::runtime_error(errMsg);
    }

    const char* qseedPeriodKey = "period";
    if (tree[qseedPeriodKey].has_key() && tree[qseedPeriodKey].has_val()) {
        ryml::from_chars(tree[qseedPeriodKey].val(), &commonConfig.qseedPeriod);
    }
    if (commonConfig.qseedPeriod == 0 || commonConfig.qseedPeriod > MAX_QSEED_PERIOD) {
        std::string errMsg = "Qseed period must be greater than 0 and less than or equal to " + std::to_string(MAX_QSEED_PERIOD) + ".";
        throw std::runtime_error(errMsg);
    }

    return commonConfig;

}

CryptokiConfig getCryptokiConfig(const std::string& yamlContents) {

    CryptokiConfig cryptokiConfig = {};
    ryml::Tree tree = ryml::parse_in_place((char*)yamlContents.c_str());

    const char* libraryFileKey = "cryptoki_lib";
    if (tree[libraryFileKey].has_key() && tree[libraryFileKey].has_val()) {
        ryml::from_chars(tree[libraryFileKey].val(), &cryptokiConfig.libraryFile);
    } 
    else {
        throw std::runtime_error(std::string(libraryFileKey) + " is not set in configuration file.");
    }

    const char* slotIDKey = "cryptoki_slot_id";
    if (tree[slotIDKey].has_key() && tree[slotIDKey].has_val()) {
        ryml::from_chars(tree[slotIDKey].val(), &cryptokiConfig.slotID);
    } 
    else {
        throw std::runtime_error(std::string(slotIDKey) + " is not set in configuration file.");
    }

    const char* pinKey = "cryptoki_user_pin";
    if (tree[pinKey].has_key() && tree[pinKey].has_val()) {
        ryml::from_chars(tree[pinKey].val(), &cryptokiConfig.pin);
    } 
    else {
        throw std::runtime_error(std::string(pinKey) + " is not set in configuration file.");
    }

    return cryptokiConfig;

}

std::string readConfigFile(const std::string& configFile) {

    std::ifstream inFile(configFile.c_str(), std::ifstream::in);
    if (inFile.fail()) {
        throw std::runtime_error("Could not open config file at " + configFile + ".");
    }

    std::stringstream strStream;
    strStream << inFile.rdbuf();
    return strStream.str();

}

int main() {

    std::string configFile = DEFAULT_CONFIG_FILE;
    const char* configFileFromEnv = std::getenv("QSEED_CONFIG_FILE");
    if (configFileFromEnv) {
        configFile = configFileFromEnv;
    }

    // Parse configurations
    std::string yamlContents = readConfigFile(configFile);
    CommonConfig commonConfig = getCommonConfig(yamlContents);
    CryptokiConfig cryptokiConfig = getCryptokiConfig(yamlContents);

    // Construct EaaS client
    EaaS eaasClient(commonConfig.qryptToken);

    // Construct HSM adapter
    CryptokiAdapter cryptokiAdapter(cryptokiConfig);
    cryptokiAdapter.printSlotInfo();

    while(1) {

        // Download quantum random
        uint32_t sizeInKibs = (commonConfig.qseedSize + 1023) / 1024;
        std::vector<uint8_t> downloadedRandom = eaasClient.requestEntropy(sizeInKibs);

        // Inject quantum random into HSM
        std::vector<uint8_t> random(downloadedRandom.begin(), downloadedRandom.begin() + commonConfig.qseedSize);
        cryptokiAdapter.injectSeedRandom(random);

        infoLog("Pushed " + std::to_string(commonConfig.qseedSize) + " bytes of quantum seed material to the HSM.");

        // Sleep until next interval
        std::this_thread::sleep_for(std::chrono::seconds(commonConfig.qseedPeriod));

    }

    return 0;
    
}