project(test)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
	${CMAKE_SOURCE_DIR}
	${KDE4_INCLUDES}
)

kde4_add_executable(x10test main.cpp)

target_link_libraries(x10test
	x10_dbus_interface
	x10_rule
	x10_logger
	${KDE4_KDECORE_LIBS}
)