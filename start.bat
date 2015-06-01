@ECHO OFF 

:_start
call :CheckEmpty "E:\com.txt".

if %_ret% equ 0 (
	goto Ask4Try
	:_input
    TTPMACRO E:\sBox\sBox\inputCOM.TTL
	goto _start
) else if %_ret% equ 2 (
    goto _end
)

:Ask4Try
SET /P runscript="SSH configuraiton not found or not correct. Configure again? (Y/N) "
if %runscript%==y goto _input
if %runscript%==Y goto _input
if %runscript%==n goto _end
if %runscript%==N goto _end
goto Ask4Try

:CheckEmpty
if "%~z1" == "" ( 
    echo File doesnt exist.
	set _ret=0
) else if "%~z1" == "0" ( 
    echo File is empty
	set _ret=1
) else ( 
    echo Configuraiton found!
	set _ret=2
)
goto:eof

:_end
echo Quit.