cmake_minimum_required(VERSION 3.16)

project(mossball VERSION 2.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

option(USE_SYSTEM_LIBLCF "Use system install of liblcf" OFF)
if(USE_SYSTEM_LIBLCF)
	find_package(liblcf REQUIRED)
else(USE_SYSTEM_LIBLCF)
	function(add_liblcf)
		set(BUILD_SHARED_LIBS OFF)
		set(LIBLCF_ENABLE_INSTALL OFF)
		add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/liblcf)
	endfunction()
	add_liblcf()
endif(USE_SYSTEM_LIBLCF)

set(PROJECT_SOURCES
	src/main.cpp
    src/ui/pickerwidget.cpp
	src/ui/pickerwidget.h
	src/ui/pickerwidget.ui
	src/ui/directorydialog.cpp
	src/ui/directorydialog.h
	src/ui/directorydialog.ui
	src/ui/changelogwidget.h
	src/ui/changelogwidget.cpp
	src/ui/changelogwidget.ui
	third_party/easyrpg_editor/dbstring.h
	third_party/zip/3rdparty/miniz-3.0.2/miniz.h
	third_party/zip/include/exceptions.hpp
	third_party/zip/include/zip.hpp
	third_party/zip/src/exceptions.cpp
	third_party/zip/src/zip.cpp
	src/mossball.cpp
	src/mossball.h
	src/chgen/chgen.cpp
	src/chgen/chgen.h
	src/submission/submission.cpp
	src/submission/submission.h
	src/data/changelog.cpp
	src/data/changelog.h
	src/utils/utils.cpp
	src/utils/utils.h
)

qt_add_executable(mossball
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    LICENSE
    mossball.qrc
)

target_link_libraries(mossball PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
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

