#ifndef HSM_ADAPTER_H
#define HSM_ADAPTER_H

#include "pkcs11.h"

#include <string>
#include <vector>
#include <memory>

class HSMAdapter {
    
  public:
    HSMAdapter() = default;
    virtual ~HSMAdapter() = default;
    HSMAdapter(const HSMAdapter &) = delete;
    HSMAdapter &operator=(const HSMAdapter &) = delete;
    
    virtual void injectSeedRandom(std::vector<uint8_t> random) = 0;

};

struct CryptokiConfig {

    /// <summary>
    /// Shared object file of the Cryptoki library.
    /// </summary>
    std::string libraryFile;

    /// <summary>
    /// A logical reader that potentially contains a token.
    /// </summary>
    unsigned long slotID;

    /// <summary>
    /// Personal Identification Number.
    /// </summary>
    std::string pin;

};

/// <summary>
/// An implementation of the HSMAdapter interface using Cryptoki PKCS#11 standard
/// </summary>
class CryptokiAdapter : public HSMAdapter {

  public:
    CryptokiAdapter(CryptokiConfig config);
    virtual ~CryptokiAdapter();
    CryptokiAdapter(const CryptokiAdapter &) = delete;
    CryptokiAdapter &operator=(const CryptokiAdapter &) = delete;

    /// <summary>
    /// Calls C_SeedRandom in the Crypotoki interface
    /// </summary>
    ///
    /// <param name="random">The random to inject</param>
    void injectSeedRandom(std::vector<uint8_t> random) override;

  private:
    CryptokiConfig _config;
    void* _pkcs11Lib;
    CK_FUNCTION_LIST_PTR _pFunctionList;

};

#endif