#include "pkcs11.h"
#include "eaas.h"
#include "base64.h"

#include <dlfcn.h>
#include <iostream>

int legacy() {
    // Load the PKCS#11 library dynamically
    void* pkcs11Lib = dlopen(SOFTHSM2_LIBRARY_PATH, RTLD_NOW);
    if (!pkcs11Lib) {
        std::cerr << "Error loading PKCS#11 library: " << dlerror() << std::endl;
        return 1;
    }

    // Get a pointer to the PKCS#11 function list
    CK_FUNCTION_LIST_PTR pFunctionList;
    CK_RV (*C_GetFunctionList)(CK_FUNCTION_LIST_PTR_PTR) =
        (CK_RV(*)(CK_FUNCTION_LIST_PTR_PTR))dlsym(pkcs11Lib, "C_GetFunctionList");

    if (!C_GetFunctionList) {
        std::cerr << "Error getting function pointer: " << dlerror() << std::endl;
        dlclose(pkcs11Lib);
        return 1;
    }

    CK_RV rv = C_GetFunctionList(&pFunctionList);
    if (rv != CKR_OK) {
        std::cerr << "Error getting function list: " << rv << std::endl;
        dlclose(pkcs11Lib);
        return 1;
    }

    // Initialize the library
    rv = pFunctionList->C_Initialize(NULL);
    if (rv != CKR_OK) {
        std::cerr << "Error initializing PKCS#11 library: " << rv << std::endl;
        dlclose(pkcs11Lib);
        return 1;
    }

    // Open a session
    CK_SESSION_HANDLE hSession;
    rv = pFunctionList->C_OpenSession(0, CKF_SERIAL_SESSION, NULL, NULL, &hSession);
    if (rv != CKR_OK) {
        std::cerr << "Error opening session: " << rv << std::endl;
        dlclose(pkcs11Lib);
        return 1;
    }

    // Get random from Qrypt
    std::string token = "abcdefg";
    EaaS eaasClient("");
    //std::string response = eaasClient.requestEntropy(1);    

    // Seed random number generator
    CK_BYTE seedData[] = {0x01, 0x02, 0x03, 0x04}; // Replace with your seed data
    rv = pFunctionList->C_SeedRandom(hSession, seedData, sizeof(seedData));
    if (rv != CKR_OK) {
        std::cerr << "Error seeding random number generator: " << rv << std::endl;
        dlclose(pkcs11Lib);
        return 1;
    }

    // Finalize the library
    rv = pFunctionList->C_Finalize(NULL);
    if (rv != CKR_OK) {
        std::cerr << "Error finalizing PKCS#11 library: " << rv << std::endl;
        dlclose(pkcs11Lib);
        return 1;
    }

    // Close the library
    dlclose(pkcs11Lib);

    return 0;
}

int main() {

    const char* qryptToken = std::getenv("QRYPT_TOKEN");
    EaaS eaasClient(qryptToken);
    std::vector<uint8_t> random = eaasClient.requestEntropy(2);

    std::string base64random = base64_encode(random.data(), random.size());
    printf("%s\n", base64random.c_str());

    return 0;
    
}