#!/bin/bash

clear

arch=$(uname -m)
target_file="kbd_midi_${arch}.bin"

./build.sh || exit 1

./"$target_file" --keys 1,2,3 --values 10,20,30 --test

