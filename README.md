# Overview
The qseed application downloads quantum entropy from Qrypt's EaaS and injects it into an HSM as seed random. The download and injection are performed periodically.

See [Seed PKCS#11 HSMs](https://docs.qrypt.com/eaas/pkcs11/) for more information.
  
## Table of Contents
1. [Quickstart using Thales HSMs](#quickstart-using-thales-hsms)
2. [Build](#build)
3. [Test](#test)
    1. [Build and Run GTests](#build-and-run-gtests)
    2. [Test using SoftHSM](#test-using-softhsm)

# Quickstart using Thales HSMs 
This section covers how to start the qseed application for Thales Network Luna 7 HSM.

1.  Install the Thales Client software that includes the cryptoki library on the client machine. 

    Follow the instructions provided by Thales. Note by default the Thales installation script will install the software in /usr/safenet/lunaclient.

2.  Establish a connection between the client and the Thales Network Luna 7 HSM. 

    Follow the instructions provided by Thales. This can be accomplished using the [lunacm](https://thalesdocs.com/gphsm/luna/7/docs/network/Content/lunacm/commands/commands.htm) application located at /usr/safenet/lunaclient/bin/lunacm. Run the `clientconfig deploy` command.

    ```bash
    sudo /usr/safenet/lunaclient/bin/lunacm
    lunacm:>clientconfig deploy -server <IP or Hostname of Luna appliance> –user <appliance username> -password <appliance password> -client <client name to create>  -partition <partition name>
    ```

3.  Create a token on the Thales Network Luna 7 HSM.

    Follow the instructions provided by Thales. When you create a token, you will be prompted to select a slot number and set a partition SO role PIN. The selected slot number will be needed for the qseed application configuration. The partition SO role PIN will be needed to create a crypto user role PIN.

4.  Create a crypto user role PIN on the Thales Network Luna 7 HSM.

    Follow the instructions provided by Thales. The crypto user role PIN will be needed for the qseed application configuration.
    
5.  Build and install the qseed application. Follow the steps in the [Build](#build) section below.

6.  Set runtime configurations using the yaml configuration file. The Qseed application will expect the configuration file at /etc/qseed/qseed_config.yml by default. You can override the default location by using the QSEED_CONFIG_FILE environment variable. The following configurations can be set in the yaml configuration file.

    | Configuration | Description |
    | ------------- | ------------|
    | qrypt_token | Token (with Entropy scope) retrieved from the Qrypt portal to get access to Qrypt services. |
    | size | Amount of seed random in bytes to inject into the HSM at the beginning of each time period. <br>Valid values are inclusively between 1 byte and 65,536 (64 KiB). Defaults to 48. |
    | period | The time period in seconds between seed random injections. <br>Valid values are inclusively between 1 second and 31,536,000 seconds (about 1 year). Defaults to 10. |
    | cryptoki_lib | Cryptoki shared library file location. |
    | cryptoki_slot_id | Cryptoki slot ID as defined in the PKCS11 specification. |
    | cryptoki_user_pin | Cryptoki crypto user role PIN as defined in the PKCS11 specification. |

    ```yaml
    qrypt_token: qrypttokenfromportal
    size: 48      # In bytes
    period: 10    # In seconds
    cryptoki_lib: /usr/safenet/lunaclient/lib/libCryptoki2_64.so
    cryptoki_slot_id: 0
    cryptoki_user_pin: 1234
    ```

7.  Run the executable. Note you need to run the application as root if root privileges are required for the Thales cryptoki library.
    ```
    sudo -E /usr/local/bin/qseed
    ```
    Sample output is shown below.
    ```
    [2024-02-29T15:02:40.280Z] SLOT #0 (Net Token Slot): token-new
    [2024-02-29T15:02:40.842Z] Pushed 48 bytes of quantum seed material to the HSM.
    [2024-02-29T15:02:51.349Z] Pushed 48 bytes of quantum seed material to the HSM.
    [2024-02-29T15:03:01.904Z] Pushed 48 bytes of quantum seed material to the HSM.
    ```

# Build
This section covers how to build and install the qseed application. 

1.  Install build tools on your system. See the dockerfiles under .devcontainer as a reference. The example below is for Ubuntu 22.04.
    ```bash
    apt-get -y install build-essential cmake
    ```    

2.  Install libcurl and rapidjson on your system. See the dockerfiles under .devcontainer as a reference. The example below is for Ubuntu 22.04.
    ```bash
    apt-get -y install libcurl4-openssl-dev rapidjson-dev
    ``` 

3.  Install rapidyaml on your system. See the dockerfiles under .devcontainer as a reference.
    ```bash
    git clone --recursive https://github.com/biojppm/rapidyaml
    cd rapidyaml
    mkdir build && cd build
    cmake ..
    cmake --build .
    cmake --install .
    cd ../..
    ``` 

4.  Create and navigate to a build directory.
    ```bash
    mkdir build && cd build
    ```

5.  Configure and build the qseed application.
    ```bash
    cmake .. 
    cmake --build .
    ```

6.  Install the qseed application.
    ```bash
    sudo cmake --install .
    ```

# Test
This section covers how to test the qseed application.

## Build and Run GTests
This section covers how to build and run the google tests.

1.  Install gtest on your system. See the dockerfiles under .devcontainer as a reference. The example below is for Ubuntu 22.04.
    ```bash
    apt-get -y install libgtest-dev
    ``` 

2.  Create and navigate to a build directory. Configure and build the qseed test application.
    ```bash
    mkdir build && cd 
    cmake -DENABLE_TESTS=ON .. 
    cmake --build .
    ```

3.  Run the qseed test application.
    ```bash
    export QRYPT_TOKEN=qrypttokenfromportal
    test/qseed_tests
    ```

## Test using SoftHSM
This section covers how to test the application with SoftHSM.

1.  Install SoftHSM on your system. See the dockerfiles under .devcontainer as a reference. The example below is for Ubuntu 22.04.
    ```bash
    apt-get -y install libssl-dev
    wget https://dist.opendnssec.org/source/softhsm-2.6.1.tar.gz
    wget https://dist.opendnssec.org/source/softhsm-2.6.1.tar.gz.sha256
    sha256sum --check softhsm-2.6.1.tar.gz.sha256
    tar -xzvf softhsm-2.6.1.tar.gz && rm softhsm-2.6.1.tar.gz
    cd softhsm-2.6.1
    ./configure
    make
    make install
    ``` 

2.  Follow the steps in the Build section above.

3.  Initialize a new token using softhsm2-util tool.
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
    The token has been initialized and is reassigned to slot 824959035
    ```

4.  Create or update the configuration file at /etc/qseed/qseed_config.yml. The cryptoki_slot_id should be set to the reassigned slot id from the second step. The cryptoki_user_pin should be set to the user PIN from the second step.
    ```yaml
    qrypt_token: qrypttokenfromportal
    cryptoki_lib: /usr/local/lib/softhsm/libsofthsm2.so
    cryptoki_slot_id: 824959035
    cryptoki_user_pin: 1234
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
    [2024-02-29T15:49:11.050Z] SLOT #824959035 (SoftHSM slot ID 0x312be03b): My First Token
    [2024-02-29T15:49:11.316Z] Pushed 48 bytes of quantum seed material to the HSM.
    [2024-02-29T15:49:21.515Z] Pushed 48 bytes of quantum seed material to the HSM.
    [2024-02-29T15:49:31.702Z] Pushed 48 bytes of quantum seed material to the HSM.
    ```

