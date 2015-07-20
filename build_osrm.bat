set VARIANT=Release
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall" x86_amd64
set PREFIX=%CD%/libs
set BOOST_ROOT=%CD%/boost_min
set TBB_INSTALL_DIR=%CD%/tbb
set TBB_ARCH_PLATFORM=intel64/vc12

set osrmdir="osrm-backend"
if not exist osrm-bin (mkdir osrm-bin)
REM if not exist osrm-backend (git clone -b develop https://github.com/Project-OSRM/osrm-backend osrm-backend)
REM cd osrm-backend

mkdir build
cd build
cmake %osrmdir% -G "Visual Studio 12 Win64" -DBoost_ADDITIONAL_VERSIONS=1.57 -DCMAKE_BUILD_TYPE=%VARIANT% -DCMAKE_INSTALL_PREFIX=%PREFIX% -DBOOST_ROOT=%BOOST_ROOT% -DBoost_USE_STATIC_LIBS=ON -T CTP_Nov2013
msbuild /p:Configuration=%Variant% /clp:Verbosity=normal /nologo OSRM.sln /flp1:logfile=build_errors.txt;errorsonly /flp2:logfile=build_warnings.txt;warningsonly /maxcpucount:4
copy /y %VARIANT%\*.pdb .
copy /y %VARIANT%\*.exe .
copy /y %VARIANT%\*.dll .

copy *.exe ..\osrm-bin
copy *.dll ..\osrm-bin
if "%VARIANT%"=="Debug" (copy *.pdb ..\osrm-bin)
REM cd ..\profiles
echo disk=c:\temp\stxxl,10000,wincall > ..\osrm-bin\.stxxl.txt
copy "%osrmdir%\profiles\*.*" ..\osrm-bin
copy "%osrmdir%\profiles\car.lua" ..\osrm-bin\profile.lua
copy "%osrmdir%\osrm_c.h" ..\osrm-bin\osrm_c.h
xcopy /y "%osrmdir%\profiles\lib" ..\osrm-bin\lib\ 
copy %PREFIX:/=\%\bin\*.dll ..\osrm-bin

cd ..
