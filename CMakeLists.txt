cmake_minimum_required(VERSION 3.1)

project(google_authenticator)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    set(HTTPLIB_IS_USING_OPENSSL TRUE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(SRC_FILES src/google_authenticator.cpp
              src/google_drive.cpp
              src/request_handler.cpp
              include/google_drive.h
              include/request_handler.h
              include/client_secret.h
              include/constants.h
              include/httplib.h
              include/mime_type.h
              include/system_utilities.h
              include/google_authenticator.h
              )

set(EXE_SOURCE app/main.cpp)

add_library(google_authenticator_lib STATIC ${SRC_FILES})
add_executable(google_authenticator ${EXE_SOURCE})

target_include_directories(google_authenticator_lib PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(google_authenticator PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(google_authenticator PUBLIC google_authenticator_lib)
target_link_libraries(google_authenticator_lib PUBLIC 
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:OpenSSL::SSL>
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:OpenSSL::Crypto>)

target_compile_definitions(google_authenticator_lib PUBLIC
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:CPPHTTPLIB_OPENSSL_SUPPORT>
        )
