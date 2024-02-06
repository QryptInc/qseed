# Overview
The qseed application downloads quantum entropy from Qrypt's EaaS and injects it into an HSM as seed random.

## Build
This section covers how to build and install the qseed application.

```bash
mkdir build && cd build
cmake .. 
make
make install
```

## Build and Run GTests
This section covers how to run the google tests.

```bash
mkdir build && cd build
cmake -DENABLE_TESTS=ON .. 
make
test/qseed_tests
```

## Test using SoftHSM
This section covers how to test the application with SoftHSM. The steps covered in the Build section above need to be completed first.

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

3.  Set environment variables. The CRYPTOKI_SLOT_ID should be set to the reassigned slot id from the first step. The CRYPTOKI_USER_PIN should be set to the user pin from the first step.
    ```bash
    export QRYPT_TOKEN=qrypttokenfromportal
    export CRYPTOKI_LIB=/usr/local/lib/softhsm/libsofthsm2.so
    export CRYPTOKI_SLOT_ID=384541823
    export CRYPTOKI_USER_PIN=1234
    ```

4.  Set LD_LIBRARY_PATH so that the installed qseed application can find the SoftHSM library
    ```bash
    export LD_LIBRARY_PATH=/usr/local/lib/softhsm:$LD_LIBRARY_PATH
    ```

5.  Run the executable
    ```
    qseed
    ```
    Sample output is shown below.
    ```
    [2024-02-06T22:46:42.973Z] Pushed 2 KBs of quantum seed material to the HSM.
    [2024-02-06T22:46:53.150Z] Pushed 2 KBs of quantum seed material to the HSM.
    [2024-02-06T22:47:03.314Z] Pushed 2 KBs of quantum seed material to the HSM.
    ```

## Setup with Thales Luna HSM
TODO

## Open Questions
Do we need a user pin for C_SeedRandom for Thales HSMs?
