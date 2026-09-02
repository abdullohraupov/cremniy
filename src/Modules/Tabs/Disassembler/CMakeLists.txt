include(FetchContent)

set(CAPSTONE_VERSION 5.0.9)
set(CAPSTONE_GIT_URL "https://github.com/capstone-engine/capstone.git")

set(LIEF_VERSION 1.0.0)
set(LIEF_RELEASE_URL "https://github.com/lief-project/LIEF/releases/download/${LIEF_VERSION}")

FetchContent_Declare(capstone
        GIT_REPOSITORY  "${CAPSTONE_GIT_URL}"
        GIT_TAG         ${CAPSTONE_VERSION}
)

set(CAPSTONE_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(CAPSTONE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CAPSTONE_BUILD_CXTAB OFF CACHE BOOL "" FORCE)

# Disable mostly unused architectures for optimization
set(CAPSTONE_ARCHITECTURE_DEFAULT OFF CACHE BOOL "" FORCE)

set(CAPSTONE_X86_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_ARM_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_ARM64_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_RISCV_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_MIPS_SUPPORT ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(capstone)

# Installing prebuilt LIEF for specific platform (Windows, MacOS, Linux).

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LIEF_URL "${LIEF_RELEASE_URL}/LIEF-${LIEF_VERSION}-Linux-x86_64.tar.gz")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(LIEF_URL "${LIEF_RELEASE_URL}/LIEF-${LIEF_VERSION}-win64.zip")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(LIEF_URL "${LIEF_RELEASE_URL}/LIEF-${LIEF_VERSION}-Darwin-arm64.tar.gz")
    else()
        set(LIEF_URL "${LIEF_RELEASE_URL}/LIEF-${LIEF_VERSION}-Darwin-x86_64.tar.gz")
    endif()
else()
    message(FATAL_ERROR "No prebuilt LIEF for ${CMAKE_SYSTEM_NAME}")
endif()

FetchContent_Declare(LIEF
        URL ${LIEF_URL}
)

FetchContent_MakeAvailable(LIEF)

list(APPEND CMAKE_PREFIX_PATH "${lief_SOURCE_DIR}")

find_package(LIEF REQUIRED COMPONENTS STATIC)
if(TARGET LIEF::LIEF)
    set_target_properties(LIEF::LIEF PROPERTIES IMPORTED_GLOBAL TRUE)
endif()

target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/disassemblertab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/disassemblertab.h
        ${CMAKE_CURRENT_SOURCE_DIR}/disassemblersettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/disassemblersettings.h
        ${CMAKE_CURRENT_SOURCE_DIR}/disassemblersettingspage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/disassemblersettingspage.h
        ${CMAKE_CURRENT_SOURCE_DIR}/disassemblerworker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/disassemblerworker.h
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm/disasmsyntaxdelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm/disasmsyntaxdelegate.h
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm/disasmtexthighlighter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm/disasmtexthighlighter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm/backends/radare2backend.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm/backends/radare2backend.h
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm/backends/capstonebackend.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm/backends/capstonebackend.h
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm/backends/parser/lief_parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm/backends/parser/lief_parser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/instructionhelpservice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/instructionhelpservice.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm
        ${CMAKE_CURRENT_SOURCE_DIR}/disasm/backends
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        capstone
        LIEF::LIEF
)
