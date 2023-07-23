
import os
import subprocess


def get_files(folder, extension):
    file_paths = []
    for root, dir, files in os.walk(folder):
        for filename in files:
            if filename.endswith(extension):
                file_paths.append(os.path.join(root, filename))

    file_paths.sort()
    return file_paths


arguments = [
    'bin/spritebaker',
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

for file in get_files('res/images', 'png'):
    arguments.append(file)

# arguments = arguments[0:20]
# print(" ".join(arguments))
subprocess.call(arguments)
