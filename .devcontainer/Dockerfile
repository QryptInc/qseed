FROM --platform=linux/amd64 ubuntu:20.04

# Update before installing anything
RUN apt-get update

# Install gcc, make, etc
RUN apt-get -y install build-essential

# Install essentials
RUN apt-get -y install curl git vim

# Install wget
RUN apt-get -y install wget

# Install PKCS11 Tool
RUN apt-get -y install opensc

# Unzip + install CMake
WORKDIR /azp/tools/
RUN wget https://cmake.org/files/v3.24/cmake-3.24.1-linux-x86_64.tar.gz
RUN tar -xzvf cmake-3.24.1-linux-x86_64.tar.gz && rm -f cmake-3.24.1-linux-x86_64.tar.gz
ENV PATH "$PATH:/azp/tools/cmake-3.24.1-linux-x86_64/bin"
RUN cmake --version

# Install libcurl, openssl
RUN apt-get -y install libcurl4-openssl-dev
RUN apt-get -y install libssl-dev

# Configure + build + install SoftHSM
WORKDIR /workspaces
RUN wget https://dist.opendnssec.org/source/softhsm-2.6.1.tar.gz
RUN wget https://dist.opendnssec.org/source/softhsm-2.6.1.tar.gz.sha256
RUN sha256sum --check softhsm-2.6.1.tar.gz.sha256
RUN tar -xzvf softhsm-2.6.1.tar.gz && rm softhsm-2.6.1.tar.gz
WORKDIR /workspaces/softhsm-2.6.1
RUN ./configure
RUN make
RUN make install

# Build + install metering client
