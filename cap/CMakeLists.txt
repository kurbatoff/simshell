add_library(${CMAKE_PROJECT_NAME}_CAP STATIC
	cap.c
)

target_include_directories(${CMAKE_PROJECT_NAME}_CAP PUBLIC
	${PROJECT_SOURCE_DIR}/shell
	${PROJECT_SOURCE_DIR}/pcsc
	${PROJECT_SOURCE_DIR}/gp
)

target_link_libraries(${CMAKE_PROJECT_NAME}_CAP
	${CMAKE_PROJECT_NAME}_SHELL
	libzip
)