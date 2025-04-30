@echo off
date /t && time /t
set SIFLI_SDK=%cd%
set TOOLS_ROOT=%cd%/tools
dir

if "%2"=="gcc" (
    set RTT_EXEC_PATH=C:\GNU MCU Eclipse\ARM Embedded GCC\8.2.1-1.2-20190119-1237\bin
    set RTT_CC=gcc
    call set_env.bat gcc
)
if "%2"=="keil" (
    set RTT_EXEC_PATH=C:/Keil_v5
    set RTT_CC=keil
    call set_env.bat
)

cd %1

:BUILD_LIB
call msvc_setup.bat
call scons --buildlib=lib_bt -j8
set build_result=%errorlevel%
goto :BUILD_END

:BUILD_END
date /t && time /t
cd %SIFLI_SDK%
exit /b %build_result%


::@echo off
::setlocal EnableDelayedExpansion
::cd ..\..\
::call set_env.bat
::
::::bt lib
::cd middleware\bluetooth\project\bluetooth_host
::call scons --buildlib=lib_bt -j8
::if %errorlevel% neq 0 exit /b %errorlevel%
::cd ..\..\..\..\
::
::::bt trans_only lib
::cd middleware\bluetooth\project\bluetooth_host_trans_only
::call scons --buildlib=lib_bt -j8
::if %errorlevel% neq 0 exit /b %errorlevel%
::cd ..\..\..\..\
::
::::bt simulator lib
::cd middleware\bluetooth\project\bluetooth_host_simulator
::call msvc_setup.bat
::call scons --buildlib=lib_bt -j8
::if %errorlevel% neq 0 exit /b %errorlevel%
::cd ..\..\..\..\
::
::::make gcc lib 
::call set_env.bat gcc
::::bt lib
::cd middleware\bluetooth\project\bluetooth_host
::call scons --buildlib=lib_bt -j8
::if %errorlevel% neq 0 exit /b %errorlevel%
::cd ..\..\..\..\
::
::echo ---------- make bt lib success ---------