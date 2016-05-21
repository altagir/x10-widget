# icons : lighter than svg
kde4_install_icons( ${ICON_INSTALL_DIR} )

# install(FILES application-x-x10.svgz
#         DESTINATION ${ICON_INSTALL_DIR}/oxygen/scalable/mimetypes)

# TODO not used. .desktop points to above scallable icon
# install(FILES plasma-applet.png
#         DESTINATION share/kde4/apps/desktoptheme/default/x10/)

install(FILES
        light_on.png light_off.png light_on_timer.png
        arrowdown.png arrowup.png clock.png
        connect_established.png connect_creating.png connect_no.png
        DESTINATION share/x10-widget/)
