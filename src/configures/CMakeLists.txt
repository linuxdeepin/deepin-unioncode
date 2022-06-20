file(GLOB SUPPORTFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.support")
file(GLOB DESKTOPFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.desktop")
file(GLOB ICON "${CMAKE_CURRENT_SOURCE_DIR}/*.png")

install(FILES ${SUPPORTFILES} DESTINATION "${SOURCES_INSTALL_RPEFIX}/configures")
install(FILES ${DESKTOPFILES} DESTINATION "/usr/share/applications")
install(FILES ${ICON} DESTINATION "/usr/share/unioncode/configures/icons")
