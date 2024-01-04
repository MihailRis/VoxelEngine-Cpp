glslc -g -fshader-stage=vert ./main.vert.glsl -o ../shaders/main.vert.spv
glslc -g -fshader-stage=frag ./main.frag.glsl -o ../shaders/main.frag.spv

glslc -g -fshader-stage=vert ./lines.vert.glsl -o ../shaders/lines.vert.spv
glslc -g -fshader-stage=frag ./lines.frag.glsl -o ../shaders/lines.frag.spv

glslc -g -fshader-stage=vert ./ui.vert.glsl -o ../shaders/ui.vert.spv
glslc -g -fshader-stage=frag ./ui.frag.glsl -o ../shaders/ui.frag.spv

glslc -g -fshader-stage=vert ./ui3d.vert.glsl -o ../shaders/ui3d.vert.spv
glslc -g -fshader-stage=frag ./ui3d.frag.glsl -o ../shaders/ui3d.frag.spv

glslc -g -fshader-stage=vert ./skybox_gen.vert.glsl -o ../shaders/skybox_gen.vert.spv
glslc -g -fshader-stage=frag ./skybox_gen.frag.glsl -o ../shaders/skybox_gen.frag.spv

glslc -g -fshader-stage=vert ./background.vert.glsl -o ../shaders/background.vert.spv
glslc -g -fshader-stage=frag ./background.frag.glsl -o ../shaders/background.frag.spv
