# Build the module
cmake_minimum_required(VERSION 2.8)

# TODO USE uname -a !!! CMAKE_SYSTEM VER NEED RECONFIGURE
exec_program(uname ARGS "-r" OUTPUT_VARIABLE UNAMER)

message( ${UNAMER})
message( ${CMAKE_SYSTEM_VERSION})

Set( KERNEL_DIR 		"/lib/modules/${UNAMER}/build" )
Set( MODULE_INSTALL_DIR "/lib/modules/${UNAMER}/kernel/drivers/usb/comm" )

Set( kbuild_cmd ${CMAKE_MAKE_PROGRAM}
                -C ${KERNEL_DIR}
                M=${CMAKE_CURRENT_SOURCE_DIR} modules)

Set( kclean_cmd ${CMAKE_MAKE_PROGRAM}
                -C ${KERNEL_DIR}
                M=${CMAKE_CURRENT_SOURCE_DIR} clean )

set ( kofile ${CMAKE_CURRENT_SOURCE_DIR}/x10cm19a.ko )

set ( ksrc
		x10-cm19a.c
		Kbuild )

set ( ktmp
		${CMAKE_CURRENT_SOURCE_DIR}/x10cm19.ao
		${CMAKE_CURRENT_SOURCE_DIR}/.x10-cm19a.o.cmd
		${CMAKE_CURRENT_SOURCE_DIR}/x10-cm19a.mod.c
		${CMAKE_CURRENT_SOURCE_DIR}/x10-cm19a.mod.o
		${CMAKE_CURRENT_SOURCE_DIR}/.x10-cm19a.mod.o.cmd
		${CMAKE_CURRENT_SOURCE_DIR}/.x10-cm19a.ko.cmd
		${CMAKE_CURRENT_SOURCE_DIR}/.tmp_versions
		${CMAKE_CURRENT_SOURCE_DIR}/Module.symvers
		${CMAKE_CURRENT_SOURCE_DIR}/modules.order )

add_custom_command ( OUTPUT ${kofile} ${ktmp}
						COMMAND ${kbuild_cmd}
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
						DEPENDS ${ksrc}
						COMMENT "Building kernel module..."
						VERBATIM )

add_custom_target ( x10_cm19a ALL DEPENDS ${kofile} )


# Installation of the module
install( FILES x10_cm19a.ko
         DESTINATION ${MODULE_INSTALL_DIR}
         PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
