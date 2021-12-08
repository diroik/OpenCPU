@echo off
@echo %PATH% | findstr /c:"%~dp0tools/msys64/usr/bin">nul
@if %errorlevel% equ 1 set PATH=%~dp0tools/msys64/usr/bin;%PATH%
rem @set PATH=%~dp0tools/msys64/usr/bin;%PATH%

set PROJECT_NAME=quectel_project
set BOARD_NAME=qcx212_0h00
set CHIP_NAME=qcx212

set KEILCC_PATH="C:/Keil_v5/ARM/ARMCC/bin/"
set PROTOCOL_TOOL_DB_PATH="..\PROTOCOL\Tool\UnilogPsParser\DB"
set COMMON_TOOL_DB_PATH="..\TOOLS\UniLogParser\DB"

echo build.bat version 20180330
echo KEILCC_PATH: %KEILCC_PATH%

if not %KEILCC_PATH% == "" (
   if not exist %KEILCC_PATH% (
      echo ERROR: Please check KEILCC_PATH setting, exit!!!
      goto end
   )
) else (
	echo ERROR: Please set KEILCC_PATH firstly, exit!!!
	goto end
)


set PARAMETERS=%1
if xx%PARAMETERS%==xx  (
echo no input paramter, use default build setting
echo default Board   is: %BOARD_NAME%
echo default Chip    is: %CHIP_NAME%
echo default Project is: %PROJECT_NAME%
)


for /f "tokens=1* delims=" %%a in ('type ".\device\target\board\%BOARD_NAME%\%BOARD_NAME%.mk"') do (
	  if "%%a" equ "BUILD_USE_OCPU_PREBUILD_LIB = n" (
			echo BUILD_USE_OCPU_PREBUILD_LIB = y
		) else (
			echo %%a
		)

)>>temp.txt

type temp.txt | findstr /v ECHO >>temp1.txt
del temp.txt
move temp1.txt ".\device\target\board\%BOARD_NAME%\%BOARD_NAME%.mk" >nul


echo OPTION: %PARAMETERS% | findstr "help" 
if not errorlevel 1 (
echo "=========================================================================================="
echo "                                                                                          "                                                                                          
echo "---------------------------Show How to Build Project--------------------------------------"
echo "                                                                                          "                                                             
echo "=========================================================================================="
echo "                                                                                          "
echo "                                                                                          "    
echo "                         make.bat Board-Project-Option                               		"
echo "Board,Project,Option could be omitted, if so default Board and project will be used      	"
echo "                                                                                          "
echo "                                                                                          "   
echo "******************************************************************************************"
echo "Supported Options:																		"
echo "       clean                                            									"
echo "       help---show how to use the build script                                            "
echo "------------------------------------------------------------------------------------------"
echo "       make																				"
echo "******************************************************************************************"
echo "       make clean																			"
echo "------------------------------------------------------------------------------------------"
goto end
)

echo OPTION: %PARAMETERS% | findstr "clean"
if not errorlevel 1 (
	make.exe -j4  clean TARGET=%BOARD_NAME% CROSS_COMPILE=%KEILCC_PATH% PROJECT=%PROJECT_NAME% 
	echo clean done ok...
	goto end
)


set starttime=%time%
echo Start time: %date% %starttime%

echo     #################### STEP1: configuration #############################
set BUILD_CREATE_OCPU_PREBUILD_LIB=y
if not exist .\out (
	mkdir .\out
	)
echo.>> .\out\outbuildlog.log
echo configuration is successful

echo     #################### STEP2: make quectel_ocpu_app #####################
 (make.exe -j4 quectel_ocpu_app TARGET=%BOARD_NAME% V=%VERBOSE% CROSS_COMPILE=%KEILCC_PATH% PROJECT=quectel_project 2>&1 ) | tee.exe .\out\outbuildlog.txt

