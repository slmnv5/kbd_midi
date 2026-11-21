
#ifndef KBD_UTILS_H
#define KBD_UTILS_H

#include "handy_list.h"
#include "handy_string.h"
#include "kbd_utils.h"
#include <alsa/asoundlib.h>

#define MAX_TOKEN_COUNT 100
#define MAX_TOKEN_LEN 64

typedef struct {
  str_view_t key;
  int value;
} kv_t;

DEFINE_LIST(kv_t, kv, (compare_str_view(a.key, b.key)));

// keep key names and MIDI values
typedef struct {
  list_kv_t list_kv; //  generic list of key as str_view, value as int

  char show_key_pressed; // used in test mode
  char send_cc;
  char channel; // MIDI channel to create
  char velocity;

  char *client_name; // MIDI client to create

  snd_seq_t *seq_handle;
  int port_id;

} client_info_t;

int read_kbd(client_info_t *moc, char *byte1, char *byte2, int fd);

const char *find_keyboard_device();

#endif // KBD_UTILS_H
