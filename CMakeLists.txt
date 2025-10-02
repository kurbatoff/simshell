cmake_minimum_required(VERSION 3.25)
project(SimShell VERSION 0.11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -Wextra \
	-Wno-unused-parameter -Wno-missing-field-initializers \
	-Wmissing-declarations -Wredundant-decls -Wcast-align \
	-Wformat -Wformat-security"
)

# Find the libzip package
# find_package(libzip CONFIG REQUIRED)


if(UNIX OR APPLE)
	set(LIBZIP_LIBRARY zip)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I/usr/include/PCSC")
elseif(WIN32)
	set(LIBZIP_LIBRARY libzip)
endif()

add_subdirectory(gp)
add_subdirectory(gsma)
add_subdirectory(lua)
add_subdirectory(cap)
add_subdirectory(Lua5.3)
add_subdirectory(mbed)
add_subdirectory(mbedtls-3.5.1/library)
add_subdirectory(pcom)
add_subdirectory(pcsc)
add_subdirectory(shell)


add_executable(${CMAKE_PROJECT_NAME}
	shell/main.c
)

target_link_libraries(${CMAKE_PROJECT_NAME}
	${CMAKE_PROJECT_NAME}_SHELL
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/shell
)