
import subprocess
import tempfile
import os
import shutil
import argparse


def run_command(command):
    # print command
    subprocess.call(command, shell=True)


def generate_png(source_icon, output_folder, size, scale):
    output_file_png = '{output_folder}/icon_{size}x{size}@{scale}x.png'.format(output_folder=output_folder, size=size, scale=scale)
    sips_command_line = 'sips -z {size} {size} {input} --out {output}'
    pixel_size = size * scale
    command = sips_command_line.format(size=pixel_size, input=source_icon, output=output_file_png)
    run_command(command)


def generate_icons(input_icon, icns_output_file, temp_dir):

    icon_sizes = [16, 32, 128, 256, 512]
    for size in icon_sizes:
        generate_png(input_icon, temp_dir, size, 1)
        generate_png(input_icon, temp_dir, size, 2)

    iconutil_command_line = 'iconutil --convert icns --output {output} {input_dir}'
    command = iconutil_command_line.format(output=icns_output_file, input_dir=temp_dir)
    run_command(command)


def main():
    parser = argparse.ArgumentParser(description='Take a single source png file and generate a bunch of sizes and then create a icns file.')
    parser.add_argument('--source-png', type=str, required=True, help='Input png file')
    parser.add_argument('--output', type=str, required=True, help='Output icns file')
    args = parser.parse_args()

    tempdir = tempfile.gettempdir() + '/generate_icons.iconset'
    os.mkdir(tempdir)

    generate_icons(args.source_png, args.output, tempdir)

    shutil.rmtree(tempdir)


main()
