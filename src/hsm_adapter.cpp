#include "hsm_adapter.h"

#include <cstring>
#include <stdexcept>
#include <dlfcn.h>

const uint32_t TOKEN_LABEL_MAX_SIZE = 32;
const uint32_t SLOT_DESCRIPTION_MAX_SIZE = 64;

CryptokiAdapter::CryptokiAdapter(CryptokiConfig config) : _config(config) {

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

}

CryptokiAdapter::~CryptokiAdapter() {
    dlclose(_pkcs11Lib);
}

void CryptokiAdapter::injectSeedRandom(const std::vector<uint8_t>& random) {

    // Initialize the library
    CK_RV rv = _pFunctionList->C_Initialize(NULL);
    if (rv != CKR_OK) {
        std::string errMsg = "C_Initialize Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }

    // Get number of slots
    CK_ULONG slotCount;
    rv = _pFunctionList->C_GetSlotList(CK_TRUE, NULL_PTR, &slotCount);
    if (rv != CKR_OK) {
        std::string errMsg = "C_GetSlotList Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }
    printf("Number of slots with tokens: %ld\n\n", slotCount);
    if (slotCount == 0) {
        std::string errMsg = "No slots with tokens.";
        throw std::runtime_error(errMsg);        
    }

    // Get slot IDs
    CK_SLOT_ID_PTR p_slot_list = (CK_SLOT_ID_PTR) malloc(slotCount*sizeof(CK_SLOT_ID));
    rv = _pFunctionList->C_GetSlotList(CK_TRUE, p_slot_list, &slotCount);
    if (rv != CKR_OK) {
        std::string errMsg = "C_GetSlotList Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }

    // Search for existing HSM token
    for (int slotNum = 0; slotNum < slotCount; slotNum++) {
        printf("SLOT ID: 0x%lx\n", p_slot_list[slotNum]);
        CK_SLOT_INFO slot_info;
        rv = _pFunctionList->C_GetSlotInfo(p_slot_list[slotNum], &slot_info);
        std::string slotDescription((char*)slot_info.slotDescription, SLOT_DESCRIPTION_MAX_SIZE);
        printf("SLOT DESCRIPTION: %s\n", slotDescription.c_str());

        CK_TOKEN_INFO tokenInfo;
        rv = _pFunctionList->C_GetTokenInfo(p_slot_list[slotNum], &tokenInfo);
        if (rv == CKR_TOKEN_NOT_PRESENT) {
            printf("NO TOKEN\n\n");
            continue;
        }
        std::string token_label((char*)tokenInfo.label, TOKEN_LABEL_MAX_SIZE);
        printf("TOKEN LABEL: %s\n\n", token_label.c_str());
    }
    free(p_slot_list);

    // Open a session
    CK_SESSION_HANDLE hSession;
    rv = _pFunctionList->C_OpenSession(_config.slotID, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL, NULL, &hSession);
    if (rv != CKR_OK) {
        std::string errMsg = "C_OpenSession Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }

    if (!_config.pin.empty()) {
        // Login to session
        CK_UTF8CHAR_PTR pin = (uint8_t*)_config.pin.c_str();
        rv = _pFunctionList->C_Login(hSession, CKU_SO, pin, strlen((char*)pin));
        if (rv != CKR_OK) {
            std::string errMsg = "C_Login Error: " + std::to_string(rv);
            throw std::runtime_error(errMsg);
        }
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

    // Finalize the library
    rv = _pFunctionList->C_Finalize(NULL);
    if (rv != CKR_OK) {
        std::string errMsg = "C_Finalize Error: " + std::to_string(rv);
        throw std::runtime_error(errMsg);
    }

}
