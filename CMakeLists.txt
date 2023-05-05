cmake_minimum_required(VERSION 3.1)

project(google_authenticator)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    set(HTTPLIB_IS_USING_OPENSSL TRUE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(SRC_FILES src/main.cpp
              src/google_authenticator.cpp
              src/google_drive.cpp
              src/request_handler.cpp)

add_executable(google_authenticator ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:OpenSSL::SSL>
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:OpenSSL::Crypto>)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:CPPHTTPLIB_OPENSSL_SUPPORT>
        )
