@echo off
SET VCVARSALL="%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat"
pushd boost
call %VCVARSALL% amd64

echo Boost Bootstrap
pushd tools\build
call bootstrap.bat
call b2 install --prefix="..\..\boost-build"
popd
set PATH=%CD%\boost-build\bin;%PATH%

call %VCVARSALL% x86
call b2 --build-dir=build32 --stagedir=stage-Win32 ^
    toolset=msvc ^
    variant=debug,release ^
    link=static ^
    threading=multi ^
    address-model=32 ^
    runtime-link=shared ^
    --with-test ^
    --with-filesystem ^
    -j 16 stage

call %VCVARSALL% amd64
call b2 --build-dir=build64 --stagedir=stage-x64 ^
    toolset=msvc ^
    variant=debug,release ^
    link=static ^
    threading=multi ^
    address-model=64 ^
    runtime-link=shared ^
    --with-test ^
    --with-filesystem ^
    -j 16 stage

popd

