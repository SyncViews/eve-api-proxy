@echo off
SET VCVARSALL="%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat"

echo Building zlib assembler
call %VCVARSALL% x86
pushd zlib\contrib\masmx86\
call bld_ml32.bat
popd

call %VCVARSALL% amd64
pushd zlib\contrib\masmx64\
call bld_ml64.bat
popd

echo Upgrading zlib project file from VC11 to VC14
pushd zlib\contrib\vstudio\vc11
devenv zlibvc.sln /upgrade

REM TODO: Maybe patch the file or just ship own project files or somthing...
echo "zlib\contrib\vstudio\vc11\zlibvc.sln zlibstat Release|Win32 configuration"
echo "must be changed to use /MD rather than /MT runtime"
pause

echo Building zlib
echo "Debug|Win32"
devenv zlibvc.sln /build "Debug|Win32" /project zlibstat
echo "Release|Win32"
devenv zlibvc.sln /build "Release|Win32" /project zlibstat
echo "Debug|x64"
devenv zlibvc.sln /build "Debug|x86" /project zlibstat
echo "Release|x64"
devenv zlibvc.sln /build "Release|x64" /project zlibstat

popd

