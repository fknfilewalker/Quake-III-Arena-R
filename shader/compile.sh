rem Generate spv from glsl

set glslDir=glsl
set spvDir=spv

set glslangValidator=%VULKAN_SDK%\bin\glslangValidator %1

for %%f in (%glslDir%\*.vert) do (
    %glslangValidator% -V %glslDir%\%%~nxf -o %spvDir%\%%~nxf.spv
)

for %%f in (%glslDir%\*.frag) do (
    %glslangValidator% -V %glslDir%\%%~nxf -o %spvDir%\%%~nxf.spv
)
$VULKAN_SDK/bin/glslangValidator  -V glsl/singleTextureClip.vert -o spv/singleTextureClip.vert.spv 
:quit
