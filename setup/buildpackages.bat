@echo off
pushd %~dp0

set SRC=%~dp0\bin

REM COMMON
REM ------------------------------------
set DEST=%~dp0\packages\org.kingnak.sietrans.common\data
rmdir /S /Q "%DEST%"
mkdir "%DEST%"
copy /Y "%SRC%\SieTrans.exe" "%DEST%"

popd
