
import os
import subprocess
import tempfile


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
arguments = [
    'bin/Debug/spritebaker',
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

with tempfile.TemporaryFile(mode='wt', suffix='.filelist', delete=True) as temp_input_file:
    for file in get_files('res/images', 'png'):
        temp_input_file.write(file + '\n')

    temp_input_file.flush()

    arguments.append(temp_input_file.name)
    # print(" ".join(arguments))
    subprocess.call(arguments)
