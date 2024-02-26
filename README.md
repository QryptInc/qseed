# Overview
The qseed application downloads quantum entropy from Qrypt's EaaS and injects it into an HSM as seed random. The download and injection are performed periodically.

See https://docs.qrypt.com/eaas/pkcs11/ for more information.
  
# Build
This section covers how to build and install the qseed application. Note that you will need an installed cryptoki library on your system.

1.  Create and navigate to a build directory.
    ```bash
    mkdir build && cd build
    ```

2.  Configure and build the qseed application.
    ```bash
    cmake .. 
    cmake --build .
    ```

3.  Install the qseed application.
    ```bash
    cmake --install .
    ```

# Quickstart with Thales HSMs 
This section covers how to start the qseed application for Thales Network Luna 7 HSM.

1.  Follow the steps in the Build section above.

2.  Initialize a new token using ckdemo utility provided by Thales.

3.  Set runtime configurations using environment variables. The following configurations can be set using environment variables.

    | ENV | Description |
    | --- | ------------|
    | QRYPT_TOKEN | Token (with Entropy scope) retrieved from the Qrypt portal to get access to Qrypt services. |
    | QSEED_SIZE | Amount of seed random in bytes to inject into the HSM at the beginning of each time period. <br>Valid values are inclusively between 1 byte and 524,288 bytes (512 kib). Defaults to 48. |
    | QSEED_PERIOD | The time period in seconds between seed random injections. <br>Valid values are inclusively between 1 second and 31,536,000 seconds (about 1 year). Defaults to 10. |
    | CRYPTOKI_LIB | Cryptoki shared library file location. |
    | CRYPTOKI_SLOT_ID | Cryptoki slot ID as defined in the PKCS11 specification. |
    | CRYPTOKI_USER_PIN | Cryptoki user PIN as defined in the PKCS11 specification. The application will skip session login if not provided. |

4.  Run the executable. Note you may need to run the application as root to properly work with Thales cryptoki library.
    ```
    sudo qseed
    ```
    Sample output is shown below.
    ```
    [2024-02-13T17:47:26.955Z] Pushed 48 bytes of quantum seed material to the HSM.
    [2024-02-13T17:47:37.127Z] Pushed 48 bytes of quantum seed material to the HSM.
    [2024-02-13T17:47:47.303Z] Pushed 48 bytes of quantum seed material to the HSM.
    ```

# Testing
This section covers how to test the qseed application.

## Build and Run GTests
This section covers how to build and run the google tests.

```bash
mkdir build && cd build
cmake -DENABLE_TESTS=ON .. 
cmake --build .
test/qseed_tests
```

## Test using SoftHSM
This section covers how to test the application with SoftHSM.

1.  Follow the steps in the Build section above.

2.  Initialize a new token using softhsm2-util tool.
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

3.  Test the module using the pkcs11-tool and the reassigned slot id from the previous step.
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

4.  Set environment variables. The CRYPTOKI_SLOT_ID should be set to the reassigned slot id from the second step. The CRYPTOKI_USER_PIN should be set to the user pin from the second step.
    ```bash
    export QRYPT_TOKEN=qrypttokenfromportal
    export CRYPTOKI_LIB=/usr/local/lib/softhsm/libsofthsm2.so
    export CRYPTOKI_SLOT_ID=384541823
    export CRYPTOKI_USER_PIN=1234
    ```

5.  Set LD_LIBRARY_PATH so that the installed qseed application can find the SoftHSM library.
    ```bash
    export LD_LIBRARY_PATH=/usr/local/lib/softhsm:$LD_LIBRARY_PATH
    ```

6.  Run the executable.
    ```
    qseed
    ```
    Sample output is shown below.
    ```
    [2024-02-13T17:47:26.955Z] Pushed 48 bytes of quantum seed material to the HSM.
    [2024-02-13T17:47:37.127Z] Pushed 48 bytes of quantum seed material to the HSM.
    [2024-02-13T17:47:47.303Z] Pushed 48 bytes of quantum seed material to the HSM.
    ```

