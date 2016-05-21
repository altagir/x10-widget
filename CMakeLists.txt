project(x10-widget)
cmake_minimum_required(VERSION 2.8)
SET(CMAKE_VERBOSE_MAKEFILE OFF)

# Version #######################################
set(MAJOR_VERSION "0")
set(MINOR_VERSION "6")
set(PATCH_VERSION "5")
set(FULL_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})

#Fix cmake failing when both qt4 and qt5-base are installed
# qmake refers to the 5.x version, so force cmake to use Qt4
FIND_PROGRAM(QTMAKE4_CMD qmake-qt4)
IF(QTMAKE4_CMD)
SET(QT_QMAKE_EXECUTABLE  "${QTMAKE4_CMD}")
ENDIF(QTMAKE4_CMD)

# CMAKE_INSTALL_PREFIX ##########################
# INSTALL default TO /usr to be loaded by kbuildsycoca4
SET(CMAKE_INSTALL_PREFIX /usr)
# here we try actually to replace default with actual KDE4 config
FIND_PROGRAM(KDE4CONFIG_CMD kde4-config)
IF(KDE4CONFIG_CMD)
  EXECUTE_PROCESS(COMMAND "${KDE4CONFIG_CMD}" --prefix
    OUTPUT_VARIABLE CMAKE_INSTALL_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF(KDE4CONFIG_CMD)
message ("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")


# x10config.h.in ################################
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/common/x10config.h.in"
  "${PROJECT_BINARY_DIR}/common/x10config.h"
  )
# for above common/x10config.h
include_directories("${PROJECT_BINARY_DIR}")

# Find the required Libaries
find_package(KDE4 REQUIRED)
INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)
find_package(KDE4Workspace REQUIRED)

