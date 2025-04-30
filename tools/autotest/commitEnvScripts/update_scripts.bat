@echo off
setlocal enabledelayedexpansion
set /p line=<.git
echo line is %line%
if "%line%"=="" (
    echo run normal_ws function
    copy  /Y tools\autotest\commitEnvScripts\commit-msg .\.git\hooks\>nul
    copy /Y tools\autotest\commitEnvScripts\pre-commit .\.git\hooks\>nul
    git config commit.template .commit_template
) else (
    echo run worktree_ws function
    if "%line%"=="%line:modules= %" (
        echo ws is not submodule.
        set "line=%line:worktrees= %"
    ) else (
        echo ws is submodule.
    )
    for /f  "tokens=2 delims= " %%a in ("!line!") do (
        set "dst=%%a"
    )
    set "dst=!dst:/=\!"
    echo !dst!
    ::echo "copy  /Y tools\ci\commitEnvScripts\commit-msg !dst!hooks\>nul"
    copy  /Y tools\autotest\commitEnvScripts\commit-msg !dst!\hooks\>nul
    copy /Y tools\autotest\commitEnvScripts\pre-commit !dst!\hooks\>nul
    git config commit.template .commit_template
)

:end
echo scripts update success