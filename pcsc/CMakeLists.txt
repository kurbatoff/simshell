add_library(${CMAKE_PROJECT_NAME}_PCSC STATIC
	iso7816.c
	pcscwrap.c
	serial.c)

target_include_directories(${CMAKE_PROJECT_NAME}_PCSC PUBLIC
	${PROJECT_SOURCE_DIR}/shell
)

if(UNIX OR APPLE)
	target_link_libraries(${CMAKE_PROJECT_NAME}_PCSC pcsclite)
elseif(WIN32)
	target_link_libraries(${CMAKE_PROJECT_NAME}_PCSC winscard)
endif()
