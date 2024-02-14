cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(GTest REQUIRED)

add_executable(qseed_tests
    qseed_tests.cpp
    ../src/base64.cpp
    ../src/eaas.cpp
    ../src/http_client.cpp
)

target_include_directories(qseed_tests PRIVATE
    "../inc"
    ${CURL_INCLUDE_DIR}
    ${Rapidjson_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
)

target_link_libraries(qseed_tests 
    CURL::libcurl
    GTest::gtest
    GTest::gtest_main
)
