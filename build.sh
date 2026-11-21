#!/bin/bash

clear

# optional update
cp -v ./../handy_string/handy_string.* ./src/ 2>/dev/null
cp -v ./../handy_list/handy_list.* ./src/ 2>/dev/null

rm -v *.so *.bin 2>/dev/null
os=$(uname -s)

if [[ "$os" != *Linux* ]]; then
    echo "Unsupported OS: $os"
    exit 1
fi

arch=$(uname -m)
target_file="kbd_midi_${arch}.bin"

gcc -O2 -Wall -fPIC -fvisibility=hidden \
  ./src/main.c ./src/kbd_utils.c  ./src/midi_utils.c ./src/handy_string.c \
  -o "$target_file" -lasound

if [ ! -f "$target_file" ]; then exit 1; fi

cp -v "$target_file" ./../musiloop/bindir/ 







