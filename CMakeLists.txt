cmake_minimum_required(VERSION 3.25)
project(SimShell VERSION 0.11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -Wextra \
	-Wno-unused-parameter -Wno-missing-field-initializers \
	-Wmissing-declarations -Wredundant-decls -Wcast-align \
	-Wformat -Wformat-security"
)

add_subdirectory(gp)
add_subdirectory(gsma)
add_subdirectory(lua)
add_subdirectory(lua5.3)
add_subdirectory(mbed)
add_subdirectory(mbedtls-3.5.1/library)
add_subdirectory(pcsc)
add_subdirectory(shell)