# CMakeList.txt : CMake project for dns-cash, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("dns-cash")

# Add source to this project's executable.
add_executable (dns-cash "dns-cash.cpp" "dns-cash.h" "src/server/DNSServer.cpp" "src/server/DNSServer.h" "src/callback/DNSRequestCallback.h" "src/callback/ICallback.h"  "src/executors/ThreadPool.h" "src/executors/ThreadPool.cpp")
target_include_directories(dns-cash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(dns-cash PRIVATE ws2_32)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET dns-cash PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
