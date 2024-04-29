set(target_name "CPSMUnitTests")
message("==== TARGET: ${target_name}")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

find_package(Threads REQUIRED)

file(GLOB src_files   "*.cpp")
file(GLOB hdr_files   "*.hpp" "*.h")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_add_executable(${target_name} WIN32)

set(ADDITIONAL_SOURCES
    TestsRc.qrc
)

target_sources("${target_name}"
    PRIVATE 
        "${ADDITIONAL_SOURCES}"
        "${src_files}"
        "${hdr_files}"
        "${proto_files}"
)
target_link_libraries("${target_name}"
    PRIVATE
    Threads::Threads
    gtest
    gtest_main
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
    CPSMTools::CPSMTools
    CPSMDBTools::CPSMDBTools
    DBTools::DBTools
    CPSMGlobals::CPSMGlobals
    libUtils::libUtils
    ${PROTOBUF_LIBRARIES}
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

add_test(NAME ${target_name} COMMAND ${target_name})

message("====> END: ${target_name}")
