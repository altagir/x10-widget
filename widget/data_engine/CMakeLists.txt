add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
	${CMAKE_SOURCE_DIR}
	${KDE4_INCLUDES}
)

set(x10_engine_SRCS
	container.cpp
    engine.cpp
    service.cpp
    job.cpp
)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_engine_x10 ${x10_engine_SRCS})

target_link_libraries(plasma_engine_x10
	x10_logger
	x10_dbus_interface
	${KDE4_PLASMA_LIBS}
)

install(TARGETS plasma_engine_x10
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-dataengine-x10.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES
			service.operations
			unit.operations
		DESTINATION ${DATA_INSTALL_DIR}/plasma/services )
