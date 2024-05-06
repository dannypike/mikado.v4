rem Convert the output from the MNIST DCGAN to a PNG file
@echo off

setlocal enableextensions
set SAMPLE_FOLDER=%~dp0..\..\bin

if !%1!==!! (
    for %%i in (%SAMPLE_FOLDER%\dcgan-sample-*.pt) do (
        call :process_sample %%~pi%%~ni
        )
    goto :cleanup
) else (
    call :process_sample %SAMPLE_FOLDER%\dcgan-sample-%1.pt
)
goto :cleanup

:process_sample
set INPUT=%1.pt
set OUTPUT=%1.png

if not exist %INPUT% (
    echo %INPUT% does not exist
    goto :cleanup
)

call python display_samples.py -i %INPUT% -o %OUTPUT%
goto :cleanup

:cleanup
goto :eof
