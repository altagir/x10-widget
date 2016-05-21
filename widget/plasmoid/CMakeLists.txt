project(plasma-x10)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${KDE4_INCLUDES}
)

add_subdirectory(icons)

set(x10-widget_SRCS
	nodes-model.cpp
	x10-widget.cpp
	x10-widget-settings.cpp
	control-widget.cpp
	control-widgetinfo.cpp
	icons/resources.cpp
)

kde4_add_ui_files(x10-widget_SRCS
	ui/config.ui
	ui/nodes.ui
	ui/rules.ui
)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_x10 ${x10-widget_SRCS})

target_link_libraries(plasma_applet_x10
	x10_logger
	x10_rule
	${KDE4_PLASMA_LIBS}
)

install(TARGETS plasma_applet_x10
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet_x10.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
