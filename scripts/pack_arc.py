'''
Copyright (c) 2026 Pyogenics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'''

'''
Simple tool to pack arc files from a target folder
--------------------------------------------------
Usage: pack_arc.py path/to/target/files path/to/output.arc filesToIncludeList1.txt filesToIncludeList2.txt filesToIncludeList3.txt ....

The tool makes use of the txt files inside Common/Media/ that define what files should be included. We should probably put those in 
a better place and maybe use something like json instead.

You can just omit the "files to include lists" to use all files available in the folder instead.
'''

from sys import argv
from glob import glob
from struct import pack
from dataclasses import dataclass


@dataclass
class FileMetaData:
    name: str
    size: int
    data: bytes


def pack_stream(format, stream, *data):
    packedData = pack(format, *data)
    stream.write(packedData)


def write_string(string, stream):
    string = string.encode("utf-8")
    pack_stream(">H", stream, len(string))
    stream.write(string)


target_dir = argv[1]
output_file_path = argv[2]
filter_file_paths = argv[3::]

# Build target file list
target_file_paths = []
if len(filter_file_paths) == 0:
    target_file_paths = glob(f"{target_dir}/*.*", recursive=True)
else:
    for filter_path in filter_file_paths:
        with open(filter_path) as filter_file:
            print(f"Reading filter file: {filter_path}")
            target_file_paths += filter_file.readlines()
print(f"Selected {len(target_file_paths)} files for {output_file_path}")

# Read our target files
target_files = []
for target_path in target_file_paths:
    with open(f"{target_dir}/{target_path.strip()}", "rb") as file: 
        file_data = file.read()
        meta_data = FileMetaData(
            target_path,
            len(file_data),
            file_data
        )
        target_files.append(meta_data)

# Compute header size so we can later calculate file offsets when writing the header
header_size = 4 # Start at 4 to include file count field
for file_meta in target_files:
    header_size += 2 # File name string length
    header_size += len(file_meta.name) # File name
    header_size += 4 # Offset
    header_size += 4 # File size

# Write arc data
current_file_offset = header_size
with open(output_file_path, "wb") as arc_file:
    # File count
    pack_stream(">I", arc_file, len(target_files))

    # Write file entries
    for file_meta in target_files:
        write_string(file_meta.name, arc_file)
        pack_stream(">2I", arc_file,
            current_file_offset,
            file_meta.size
        )
        current_file_offset += file_meta.size
    # Write file data
    for file_meta in target_files:
        arc_file.write(file_meta.data)