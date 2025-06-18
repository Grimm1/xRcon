@echo off
setlocal enabledelayedexpansion

set "outputFile=%~dp0merged_output.txt"
echo. > "%outputFile%"

for %%F in ("%~dp0*.h" "%~dp0*.cpp" "%~dp0*.rc" "%~dp0*.md") do (

        echo --- %%~fF --- >> "%outputFile%"
        type "%%F" >> "%outputFile%"
        echo. >> "%outputFile%"
    )
)

echo Processing complete. Check "%outputFile%".