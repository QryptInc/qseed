#include "hsm_adapter.h"
#include "common.h"

#include <cstring>
#include <stdexcept>
#include <dlfcn.h>

CryptokiAdapter::CryptokiAdapter(CryptokiConfig config) : _config(config), _pkcs11Lib(NULL), _pFunctionList(NULL) {

    // Load the PKCS#11 library dynamically
    void* _pkcs11Lib = dlopen(_config.libraryFile.c_str(), RTLD_NOW);
    if (!_pkcs11Lib) {
        std::string errMsg = "Error loading PKCS#11 library: " + std::string(dlerror());
        throw std::runtime_error(errMsg);
    }

    // Get a pointer to the PKCS#11 function list
    CK_RV (*C_GetFunctionList)(CK_FUNCTION_LIST_PTR_PTR) =
        (CK_RV(*)(CK_FUNCTION_LIST_PTR_PTR))dlsym(_pkcs11Lib, "C_GetFunctionList");
    if (!C_GetFunctionList) {
        dlclose(_pkcs11Lib);
        std::string errMsg = "Error getting function pointer: " + std::string(dlerror());
        throw std::runtime_error(errMsg);
    }

    CK_RV rv = C_GetFunctionList(&_pFunctionList);
    if (rv != CKR_OK) {
        dlclose(_pkcs11Lib);
        std::string errMsg = "C_GetFunctionList Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }

    // Initialize the library
    rv = _pFunctionList->C_Initialize(NULL);
    if (rv != CKR_OK) {
        std::string errMsg = "C_Initialize Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }

}

CryptokiAdapter::~CryptokiAdapter() {

    // Finalize the library
    if (_pFunctionList != NULL) {
        CK_RV rv = _pFunctionList->C_Finalize(NULL);
        if (rv != CKR_OK) {
            std::string errMsg = "C_Finalize Error: " + std::to_string(rv);
            printf("%s\n", errMsg.c_str());
        }
    }

    dlclose(_pkcs11Lib);

}

void CryptokiAdapter::printSlotInfo() {

    // Get slot info
    CK_SLOT_INFO slotInfo;
    CK_RV rv = _pFunctionList->C_GetSlotInfo(_config.slotID, &slotInfo);
    if (rv != CKR_OK) {
        std::string errMsg = "C_GetSlotInfo Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }
    std::string slotDescription = trimWhitespace(std::string((char*)slotInfo.slotDescription, sizeof(slotInfo.slotDescription)));

    // Get token info
    CK_TOKEN_INFO tokenInfo;
    rv = _pFunctionList->C_GetTokenInfo(_config.slotID, &tokenInfo);
    if (rv == CKR_TOKEN_NOT_PRESENT) {
        printf("SLOT 0x%lx (%s): NO TOKEN\n", _config.slotID, slotDescription.c_str());
        return;
    }
    if (rv != CKR_OK) {
        std::string errMsg = "C_GetTokenInfo Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }
    std::string tokenLabel = trimWhitespace(std::string((char*)tokenInfo.label, sizeof(tokenInfo.label)));
    printf("SLOT 0x%lx (%s): %s\n", _config.slotID, slotDescription.c_str(), tokenLabel.c_str());

}

void CryptokiAdapter::injectSeedRandom(const std::vector<uint8_t>& random) {

    // Open a session
    CK_SESSION_HANDLE hSession;
    CK_RV rv = _pFunctionList->C_OpenSession(_config.slotID, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL, NULL, &hSession);
    if (rv != CKR_OK) {
        std::string errMsg = "C_OpenSession Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }

    // Login to session
    CK_UTF8CHAR_PTR pin = (uint8_t*)_config.pin.c_str();
    rv = _pFunctionList->C_Login(hSession, CKU_SO, pin, strlen((char*)pin));
    if (rv != CKR_OK) {
        std::string errMsg = "C_Login Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }

    // Seed random number generator
    rv = _pFunctionList->C_SeedRandom(hSession, (uint8_t*)random.data(), random.size());
    if (rv != CKR_OK) {
        std::string errMsg = "C_SeedRandom Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }

    // Close session
    rv = _pFunctionList->C_CloseSession(hSession);
    if (rv != CKR_OK) {
        std::string errMsg = "C_CloseSession Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }

}
