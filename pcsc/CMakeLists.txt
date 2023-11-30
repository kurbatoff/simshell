add_library(${CMAKE_PROJECT_NAME}_PCSC STATIC
	iso7816.c
	pcscwrap.c)

target_include_directories(${CMAKE_PROJECT_NAME}_PCSC PUBLIC
	${PROJECT_SOURCE_DIR}/shell
)

if(UNIX OR APPLE)
	message("PCSC: ${UNIX} ${APPLE}")
	target_link_libraries(${CMAKE_PROJECT_NAME}_PCSC pcsclite)
else(WIN32)
	message("PCSC: WIN32")
	target_link_libraries(${CMAKE_PROJECT_NAME}_PCSC winscard)
endif()
