@echo off

set RTS_BUILD=%RTS_ROOT%/build/
set RTS_CODE=%RTS_ROOT%/code/

set MODULE_NAME=openddl
set MODULE_BUILD=%RTS_BUILD%%MODULE_NAME%/
set MODULE_CODE=%RTS_CODE%%MODULE_NAME%/

if not exist %MODULE_BUILD% mkdir %MODULE_BUILD%

rem - Why does this need quotes??? Otherwise I get "syntax is incorrect"
pushd "%MODULE_BUILD%"

rem - 4061 and 4062 - Missing label in switch statement
rem - 4201 - Warning about non-standard anonymous struct
rem - 4100 - Unreferenced formal parameter
rem - 4189 - Local variable is initialized but not referenced
rem - 4456 - Shadow local variable name
rem - 4505 - Unreferenced local function

set CommonCompilerDefines=-DDEBUG_BUILD=1
set CommonCompilerFlags=-MTd -nologo -I%RTS_CODE% -Gm- -GR- -EHa- -Od -fp:fast -Oi -WX -W4 -w44061 -w44062 -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -FC -Z7
set CommonLinkerFlags=-incremental:no -opt:ref

rem - This depends on hgen being built...

echo ^> Hgen - [Begin]
%RTS_BUILD%/hgen/hgen.exe %MODULE_CODE% %RTS_CODE% > nul
echo ^> Hgen - [Complete]

echo ^> Compiling Tests [Begin]
cl %CommonCompilerFlags% %CommonCompilerDefines% "%RTS_ROOT%/code/openddl/tests/tests.cpp" /link %CommonLinkerFlags%
echo ^> Compiling Tests - [Complete]

echo ^> Running Tests [Begin]
tests.exe
echo ^> Running Tests [Complete]

popd
