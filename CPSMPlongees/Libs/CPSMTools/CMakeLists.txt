set(target_name "CPSMTools")
message("==== TARGET: ${target_name}")

file(GLOB src_files   "*.cpp")
file(GLOB hdr_files   "*.hpp" "*.h")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library("${target_name}")
add_library(${target_name}::${target_name} ALIAS "${target_name}")

set(ADDITIONAL_SOURCES

)

target_sources("${target_name}"
    PRIVATE 
        "${ADDITIONAL_SOURCES}"
        "${src_files}"
        "${hdr_files}"
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

message("====> END: ${target_name}")
