set(target_name "CPSMGlobals")
message("==== TARGET: ${target_name}")

file(GLOB src_files   "*.cpp")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library("${target_name}")
add_library(${target_name}::${target_name} ALIAS "${target_name}")

set(ADDITIONAL_SOURCES
  CPSMGlobals.hpp
  CpsmGlobalConfiguration.hpp.in
)

configure_file(CpsmGlobalConfiguration.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/CpsmGlobalConfiguration.hpp)

target_sources("${target_name}"
    PRIVATE 
        "${ADDITIONAL_SOURCES}"
        "${src_files}"
        "${proto_files}"
)
target_link_libraries("${target_name}"
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    libLogger::libLogger
)

target_include_directories(${target_name}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_features(${target_name} PRIVATE cxx_auto_type)
target_compile_options(${target_name} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wpedantic>)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
#    add_compile_definitions(CMAKE_DEBUG_MODE)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

if(CPSM_BUILD_AS_MOCKUP)
    add_compile_definitions(CPSM_BUILD_AS_MOCKUP)
endif()

message("====> END: ${target_name}")
