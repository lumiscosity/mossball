cmake_minimum_required(VERSION 3.16)

project(mossball VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

option(USE_SYSTEM_LIBLCF "Use system install of liblcf" OFF)
if(USE_SYSTEM_LIBLCF)
	find_package(liblcf REQUIRED)
else(USE_SYSTEM_LIBLCF)
	function(add_liblcf)
		if(WIN32)
			set(BUILD_SHARED_LIBS OFF)
		endif(WIN32)
		set(LIBLCF_ENABLE_INSTALL OFF)
		add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/liblcf)
	endfunction()
	add_liblcf()
endif(USE_SYSTEM_LIBLCF)

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/bit7z)

set(PROJECT_SOURCES
        src/main.cpp
        src/lcfops.h
        src/pickerwidget.cpp
        src/pickerwidget.h
        src/pickerwidget.ui
        src/directorydialog.cpp
        src/directorydialog.h
        src/directorydialog.ui
        src/changelogwidget.h
        src/changelogwidget.cpp
        src/changelogwidget.ui
        third_party/easyrpg_editor/dbstring.h
)

qt_add_executable(mossball
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    LICENSE
    mossball.qrc
)

target_link_libraries(mossball PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(mossball PRIVATE bit7z)
target_link_libraries(mossball PRIVATE lcf)


if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.mossball)
endif()
set_target_properties(mossball PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS mossball
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(mossball)