#remove -Wcast-align warning from compiler
#they give annoying warning using std::map, list... on ARM
string(REPLACE "-Wcast-align " "" CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}")
string(REPLACE "-Wcast-align " "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

########### subdirectories ######################

add_subdirectory(common)
add_subdirectory(controller)
add_subdirectory(widget)
add_subdirectory(cmd)

add_subdirectory(doc)
add_subdirectory(install)

add_subdirectory(test)

#################################################
##
##    Deb Package Generator
##
#################################################

IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  MESSAGE(STATUS "CMakeRoot: ${CMAKE_ROOT}/Modules/CPack.cmake")
  INCLUDE(InstallRequiredSystemLibraries)

  SET(CPACK_SET_DESTDIR "on")
  SET(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
  SET(CPACK_GENERATOR "DEB;TGZ")
#  SET(CPACK_GENERATOR "DEB;RPM;STGZ;TBZ2;TGZ;TZ;ZIP;SOURCE_TGZ;SOURCE_TZ;SOURCE_ZIP;")
  SET(CPACK_SOURCE_IGNORE_FILES
      "/build/"
      "/release/"
      "/.svn/"
      ".kdev4$"
      "~$"
      )

  SET(CPACK_DEBIAN_PACKAGE_NAME         ${CMAKE_PROJECT_NAME})
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "X10 Control Widget")
  SET(CPACK_PACKAGE_VENDOR              "Sebastien Senechal")
  SET(CPACK_PACKAGE_CONTACT             "Sebastien Senechal <altagir@gmail.com>")
 
  SET(CPACK_RESOURCE_FILE_LICENSE   ${CMAKE_CURRENT_SOURCE_DIR}/debian/COPYRIGHT)
 
  SET(CPACK_PACKAGE_VERSION_MAJOR	${MAJOR_VERSION})
  SET(CPACK_PACKAGE_VERSION_MINOR	${MINOR_VERSION})	# to create a debian confirm file-version-string
  STRING(REGEX REPLACE "_" "-" CPACK_PACKAGE_VERSION_PATCH ${PATCH_VERSION})

  SET(CPACK_PACKAGE_FILE_NAME
    ${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}_${CMAKE_SYSTEM_PROCESSOR})
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME
    ${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}_source)

  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA 
    "${CMAKE_SOURCE_DIR}/debian/preinst;${CMAKE_SOURCE_DIR}/debian/postinst;${CMAKE_SOURCE_DIR}/debian/prerm;${CMAKE_SOURCE_DIR}/debian/postrm;${CMAKE_SOURCE_DIR}/debian/config;${CMAKE_SOURCE_DIR}/debian/templates;${CMAKE_SOURCE_DIR}/debian/conffiles")

  SET(CPACK_STRIP_FILES                 TRUE)
  #SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)

  ##	DEBIAN SPECIFIC SETTINGS
  ##
  SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION  "A X10 Control widget to interface with a X10CM19A usb Controller\n")
  SET(CPACK_DEBIAN_PACKAGE_PRIORITY     "optional")
  SET(CPACK_DEBIAN_PACKAGE_SECTION      "kde")

  SET(CPACK_DEBIAN_PACKAGE_DEPENDS      "dbus, debconf (>= 0.2.26)")
  SET(DEBIAN_PACKAGE_BUILDS_DEPENDS     "kdelibs5-dev, kdebase-workspace-dev, libusb-dev")
  
  FIND_PROGRAM(DPKG_CMD dpkg)
  IF(DPKG_CMD)
    EXECUTE_PROCESS(COMMAND "${DPKG_CMD}" --print-architecture
      OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  ENDIF(DPKG_CMD)

  INCLUDE(CPack)
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")


## On debian systems we install or add the following files to the package
## /usr/share/doc/plasma-applet-x10
##   copyright
##   README.gz
##   changelog.Debian.gz
##   TODO
IF(EXISTS "/etc/debian_version")
  MESSAGE(STATUS "Debian System detected...")
  IF(NOT EXISTS "/bin/gzip")
    MESSAGE(FATAL_ERROR "Program gzip is missing on your system.")
  ENDIF(NOT EXISTS "/bin/gzip")

  ADD_CUSTOM_TARGET(debian_package
                    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/README.gz
                            ${CMAKE_CURRENT_BINARY_DIR}/changelog.Debian.gz)

  ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/README.gz
                     COMMAND /bin/rm ARGS -f ${CMAKE_CURRENT_BINARY_DIR}/README.gz
                     COMMAND /bin/cp ARGS ${CMAKE_CURRENT_SOURCE_DIR}/debian/README ${CMAKE_CURRENT_BINARY_DIR}/README
                     COMMAND /bin/gzip ARGS -9 ${CMAKE_CURRENT_BINARY_DIR}/README
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/debian/README)

  ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/changelog.Debian.gz
                     COMMAND /bin/rm ARGS -f ${CMAKE_CURRENT_BINARY_DIR}/changelog.Debian.gz
                     COMMAND /bin/cp ARGS ${CMAKE_CURRENT_SOURCE_DIR}/debian/changelog ${CMAKE_CURRENT_BINARY_DIR}/changelog.Debian
                     COMMAND /bin/gzip ARGS -9 ${CMAKE_CURRENT_BINARY_DIR}/changelog.Debian
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/debian/changelog)

  INSTALL(FILES         ${CMAKE_CURRENT_SOURCE_DIR}/debian/COPYRIGHT
          DESTINATION   /usr/share/doc/${CMAKE_PROJECT_NAME}
          RENAME        copyright
          COMPONENT     ApplicationData
         )
  INSTALL(FILES         ${CMAKE_CURRENT_BINARY_DIR}/README.gz
                        ${CMAKE_CURRENT_BINARY_DIR}/changelog.Debian.gz
                        debian/TODO
          DESTINATION   /usr/share/doc/${CMAKE_PROJECT_NAME}
          COMPONENT     ApplicationData
         )

  ## this might be dirty, but the target debian_package needs to be called in order to create some files for a debian package
  ## and this was the only idea i had...
  ## if anybody has a better idea, how to generate and add the files in /usr/share/doc/plasma-widget-x10/
  ## to the debian package, please let me know
  ADD_DEPENDENCIES(plasma_applet_x10 debian_package)
ENDIF(EXISTS "/etc/debian_version")
