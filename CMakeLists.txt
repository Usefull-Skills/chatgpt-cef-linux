cmake_minimum_required(VERSION 3.21)
project(chatgpt_cef_v2 LANGUAGES C CXX)

if(NOT CEF_ROOT)
  if(DEFINED ENV{CEF_ROOT})
    set(CEF_ROOT "$ENV{CEF_ROOT}")
  else()
    message(FATAL_ERROR "Set CEF_ROOT to the extracted CEF binary distribution")
  endif()
endif()
list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)

add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

set(APP_SRCS
  src/main.cc
  src/app.cc
  src/app.h
  src/controller.cc
  src/controller.h
  src/client.cc
  src/client.h
)

add_executable(chatgpt-cef-v2 ${APP_SRCS})
SET_EXECUTABLE_TARGET_PROPERTIES(chatgpt-cef-v2)
target_include_directories(chatgpt-cef-v2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CEF_ROOT}")
target_link_libraries(chatgpt-cef-v2 PRIVATE libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

set(APP_OUT_DIR "${CMAKE_BINARY_DIR}/bin")
set_target_properties(chatgpt-cef-v2 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${APP_OUT_DIR}"
  BUILD_RPATH "$ORIGIN"
  INSTALL_RPATH "$ORIGIN"
)
COPY_FILES("chatgpt-cef-v2" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${APP_OUT_DIR}")
COPY_FILES("chatgpt-cef-v2" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${APP_OUT_DIR}")
SET_LINUX_SUID_PERMISSIONS("chatgpt-cef-v2" "${APP_OUT_DIR}/chrome-sandbox")
