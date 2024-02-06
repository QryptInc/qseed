cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(qseed CXX)
option(ENABLE_TESTS "Generate a test executable" OFF)

find_package(CURL REQUIRED)
find_package(RapidJSON REQUIRED)
find_library(SOFTHSM2_LIBRARY NAMES softhsm2 REQUIRED HINTS /usr/local/lib/softhsm)

add_executable (qseed 
    src/base64.cpp
    src/eaas.cc 
    src/hsm_adapter.cpp
    src/http_client.cc
    src/qseed.cc
)

target_include_directories(qseed PRIVATE 
    "./inc"
    ${CURL_INCLUDE_DIR}
    ${Rapidjson_INCLUDE_DIRS}
)

target_link_libraries(qseed 
    ${CMAKE_DL_LIBS} 
    ${SOFTHSM2_LIBRARY}
    CURL::libcurl
)

# Install to usr/local/bin
install(TARGETS qseed DESTINATION bin)

# Build tests if enabled
if(ENABLE_TESTS)
    add_subdirectory(test)
endif()
