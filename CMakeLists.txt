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

# Headers only — safe to collect on every platform
file(GLOB COMMON_SOURCES
   "src/*.h"
)

# All .cpp sources are Windows-only right now (WASAPI/COM), so only
# collect and compile them when actually building on Windows.
if(WIN32)
    file(GLOB PLATFORM_SOURCES
        "src/*.cpp"
    )
else()
    set(PLATFORM_SOURCES "")
endif()

set(SOURCE_FILES ${COMMON_SOURCES} ${PLATFORM_SOURCES})

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".node"
    LINKER_LANGUAGE CXX
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_JS_INC}
    $<$<BOOL:${WIN32}>:${wil_SOURCE_DIR}/include>
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB})
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE windowsapp.lib mfuuid.lib)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
  # Generate node.lib
  execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
endif()
