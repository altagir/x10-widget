
include_directories(
	${CMAKE_SOURCE_DIR}
	${KDE4_INCLUDES}
)

# LOGGER Library
kde4_add_library(x10_logger         STATIC logger.cpp)

# Config Library (/etc/x10/x10.conf)
# kde4_add_library(x10_config       STATIC config.cpp)

# RULE Library
kde4_add_library(x10_rule           STATIC rule.cpp)

# X10 Dbus Interface Library
kde4_add_library(x10_dbus_interface STATIC 
    X10DbusInterface.cpp
    interfacehandler.cpp
)

IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  SET_TARGET_PROPERTIES(x10_logger PROPERTIES COMPILE_FLAGS "-fPIC")
#   SET_TARGET_PROPERTIES(x10_config PROPERTIES COMPILE_FLAGS "-fPIC")
  SET_TARGET_PROPERTIES(x10_rule   PROPERTIES COMPILE_FLAGS "-fPIC")
  SET_TARGET_PROPERTIES(x10_dbus_interface PROPERTIES COMPILE_FLAGS "-fPIC")
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

