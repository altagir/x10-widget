if(WIN32)
   set(MAN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/man/man1)
else()
   set(MAN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/man/man1)
endif()

install (
  FILES x10.1
  DESTINATION ${MAN_INSTALL_DIR}
  COMPONENT doc
)
