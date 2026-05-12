# generate_website.py — remplace minify_www.py sur Windows
# Pas besoin de node.js. Lance avec : python generate_website.py
import os, gzip

source_dir = "src/web/orig"
target_file = "src/web/website.h"
output = ""

def add_file(name, data):
    global output
    gz = gzip.compress(data, compresslevel=9)
    n = name.replace('.','_')
    output += f"\n//File: {name}.gz size:{len(gz)}\n"
    output += f"#define {n}_gz_len {len(gz)}\n"
    output += f"const uint8_t {n}_gz[] = {{\n"
    for i, b in enumerate(gz):
        output += f" 0x{b:02X}"
        if i < len(gz)-1: output += ","
        if (i+1) % 16 == 0: output += "\n"
    output += "};\n\n"
    print(f"  {name}: {len(data)} → {len(gz)} gz")

for f in sorted(os.listdir(source_dir)):
    with open(os.path.join(source_dir, f), 'rb') as fp:
        add_file(f, fp.read())

with open(target_file, 'w') as fp:
    fp.write(output)

print(f"\n✓ {target_file} ({os.path.getsize(target_file)//1024}KB)")
