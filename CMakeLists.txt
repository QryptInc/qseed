cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
project (qseed CXX)

find_package(RapidJSON)

# build the executable
add_executable (qseed 
    src/base64.cpp
    src/eaas.cc 
    src/http_client.cc
    src/qseed.cc
)

# include header files
target_include_directories(qseed PRIVATE "./inc")

# Set LIBRARY_PATH macro for get_random.cpp
target_compile_definitions(qseed PRIVATE LIBRARY_PATH="/usr/local/lib/softhsm/libsofthsm2.so")

# link to dependent libraries
find_library(CURL_LIBRARY NAMES curl REQUIRED HINTS /usr/lib/x86_64-linux-gnu)
find_library(SOFTHSM2_LIBRARY NAMES softhsm2 REQUIRED HINTS /usr/local/lib/softhsm)
target_link_libraries(qseed ${CMAKE_DL_LIBS} ${SOFTHSM2_LIBRARY} ${CURL_LIBRARY})

# pass the location of softhsm to use in the code
target_compile_definitions(qseed PRIVATE SOFTHSM2_LIBRARY_PATH="${SOFTHSM2_LIBRARY}")

# install to usr/local/bin
install (TARGETS qseed DESTINATION bin)