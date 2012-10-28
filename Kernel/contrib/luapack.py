#!/usr/bin/python2

import sys
import struct

path = sys.argv[1]
target = sys.argv[2]
f = open(path, "r")
source = f.read()
f.close()

fs_path = path.replace("src/", "")
data = struct.pack(">H", len(fs_path))
data += fs_path
data += struct.pack(">L", len(source))
data += source

f = open(target, "wb")
f.write(data)
f.close()