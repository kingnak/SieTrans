@echo off
pushd %~dp0

IF "%Qt5Dir64%" == "" (
	set QT5SRC=D:\Qt\5.12.6\msvc2017_64
) ELSE (
	set QT5SRC=%Qt5Dir64%
)

IF NOT EXIST "%QT5SRC%\bin\Qt5Core.dll" (
	echo Qt5Src is not set correctly
	GOTO :Exit
)

REM BASE
REM ------------------------------------
set QT5DEST=%~dp0\packages\org.kingnak.sietrans.qt5.base\data
rmdir /S /Q "%QT5DEST%"
mkdir "%QT5DEST%"
copy /Y "%QT5SRC%\bin\Qt5Core.dll" "%QT5DEST%"
copy /Y "%QT5SRC%\bin\D3Dcompiler_47.dll" "%QT5DEST%"
copy /Y "%QT5SRC%\bin\libEGL.dll" "%QT5DEST%"
copy /Y "%QT5SRC%\bin\libGLESV2.dll" "%QT5DEST%"
copy /Y "%QT5SRC%\bin\opengl32sw.dll" "%QT5DEST%"
copy /Y "%QT5SRC%\bin\Qt5Gui.dll" "%QT5DEST%"
copy /Y "%QT5SRC%\bin\Qt5Widgets.dll" "%QT5DEST%"

mkdir "%QT5DEST%\platforms"
copy /Y "%QT5SRC%\plugins\platforms\qwindows.dll" "%QT5DEST%\platforms"

:Exit
popd