rem add by taber 20200322 check warning error
echo     #######################################################################
find "Program Size:" .\out\outbuildlog.log
for /f "delims=: tokens=2" %%i in (' find /C /I "Warning:" .\out\outbuildlog.log ') do set warning_num=%%i
for /f "delims=: tokens=2" %%i in (' find /C /I "Error:" .\out\outbuildlog.log ') do set error_num=%%i
echo warning_num=%warning_num%, error_num=%error_num%

if %error_num% GTR 0 (
echo     #######################################################################
echo     ##                                                                   ##
echo     ##                    ########    ###     ####  ##                   ##
echo     ##                    ##         ## ##     ##   ##                   ##
echo     ##                    ##        ##   ##    ##   ##                   ##
echo     ##                    ######   ##     ##   ##   ##                   ##
echo     ##                    ##       #########   ##   ##                   ##
echo     ##                    ##       ##     ##   ##   ##                   ##
echo     ##                    ##       ##     ##  ####  ########             ##
echo     ##                                                                   ##
echo     #######################################################################  
goto end
)

if %warning_num% GTR 0 (
echo     #######################################################################
echo     ##                                                                   ##
echo     ##           ##          ##    ###      ########  ###     ##         ## 
echo     ##           ##          ##   ## ##     ##    ##  ####    ##         ##
echo     ##           ##    #    ##   ##   ##    ##    ##  ## ##   ##         ##
echo     ##            ##   #   ##   ##     ##   ##   ##   ##  ##  ##         ##
echo     ##             ## ### ##   ###########  ## ##     ##   ## ##         ##
echo     ##             ## # # ##   ##       ##  ##   ##   ##    ####         ##
echo     ##             ###   ###   ##       ##  ##    ##  ##     ###         ##
echo     ##                                                                   ##
echo     #######################################################################  
)

rem copy log database to output dir after compile successfully
rem cp .\tools\comdb.txt .\out\%BOARD_NAME%\%PROJECT_NAME%\ || (goto:failHandle)


:complete

set endtime=%time%
echo .
echo End time: %date% %endtime%

set /a h1=%starttime:~0,2%
set /a m1=1%starttime:~3,2%-100
set /a s1=1%starttime:~6,2%-100
set /a h2=%endtime:~0,2%
set /a m2=1%endtime:~3,2%-100
set /a s2=1%endtime:~6,2%-100
if %h2% LSS %h1% set /a h2=%h2%+24
set /a ts1=%h1%*3600+%m1%*60+%s1%
set /a ts2=%h2%*3600+%m2%*60+%s2%
set /a ts=%ts2%-%ts1%
set /a h=%ts%/3600
set /a m=(%ts%-%h%*3600)/60
set /a s=%ts%%%60
echo Built took %h% hours %m% minutes %s% seconds

echo     #######################################################################
echo     ##                                                                   ##
echo     ##                 ########     ###     ######   ######              ##
echo     ##                 ##     ##   ## ##   ##    ## ##    ##             ##
echo     ##                 ##     ##  ##   ##  ##       ##                   ##
echo     ##                 ########  ##     ##  ######   ######              ##
echo     ##                 ##        #########       ##       ##             ##
echo     ##                 ##        ##     ## ##    ## ##    ##             ##
echo     ##                 ##        ##     ##  ######   ######              ##
echo     ##                                                                   ##
echo     #######################################################################

echo build successfully

:end
goto:eof

:failHandle
echo fail
echo     #######################################################################
echo     ##                                                                   ##
echo     ##                    ########    ###     ####  ##                   ##
echo     ##                    ##         ## ##     ##   ##                   ##
echo     ##                    ##        ##   ##    ##   ##                   ##
echo     ##                    ######   ##     ##   ##   ##                   ##
echo     ##                    ##       #########   ##   ##                   ##
echo     ##                    ##       ##     ##   ##   ##                   ##
echo     ##                    ##       ##     ##  ####  ########             ##
echo     ##                                                                   ##
echo     ####################################################################### 
goto:eof

