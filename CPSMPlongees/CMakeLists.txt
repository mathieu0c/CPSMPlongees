cmake_minimum_required(VERSION 3.5)



if(UNIX)
    message( FATAL_ERROR "Linux is not supported, CMake will exit." )
endif(UNIX)

#VERSION = MAJOR.MINOR.PATCH
set(VERSIONING_MAJOR 0)#incompatible update
set(VERSIONING_MINOR 0)#new features added
set(VERSIONING_PATCH 0)#bug fixed / minor change


set(EXE_NAME "CPSMPlongees")


project(CPSMPlongees VERSION ${VERSIONING_MAJOR}.${VERSIONING_MINOR}.${VERSIONING_PATCH} LANGUAGES CXX)




###############################
#                             #
#        CMake config         #
#                             #
###############################

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_ROOT}/Modules")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# --- CMake files includes ---
include(cmake/modules/Utils.cmake)


###############################
#                             #
#            LIBS             #
#                             #
###############################


if(NOT DEFINED BASE_PROTO_PATH)
    set(BASE_PROTO_PATH "C:\\msys64\\mingw64")
endif()


# --- Qt ---
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Network Sql REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network Sql REQUIRED)


# --- Sudirectories ---

add_subdirectory(Libs)




###############################
#                             #
#          Sources            #
#                             #
###############################

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/iconResource.rc")

set(PROJECT_SOURCES
        LoggerHandler.hpp

        Constants.hpp
        
        main.cpp
        MainWindow.cpp
        MainWindow.hpp
        MainWindow.ui

        Assets/ProgResources.qrc

        Gui/DiverEdit.hpp Gui/DiverEdit.cpp Gui/DiverEdit.ui
        Gui/DiveEdit.hpp Gui/DiveEdit.cpp Gui/DiveEdit.ui
        Gui/DiveSearch.hpp Gui/DiveSearch.cpp Gui/DiveSearch.ui
        Gui/DiverSearch.hpp Gui/DiverSearch.cpp Gui/DiverSearch.ui
        Gui/DialogConfirmListDeletion.hpp Gui/DialogConfirmListDeletion.cpp Gui/DialogConfirmListDeletion.ui

        Models/table_model_test.hpp Models/table_model_test.cpp
)



###############################
#                             #
#        Compilation          #
#                             #
###############################

add_compile_definitions(CMAKEMACRO_PROJECT_EXE="${CMAKE_PROJECT_NAME}")
add_compile_definitions(CMAKEMACRO_BASE_PROJECT_DIR="${CMAKE_SOURCE_DIR}")

add_compile_options(-Wall -Wextra -Wpedantic)

qt_add_executable(${EXE_NAME} WIN32
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    ${APP_ICON_RESOURCE_WINDOWS}
)

target_link_libraries(${EXE_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
    libSimpleUpdater::libSimpleUpdater
    CPSMTools::CPSMTools
    CPSMDBTools::CPSMDBTools
    DBTools::DBTools
    CPSMGlobals::CPSMGlobals
    libLogger::libLogger
)


qt_finalize_executable(${EXE_NAME})

# --- Properties ---

set_property(GLOBAL PROPERTY COMPILE_WARNING_AS_ERROR OFF)

target_include_directories(${EXE_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)


###############################
#                             #
# Code macros (definitions)   #
#                             #
###############################

add_definitions("-DPROJECT_V_MAJOR=${VERSIONING_MAJOR}")
add_definitions("-DPROJECT_V_MINOR=${VERSIONING_MINOR}")
add_definitions("-DPROJECT_V_PATCH=${VERSIONING_PATCH}")
add_definitions("-DPROJECT_GITHUB_RELEASE_DEFINE=\"https://api.github.com/repos/mathieu0c/CPSMPlongees/releases/latest\"")

add_definitions("-DPROJECT_VERSION=\"${CMAKE_PROJECT_VERSION}\"")
add_definitions("-DPROJECT_NAME=\"${CMAKE_PROJECT_NAME}\"")
add_definitions("-DWPROJECT_NAME=L\"${CMAKE_PROJECT_NAME}\"")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(CMAKE_DEBUG_MODE)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

add_definitions("-DCMAKE_PROGRAM_NAME=\"${EXE_NAME}\"")

#define LOG_HANDLER_BASE_DIR_QSTRING
IF(CMAKE_BUILD_TYPE MATCHES Debug)
    set(LOG_HANDLER_BASE_DIR_MACRO "-DLOG_HANDLER_BASE_DIR_QSTRING=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+\"/\"+QString{\"${EXE_NAME}_debug\"}" CACHE BOOL "" FORCE)
ELSE()
    set(LOG_HANDLER_BASE_DIR_MACRO "-DLOG_HANDLER_BASE_DIR_QSTRING=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+\"/\"+QString{\"${EXE_NAME}\"}" CACHE BOOL "" FORCE)
ENDIF()

add_definitions(${LOG_HANDLER_BASE_DIR_MACRO})


add_subdirectory(Tests)
