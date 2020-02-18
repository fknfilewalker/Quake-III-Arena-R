rem Generate spv from glsl

set glslDir=glsl
set spvDir=spv
set headerDir=header

set glslangValidator=%VULKAN_SDK%\bin\glslangValidator %1

rem SPV
for %%f in (%glslDir%\rasterizer\*.vert) do (
    %glslangValidator% -DGLSL -V %glslDir%\rasterizer\%%~nxf -o %spvDir%\%%~nxf.spv
)
for %%f in (%glslDir%\rasterizer\*.frag) do (
    %glslangValidator% -DGLSL -V %glslDir%\rasterizer\%%~nxf -o %spvDir%\%%~nxf.spv
)
for %%f in (%glslDir%\compute\*.comp) do (
    %glslangValidator% -DGLSL -V %glslDir%\compute\%%~nxf -o %spvDir%\%%~nxf.spv
)

Rem RTX
for %%f in (%glslDir%\raytracer\*.rgen) do (
    %glslangValidator% -DGLSL -V %glslDir%\raytracer\%%~nxf -o %spvDir%\%%~nxf.spv
)
for %%f in (%glslDir%\raytracer\*.rmiss) do (
    %glslangValidator% -DGLSL -V %glslDir%\raytracer\%%~nxf -o %spvDir%\%%~nxf.spv
)
for %%f in (%glslDir%\raytracer\*.rchit) do (
    %glslangValidator% -DGLSL -V %glslDir%\raytracer\%%~nxf -o %spvDir%\%%~nxf.spv
)
for %%f in (%glslDir%\raytracer\*.rahit) do (
    %glslangValidator% -DGLSL -V %glslDir%\raytracer\%%~nxf -o %spvDir%\%%~nxf.spv
)

rem C HEADER
for %%f in (%glslDir%\rasterizer\*.vert) do (
    %glslangValidator% -DGLSL -V %glslDir%\rasterizer\%%~nxf --vn %%~nfVert -o %headerDir%\%%~nxf.h
)
for %%f in (%glslDir%\rasterizer\*.frag) do (
    %glslangValidator% -DGLSL -V %glslDir%\rasterizer\%%~nxf --vn %%~nfFrag -o %headerDir%\%%~nxf.h
)
for %%f in (%glslDir%\compute\*.comp) do (
    %glslangValidator% -DGLSL -V %glslDir%\compute\%%~nxf --vn %%~nfComp -o %headerDir%\%%~nxf.h
)

Rem RTX
for %%f in (%glslDir%\raytracer\*.rgen) do (
    %glslangValidator% -DGLSL -V %glslDir%\raytracer\%%~nxf --vn %%~nfRGen -o %headerDir%\%%~nxf.h
)
for %%f in (%glslDir%\raytracer\*.rmiss) do (
    %glslangValidator% -DGLSL -V %glslDir%\raytracer\%%~nxf --vn %%~nfRMiss -o %headerDir%\%%~nxf.h
)
for %%f in (%glslDir%\raytracer\*.rchit) do (
    %glslangValidator% -DGLSL -V %glslDir%\raytracer\%%~nxf --vn %%~nfRCHit -o %headerDir%\%%~nxf.h
)
for %%f in (%glslDir%\raytracer\*.rahit) do (
    %glslangValidator% -DGLSL -V %glslDir%\raytracer\%%~nxf --vn %%~nfRAHit -o %headerDir%\%%~nxf.h
)

:quit
