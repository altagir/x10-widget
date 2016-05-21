# CONFIG #######################################################################

# config file, writeable since overwriten by service when modified
install(FILES x10.conf
        DESTINATION /etc/x10/
        PERMISSIONS OWNER_WRITE OWNER_READ
                    GROUP_WRITE GROUP_READ
                    WORLD_READ )

install(FILES local_dbusdaemon_address.conf
        DESTINATION /etc/x10/
        PERMISSIONS OWNER_WRITE OWNER_READ
                    GROUP_WRITE GROUP_READ
                    WORLD_READ )


# X10 ##########################################################################

# rules for driver
install(FILES cm19a.rules
        DESTINATION /etc/udev/rules.d/)

# blacklisting ati_remote & others
install(FILES blacklist-x10.conf
        DESTINATION /etc/modprobe.d/)


# /VAR/SPOOL/ ##################################################################

# initial empty rules with correct permission
# install(FILES rules.conf
#         DESTINATION /var/spool/x10/
#         PERMISSIONS OWNER_WRITE OWNER_READ
#                     GROUP_WRITE GROUP_READ
#                     WORLD_WRITE WORLD_READ )
# 
# # initial empty rules with correct permission
# install(FILES data.conf
#         DESTINATION /var/spool/x10/
#         PERMISSIONS OWNER_WRITE OWNER_READ
#                     GROUP_WRITE GROUP_READ
#                     WORLD_WRITE WORLD_READ )


# DBUS #########################################################################

include(PkgConfigGetVar.cmake)
dbus_add_activation_service(ca.cyberdine.x10.service.in)

# the dbus interface file
install(FILES ca.cyberdine.x10.conf
        DESTINATION /etc/dbus-1/system.d)

# the dbus_daemon config_file (for remote access)
IF(EXISTS "/usr/sbin/apparmor_status")
   install(FILES x10_dbus_daemon-apparmor.conf
           RENAME x10_dbus_daemon.conf
           DESTINATION /etc/dbus-1/ )
   # apparmor profile, not functionnal yet for remote... cleanup IF above when done TODO
   #install(FILES usr.lib.kde4.libexec.x10_service
   #        DESTINATION /etc/apparmor.d/ )
ELSE (EXISTS "/usr/sbin/apparmor_status")
   install(FILES x10_dbus_daemon.conf
           DESTINATION /etc/dbus-1/ )
ENDIF(EXISTS "/usr/sbin/apparmor_status")

# the upstart job
install(FILES x10dbus_upstart.conf
        RENAME x10dbus.conf
        DESTINATION /etc/init/ )

# daemon script to start dbus dameon
install(FILES x10-daemon
#       RENAME x10
        DESTINATION /etc/init.d/
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE 
                    GROUP_READ  GROUP_EXECUTE 
                    WORLD_READ  WORLD_EXECUTE )

# daemon config
install(FILES etc_default_x10-daemon
	RENAME x10-daemon
	DESTINATION /etc/default/)

