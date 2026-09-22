
import os
import subprocess
import tempfile
import platform


def get_files(folder, extension):
    file_paths = []
    for root, dir, files in os.walk(folder):
        for filename in files:
            if filename.endswith(extension):
                file_paths.append(os.path.normpath(os.path.join(root, filename)))

    file_paths.sort()
    return file_paths

#
# Hardcoded binary path is not great.
#
if platform.system() == 'Darwin':
    binary_file = 'bin/spritebaker'
elif platform.system() == 'Windows':
    binary_file = 'bin/Debug/spritebaker.exe'
else:
    binary_file = 'bin/Debug/spritebaker'

# Some Python installs (e.g. the Microsoft Store build) fail to resolve a relative
# executable path in subprocess.call even with a correct cwd, so make it absolute.
binary_file = os.path.abspath(binary_file)

arguments = [
    binary_file,
    '-width', '2048',
    '-height', '1024',
    '-padding', '2',
    '-bg_color', '255 0 255 0',
    '-trim_images',
    '-sprite_format',
    '-sprite_folder', 'res/sprites/',
    '-output', 'res/sprite_atlas.png'
]

arguments.append('-input')

with tempfile.NamedTemporaryFile(mode='wt', suffix='.filelist', delete=False) as temp_input_file:
    for file in get_files('res/images', 'png'):
        temp_input_file.write(file + '\n')

    temp_input_file.flush()

    arguments.append(temp_input_file.name)
    # print(" ".join(arguments))
    subprocess.call(arguments)
