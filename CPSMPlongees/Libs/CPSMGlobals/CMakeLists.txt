set(target_name "CPSMGlobals")
message("==== TARGET: ${target_name}")

if(CPSM_BUILD_AS_MOCKUP)
    message(STATUS "Build as MOCKUP")
    add_compile_definitions(CPSM_BUILD_AS_MOCKUP)
else()
    message(STATUS "NOT Build as MOCKUP")
endif()

###############################
#                             #
#      Retrieve variables     #
#                             #
###############################

set(MAIN_PROJECT_VERSIONING_MAJOR $ENV{MAIN_VERSIONING_MAJOR})
set(MAIN_PROJECT_VERSIONING_MINOR $ENV{MAIN_VERSIONING_MINOR})
set(MAIN_PROJECT_VERSIONING_PATCH $ENV{MAIN_VERSIONING_PATCH})

set(PROJECT_GITHUB_RELEASE_DEFINE $ENV{MAIN_GITHUB_URL})

###############################
#                             #
#   END Retrieve variables    #
#                             #
###############################


message(STATUS "Main project versioning: ${MAIN_PROJECT_VERSIONING_MAJOR}.${MAIN_PROJECT_VERSIONING_MINOR}.${MAIN_PROJECT_VERSIONING_PATCH}")

file(GLOB src_files   "*.cpp")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library("${target_name}")
add_library(${target_name}::${target_name} ALIAS "${target_name}")

set(ADDITIONAL_SOURCES
  MainSoftVersion.hpp
  CPSMGlobals.hpp
  CpsmGlobalConfiguration.hpp.in
  CpsmGlobalConfiguration.hpp
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
    libSimpleUpdater::libSimpleUpdater
)
target_link_libraries("${target_name}"
    PUBLIC
    libSimpleUpdater::libSimpleUpdater
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
