add_library(${CMAKE_PROJECT_NAME}_MBED STATIC
	hal_aes.c
	hal_des.c
	mbedwrap.c
)

target_include_directories(${CMAKE_PROJECT_NAME}_MBED PUBLIC
	${PROJECT_SOURCE_DIR}/mbedtls-3.5.1/include
	${PROJECT_SOURCE_DIR}/mbedtls-3.5.1/library
	${PROJECT_SOURCE_DIR}/gp
	${PROJECT_SOURCE_DIR}/gsma
	${PROJECT_SOURCE_DIR}/lua
	${PROJECT_SOURCE_DIR}/shell
)

target_link_libraries(${CMAKE_PROJECT_NAME}_MBED
	${CMAKE_PROJECT_NAME}_MBEDTLS
)