add_library(${CMAKE_PROJECT_NAME}_PCOM STATIC
	libApduEngine.c
)

target_include_directories(${CMAKE_PROJECT_NAME}_PCOM PUBLIC
	${PROJECT_SOURCE_DIR}/shell
	${PROJECT_SOURCE_DIR}/pcsc
)

target_link_libraries(${CMAKE_PROJECT_NAME}_PCOM
	${CMAKE_PROJECT_NAME}_SHELL
	${CMAKE_PROJECT_NAME}_pcsc
)