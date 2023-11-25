add_library(${CMAKE_PROJECT_NAME}_GP STATIC
	cap.c
	getstatus.c
	globalplatform.c
	keys.c
	scp02.c
	scp03.c
	scp11.c
	securechannel.c
	sw.c
)

target_include_directories(${CMAKE_PROJECT_NAME}_GP PUBLIC
	${PROJECT_SOURCE_DIR}/pcsc
	${PROJECT_SOURCE_DIR}/shell
	${PROJECT_SOURCE_DIR}/mbed
	${PROJECT_SOURCE_DIR}/unzip
	${PROJECT_SOURCE_DIR}/gsma
	${PROJECT_SOURCE_DIR}/mbedtls-3.5.1/include
	${PROJECT_SOURCE_DIR}/mbedtls-3.5.1/library
)

target_link_libraries(${CMAKE_PROJECT_NAME}_GP
	${CMAKE_PROJECT_NAME}_PCSC
	${CMAKE_PROJECT_NAME}_SHELL
	${CMAKE_PROJECT_NAME}_GSMA
	${CMAKE_PROJECT_NAME}_LUAWRAP
	${CMAKE_PROJECT_NAME}_MBED
)