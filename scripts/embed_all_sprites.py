
import os
from embed_to_h import convert_file

def get_files():
    file_paths = []
    for root, dir, files in os.walk('res/sprites'):
        for filename in files:
            if not filename.startswith('.') and filename.endswith('sprite'): # Skip .DS_Store files
                file_paths.append(os.path.join(root, filename))

    return file_paths

for file in get_files():
    convert_file('-ascii', file)
