# kbd_midi - using computer keyboard as MIDI controller.

*This is a Linux-only application*

There’s a *test.sh* script that builds and tests the application. 
Every key press or release will print the key name and MIDI message it sends.

The script uses these arguments: --keys 1,2,3 --values 10,20,30 --test


```bash
$ ./kbd_midi_aarch64.bin
You must be admin or belong to group 'Input'.
App. creates a MIDI output port and sends MIDI note or CC when
a key is pressed or released.
When key is released velocity is 0.
Arguments:
  --keys - list of key names to send note, required
  --values - list of MIDI values assigned to keys (0-127), required
  --send-cc - send CC (control change) instead of note, default: false
  --client - MIDI client name default: kbd_midi
  --channel - MIDI channel (0-15), default: 0
  --velocity - MIDI velocity when pressed (1-127), default: 127
  --test - test mode, print key name and MIDI value
  --help - show this info

Examples, note quotes around spaces:
--keys '1, 2, q, w' --values 11,12,13,14 --channel 2
--keys 1,2 --values '11 , 12' --velocity 50
--keys 'space , tab' --values ' 11 , 12' --client my_midi --channel 0
--keys '~ , F2' --values '11,12' --send-cc
```
