cmake_minimum_required(VERSION 3.15...3.31)
project(loopback_capture_addon)
add_compile_definitions(-DNAPI_VERSION=4)

if(WIN32)
    include(FetchContent)
    FetchContent_Declare(
        wil
        GIT_REPOSITORY https://github.com/microsoft/wil.git
        GIT_TAG        master
        EXCLUDE_FROM_ALL
        SYSTEM
    )
    set(WIL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(WIL_BUILD_PACKAGING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(wil)
endif()

# Collect common source files (headers and non-platform-specific cpp)
file(GLOB COMMON_SOURCES
   "src/*.h"
)

# Collect platform-independent cpp files (if you have any)
# You might want to explicitly list these instead of using GLOB
file(GLOB COMMON_CPP
   "src/*.cpp"
)

# Platform-specific sources
if(WIN32)
    file(GLOB PLATFORM_SOURCES
        "src/*_win.cpp"
    )
    # Exclude Linux files from the build
    list(FILTER COMMON_CPP EXCLUDE REGEX ".*_linux\\.cpp$")
elseif(UNIX)
    find_package(OpenSSL REQUIRED)
    file(GLOB PLATFORM_SOURCES
        "src/*_linux.cpp"
    )
    # Exclude Windows files from the build
    list(FILTER COMMON_CPP EXCLUDE REGEX ".*_win\\.cpp$")
endif()

# Also exclude platform-specific files from COMMON_CPP
list(FILTER COMMON_CPP EXCLUDE REGEX ".*_win\\.cpp$")
list(FILTER COMMON_CPP EXCLUDE REGEX ".*_linux\\.cpp$")

# Combine all sources
set(SOURCE_FILES ${COMMON_SOURCES} ${COMMON_CPP} ${PLATFORM_SOURCES})



add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_JS_INC}
    $<$<BOOL:${WIN32}>:${wil_SOURCE_DIR}/include>
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB})
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE windowsapp.lib mfuuid.lib)
endif()

if(UNIX)
   target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} OpenSSL::SSL OpenSSL::Crypto)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
  # Generate node.lib
  execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
endif()
