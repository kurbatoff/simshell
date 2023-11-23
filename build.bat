echo off
if exist build (
	echo Removind build dir
	rmdir /s /q build
)

mkdir build
cd build
cmake -GNinja ../
cmake --build .
cd ..