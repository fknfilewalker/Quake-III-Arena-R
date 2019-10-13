rem Generate spv from glsl

set glslDir=glsl
set spvDir=spv
set headerDir=header

set glslangValidator=%VULKAN_SDK%\bin\glslangValidator %1

rem SPV
for %%f in (%glslDir%\*.vert) do (
    %glslangValidator% -V %glslDir%\%%~nxf -o %spvDir%\%%~nxf.spv
)

for %%f in (%glslDir%\*.frag) do (
    %glslangValidator% -V %glslDir%\%%~nxf -o %spvDir%\%%~nxf.spv
)

Rem RTX
for %%f in (%glslDir%\*.rgen) do (
    %glslangValidator% -V %glslDir%\%%~nxf -o %spvDir%\%%~nxf.spv
)
for %%f in (%glslDir%\*.rmiss) do (
    %glslangValidator% -V %glslDir%\%%~nxf -o %spvDir%\%%~nxf.spv
)
for %%f in (%glslDir%\*.rchit) do (
    %glslangValidator% -V %glslDir%\%%~nxf -o %spvDir%\%%~nxf.spv
)
for %%f in (%glslDir%\*.rahit) do (
    %glslangValidator% -V %glslDir%\%%~nxf -o %spvDir%\%%~nxf.spv
)

rem C HEADER
for %%f in (%glslDir%\*.vert) do (
    %glslangValidator% -V %glslDir%\%%~nxf --vn %%~nfVert -o %headerDir%\%%~nxf.h
)

for %%f in (%glslDir%\*.frag) do (
    %glslangValidator% -V %glslDir%\%%~nxf --vn %%~nfFrag -o %headerDir%\%%~nxf.h
)

Rem RTX
for %%f in (%glslDir%\*.rgen) do (
    %glslangValidator% -V %glslDir%\%%~nxf --vn %%~nfRGen -o %headerDir%\%%~nxf.h
)
for %%f in (%glslDir%\*.rmiss) do (
    %glslangValidator% -V %glslDir%\%%~nxf --vn %%~nfRMiss -o %headerDir%\%%~nxf.h
)
for %%f in (%glslDir%\*.rchit) do (
    %glslangValidator% -V %glslDir%\%%~nxf --vn %%~nfRCHit -o %headerDir%\%%~nxf.h
)
for %%f in (%glslDir%\*.rahit) do (
    %glslangValidator% -V %glslDir%\%%~nxf --vn %%~nfRAHit -o %headerDir%\%%~nxf.h
)

:quit
