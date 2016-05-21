
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
	${CMAKE_SOURCE_DIR}
	${KDE4_INCLUDES}
)

set(x10_service_SRCS 
    container.cpp
    queue-manager.cpp
    rules-manager.cpp
    x10dbusservice.cpp
    X10DbusAdaptor.cpp
)

qt4_add_dbus_interface(x10_service_SRCS ca.cyberdine.x10.xml x10_interface)

kde4_add_executable(x10_service ${x10_service_SRCS})

target_link_libraries(x10_service
	x10_core
	x10_rule
	${KDE4_KDECORE_LIBS}
)

install(TARGETS x10_service RUNTIME DESTINATION lib/kde4/libexec)
