cmake_minimum_required (VERSION 3.8)

set(RAW_LIB_NAME Utils)
set(LIB_NAME lib${RAW_LIB_NAME})

message("------- <${RAW_LIB_NAME}> -------")


############################
#
#       CMake config
#
############################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_COMPIL_MODE_VAR 0 CACHE INTERNAL "")
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(CMAKE_COMPIL_MODE_VAR 1 CACHE INTERNAL "")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_COMPIL_MODE_VAR 2 CACHE INTERNAL "")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  set(CMAKE_COMPIL_MODE_VAR 3 CACHE INTERNAL "")
endif()


############################
#
#       Dependencies
#
############################


###############
#   SPDLog
###############
Include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG "v1.12.0"
)
FetchContent_MakeAvailable(spdlog)

###############
#   Qt
###############
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


############################
#
#       Project
#
############################

project(${LIB_NAME})


############################
#
#       Source files
#
############################


set(INCLUDE_DIR include/${RAW_LIB_NAME})
file(GLOB src_files   "**/*.cpp")
file(GLOB hdr_files   "**/*.hpp" "**/*.h" "${INCLUDE_DIR}/**/*.hpp" "${INCLUDE_DIR}/**/*.h")

 set(SRC_DIR src)
# set(SOURCE_FILES
#   ${INCLUDE_DIR}/btype.hpp
#   ${INCLUDE_DIR}/logger_setup.hpp
#   ${INCLUDE_DIR}/logger.hpp
#   ${INCLUDE_DIR}/qstring_callback_sink.hpp

#   ${SRC_DIR}/constants.hpp
# )
set(SOURCE_FILES ${hdr_files} ${src_files}
  ${INCLUDE_DIR}/chrono.hpp
  ${INCLUDE_DIR}/list_filters.hpp
  ${INCLUDE_DIR}/AsyncFileCopyer.hpp
)


############################
#
#       Targets
#
############################

# We declare the project as a static library and add all the source code files to it.
add_library(${LIB_NAME} STATIC ${SOURCE_FILES})
#Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(${LIB_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})

target_compile_definitions(${LIB_NAME} INTERFACE CMAKE_COMPIL_MODE=${CMAKE_COMPIL_MODE_VAR})

target_include_directories(${LIB_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${LIB_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)
target_link_libraries(${LIB_NAME}
    PUBLIC
    libLogger::libLogger
)

#Set target properties
target_include_directories(${LIB_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${SRC_DIR}
)

target_compile_features(${LIB_NAME} PRIVATE cxx_auto_type)
target_compile_options(${LIB_NAME} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wpedantic>)

message("----- END <${RAW_LIB_NAME}> -----")
