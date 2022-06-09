setlocal

:: make path local because vcvarsall.bat adds to it
setlocal Path=%PATH%

set SRC=%cd%
set COM_BUILD="C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
set PRO_BUILD="C:\Program Files (x86)\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
set ENT_BUILD="C:\Program Files (x86)\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
set PRE_BUILD="C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat"

call %PRE_BUILD% x64

:: set CMAKE_PREFIX_PATH=C:\copperspice\cmake\CopperSpice

:: -DWITH_WEBKIT=NO 
:: -DWITH_MYSQL_PLUGIN=NO
:: -DCMAKE_INSTALL_PREFIX=install

mkdir x64
mkdir x64\release
pushd x64\release
::cmake -DCMAKE_INSTALL_PREFIX=install -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release -Wno-dev -G "Visual Studio 17 2022" %SRC%
cmake -DCMAKE_INSTALL_PREFIX=install -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release -Wno-dev -G Ninja %SRC%
popd

mkdir x64\debug
pushd x64\debug
::cmake -DCMAKE_INSTALL_PREFIX=install -DBUILD_TESTS=ON -DWITH_WEBKIT=NO -DCMAKE_BUILD_TYPE=Debug -Wno-dev -G "Visual Studio 17 2022" %SRC%
cmake -DCMAKE_INSTALL_PREFIX=install -DBUILD_TESTS=ON -DWITH_WEBKIT=NO -DCMAKE_BUILD_TYPE=Debug -Wno-dev -G Ninja %SRC%
popd

set NINJA_STATUS=[%e %f/%t @%r] 

@echo off
echo Compile using: (order of arguments is important!)
echo cmake --build x64\release
echo   or 
echo cmake --build x64\debug

pause
