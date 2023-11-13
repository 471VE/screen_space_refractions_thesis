import os
import subprocess
import pathlib

if __name__ == '__main__':
    glslang_cmd = "glslangValidator"

    shader_list = ["model.vert", "model.frag", "simple_skybox.vert", "simple_skybox.frag", "refraction.frag"]

    for shader in shader_list:
        subprocess.run([glslang_cmd, "-V", shader, "-o", "../../resources/shaders/{}.spv".format(shader)])

