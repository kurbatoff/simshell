cmake_minimum_required(VERSION 3.25)
project(simshell VERSION 0.11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -O0 -Wextra \
	-Wno-unused-parameter -Wno-missing-field-initializers \
	-Wmissing-declarations -Wredundant-decls -Wcast-align \
	-Wformat -Wformat-security"
)


find_package(ZLIB)
if (ZLIB_FOUND)
	message(STATUS "zlib found")
else()
	message(FATAL_ERROR "zlib not found on the host system.\n"
						"Pass the following command to install it:\n"
						"(Ubuntu): sudo apt install zlib1g-dev\n"
						"(MacOS): brew install zlib"
	)
endif()

# find_package(LIBZIP)
# if (LIBZIP_FOUND)
# 	message(STATUS "libzip found")
# else()
# 	message(FATAL_ERROR "libzip not found on the host system.\n")
# endif()

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