find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    set(HTTPLIB_IS_USING_OPENSSL TRUE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(SRC_FILES ${CMAKE_SOURCE_DIR}/src/google_authenticator.cpp
              ${CMAKE_SOURCE_DIR}/src/google_drive.cpp
              ${CMAKE_SOURCE_DIR}/src/request_handler.cpp
	      ${CMAKE_SOURCE_DIR}/include/google_drive.h
              ${CMAKE_SOURCE_DIR}/include/request_handler.h
              ${CMAKE_SOURCE_DIR}/include/client_secret.h
              ${CMAKE_SOURCE_DIR}/include/constants.h
              ${CMAKE_SOURCE_DIR}/include/httplib.h
              ${CMAKE_SOURCE_DIR}/include/mime_type.h
              ${CMAKE_SOURCE_DIR}/include/system_utilities.h
              ${CMAKE_SOURCE_DIR}/include/google_authenticator.h
              )

add_library(google_authenticator STATIC ${SRC_FILES})
target_include_directories(google_authenticator PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(google_authenticator PUBLIC 
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:OpenSSL::SSL>
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:OpenSSL::Crypto>)

target_compile_definitions(google_authenticator PUBLIC
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:CPPHTTPLIB_OPENSSL_SUPPORT>
        )
