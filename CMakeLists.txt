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

# Main executable file
set(MAIN_SOURCE "dns-cash.cpp")

# Collect sources from each subdirectory
file(GLOB SERVER_SOURCES "src/server/*.cpp")
file(GLOB CALLBACK_SOURCES "src/callback/*.cpp")
file(GLOB EXECUTOR_SOURCES "src/executors/*.cpp")
file(GLOB PROCESSOR_SOURCES "src/processor/*.cpp")
file(GLOB UTILS_SOURCES "src/utils/*.cpp")

add_executable(dns-cash 
    ${MAIN_SOURCE}
    ${SERVER_SOURCES}
    ${CALLBACK_SOURCES}
    ${EXECUTOR_SOURCES}
    ${PROCESSOR_SOURCES}
    ${UTILS_SOURCES}
)
target_include_directories(dns-cash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link jemalloc using pkg-config
find_package(PkgConfig REQUIRED)
pkg_search_module(JEMALLOC REQUIRED jemalloc)

target_include_directories(dns-cash PRIVATE ${JEMALLOC_INCLUDE_DIRS})
target_link_libraries(dns-cash PRIVATE ${JEMALLOC_LIBRARIES} ws2_32)
target_link_directories(dns-cash PRIVATE ${JEMALLOC_LIBRARY_DIRS})


# Increase MSVC stack size to 16 MB
if (MSVC)
    target_compile_options(dns-cash PRIVATE
        $<$<CONFIG:Release>:
            # Core speed optimizations
            /O2                     # Maximum speed optimization 
            /Ob3                    # Most aggressive inlining (VS2019+) 
            /Oi                     # Enable intrinsic functions 
            /Ot                     # Favor fast code over small code 
            /GL                     # Whole program optimization 
            
            # Floating point optimizations
            /fp:fast                # Fast floating-point (relaxed IEEE 754) 
            /fp:except-             # Disable FP exceptions for speed 
            
            # SIMD vectorization - Choose based on your target CPU:
            /arch:AVX2              # AVX2 256-bit SIMD (Intel Haswell 2013+, AMD 2015+) 
            # /arch:AVX512          # AVX-512 512-bit SIMD (Intel Skylake-X 2017+, AMD Zen4 2022+) - UNCOMMENT for latest CPUs 
            # /arch:AVX             # Fallback for older CPUs (2011+)
            
            # Auto-parallelization
            /Qpar                   # Auto-parallelize loops across CPU cores 
            /Qvec-report:2          # Vectorization diagnostic reports 
            
            # Function & data optimizations
            /GF                     # Enable string pooling 
            /Gy                     # Enable function-level linking 
            /Gw                     # Optimize global data (package in COMDATs) 
            
            # Modern C++ optimizations
            /constexpr:depth2048    # Deeper constexpr evaluation (default 512) 
            /constexpr:steps2000000 # More constexpr steps (default 100000) 
            
            # Code cleanup
            /Zc:inline              # Remove unreferenced COMDAT functions/data 
            
            # CPU-specific tuning
            # /favor:blend            # Balanced for AMD & Intel CPUs 
            # /favor:INTEL64        # Optimize for Intel x64 CPUs 
            /favor:AMD64          # Optimize for AMD x64 CPUs 
        >
    )
    
    target_link_options(dns-cash PRIVATE
        $<$<CONFIG:Release>:
            # Link-time optimizations
            /LTCG                   # Link-time code generation (required for /GL) 
            /OPT:REF                # Eliminate unreferenced functions/data 
            /OPT:ICF                # Identical COMDAT folding (merge duplicate code) 
            
            # Performance tweaks
            /INCREMENTAL:NO         # Disable incremental linking (faster runtime) 
            /RELEASE                # Set checksum in executable 
        >
        
        # Stack size for all builds
        /STACK:16777216             # 16 MB stack
    )
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET dns-cash PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
