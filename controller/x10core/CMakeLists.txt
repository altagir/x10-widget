
include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
)

set(folder_SOURCES
	libusbutils.cpp
	x10codes.cpp
	x10controller.cpp
	x10plugin-driver.cpp
	x10plugin-libusb.cpp
)

kde4_add_library(x10_core STATIC ${folder_SOURCES})

########### install files ###############

target_link_libraries(
	x10_core 
	x10_logger
	usb
)

# install(TARGETS x10_core DESTINATION lib)

# set_target_properties(x10_core PROPERTIES VERSION ${FULL_VERSION})
