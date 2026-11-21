#ifndef MIDI_UTILS_H
#define MIDIUTILS_H

#include "kbd_utils.h"

void send_midi_event(client_info_t moc, char byte1, char byte2);
int open_midi_out_port(snd_seq_t **seq, int *port_id, const char *name);
int check_input_client_exists(const char *partOfName);

#endif // MIDI_UTILS_H
