@echo off
@set VMTOOL="D:\dev\tools\VMProtect Ultimate\VMProtect_Con.exe"
@set TARGET_DIR=%1
@set TARGET_NAME=%2
@set TARGET_EXT=%3
@set COPY_TO_DIR=%4
@set HAS_VMP=%5

@if %HAS_VMP% equ "1" (
	@set SRC_FILE=%TARGET_DIR%%TARGET_NAME%%TARGET_EXT%.vmp
	::call %VMTOOL% %SRC_FILE%
	::@copy %TARGET_DIR%\%TARGET_NAME%.vmp%TARGET_EXT% %TARGET_DIR%\%TARGET_NAME%%TARGET_EXT% /Y
)

@if %HAS_VMP% equ "1" (
	::call %VMTOOL% %SRC_FILE%
	::@copy %TARGET_DIR%\%TARGET_NAME%.vmp%TARGET_EXT% %TARGET_DIR%\%TARGET_NAME%%TARGET_EXT% /Y
)

::数字签名
@set SIGN_TOOL_DIR=%TARGET_DIR%\..\product\sign
@set SIGN_TOOL=%SIGN_TOOL_DIR%\signtool.exe
::@cd %SIGN_TOOL_DIR%
::%SIGN_TOOL% sign /v /ac MS_xs_UTN.cer /f yedone.pfx /p YEDONE08yedone /tr http://timestamp.wosign.com/rfc3161 %TARGET_DIR%\%TARGET_NAME%%TARGET_EXT%
::%SIGN_TOOL% verify /v /kp %TARGET_DIR%\%TARGET_NAME%%TARGET_EXT%

::复制目标文件到lib目录
::@copy %TARGET_DIR%\%TARGET_NAME%%TARGET_EXT% %COPY_TO_DIR%\%TARGET_NAME%%TARGET_EXT%
::@copy %TARGET_DIR%\%TARGET_NAME%.pdb %COPY_TO_DIR%\%TARGET_NAME%.pdb
@copy %TARGET_DIR%\%TARGET_NAME%.lib %COPY_TO_DIR%\%TARGET_NAME%.lib