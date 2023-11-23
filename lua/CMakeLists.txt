add_library(${CMAKE_PROJECT_NAME}_LUA STATIC
	luawrap.c
)

target_include_directories(${CMAKE_PROJECT_NAME}_LUA PUBLIC
	${PROJECT_SOURCE_DIR}/Lua5.3/include
	${PROJECT_SOURCE_DIR}/pcsc
	${PROJECT_SOURCE_DIR}/shell
)

target_link_libraries(${CMAKE_PROJECT_NAME}_LUA
	${CMAKE_PROJECT_NAME}_SHELL
	${CMAKE_PROJECT_NAME}_PCSC
)