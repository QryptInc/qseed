## Initialize SoftHSM

1.  Initialize a new token using softhsm2-util tool.
    ```
    softhsm2-util --init-token --slot 0 --label "My First Token"
    ```
    Sample output is shown below.
    ```
    === SO PIN (4-255 characters) ===
    Please enter SO PIN: ****
    Please reenter SO PIN: ****
    === User PIN (4-255 characters) ===
    Please enter user PIN: ****
    Please reenter user PIN: ****
    The token has been initialized and is reassigned to slot 384541823
    ```

2.  Test the module using the pkcs11-tool and the reassigned slot id from the previous step.
    ```
    pkcs11-tool --module /usr/local/lib/softhsm/libsofthsm2.so --slot 384541823 -l -t
    ```
    Sample output is shown below.
    ```
    Using slot 0 with a present token (0x16eba47f)
    Logging in to "My First Token".
    Please enter User PIN: 
    C_SeedRandom() and C_GenerateRandom():
      seems to be OK
    Digests:
      all 4 digest functions seem to work
      MD5: OK
      SHA-1: OK
    Signatures: not implemented
    Verify (currently only for RSA)
      No private key found for testing
    Unwrap: not implemented
    Decryption (currently only for RSA)
    No errors
    ```

## Build
```bash
mkdir build
cd build
cmake .. && make
make install
```

## Installation
1. Set environment variables
```
export QRYPT_TOKEN=qrypttokenfromportal
export CRYPTOKI_LIB=/usr/local/lib/softhsm/libsofthsm2.so
export CRYPTOKI_SLOT_ID=384541823
export CRYPTOKI_USER_PIN=1234
```

## Open Questions
Do we need a user pin for C_SeedRandom for Thales HSMs?
