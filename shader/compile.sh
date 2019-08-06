#!/bin/zsh


glslDir="glsl"
spvDir="spv"

glslangValidator=$VULKAN_SDK/bin/glslangValidator

echo vert:
for f in $glslDir/*.vert; do
	echo $(basename "$f")
	$glslangValidator -V $f -o $spvDir/$(basename "$f").spv
done

echo frag:
for f in $glslDir/*.frag; do
	echo $(basename "$f")
	$glslangValidator -V $f -o $spvDir/$(basename "$f").spv
done

#for f in $glslDir\*.frag; do
#    $glslangValidator -V $glslDir\~nxf -o $spvDir\~nxf.spv
#done
