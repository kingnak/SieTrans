@echo off
pushd %~dp0

REM TODO: Dynamic?
set SRC=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Redist\MSVC\14.16.27012\x64\Microsoft.VC141.CRT

set DEST=%~dp0\packages\org.kingnak.sietrans.vs2019\data
rmdir /S /Q "%DEST%"
mkdir "%DEST%"
copy /Y "%SRC%\msvcp140.dll" "%DEST%"
copy /Y "%SRC%\vccorlib140.dll" "%DEST%"
copy /Y "%SRC%\vcruntime140.dll" "%DEST%"

popd
