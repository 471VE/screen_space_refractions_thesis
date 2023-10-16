import os
import subprocess
import pathlib

if __name__ == '__main__':
    glslang_cmd = "glslangValidator"

    shader_list = ["shader.vert", "shader.frag", "sky_shader.vert", "sky_shader.frag", "sky_shader_refraction.frag"]

    for shader in shader_list:
        subprocess.run([glslang_cmd, "-V", shader, "-o", "../../resources/shaders/{}.spv".format(shader)])

