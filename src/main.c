#ifndef __linux__
#error "This app requires Linux"
#endif

#include "main.h"
#include "handy_string.h"
#include "kbd_utils.h"
#include "midi_utils.h"

static const char *gHelpStr =
    "You must be admin or belong to group 'Input'.\n"
    "App. creates MIDI OUT port and sends MIDI command when\n"
    "typing key is pressed or released\n"
    "when key is released velocity is 0\n"
    "Arguments:\n"
    "  --keys - list of key names to send note, required\n"
    "  --values - list of MIDI values assigned to keys (0-127), required\n"
    "  --send-cc - send CC (control change) instead of note, default: false\n"
    "  --client - MIDI client name default: kbd_midi\n"
    "  --channel - MIDI channel (0-15), default: 0\n"
    "  --velocity - MIDI velocity when pressed (1-127), default: 127\n"
    "  --test - test mode, print key name and MIDI value\n"
    "  --help - show this info\n\n"
    "Examples, note quotes around spaces:\n"
    "--keys '1, 2, q, w' --values 11,12,13,14 --channel 2\n"
    "--keys 1,2 --values '11 , 12' --velocity 50\n"
    "--keys 'space , tab' --values ' 11 , 12' --client my_midi --channel 0\n"
    "--keys '~ , F2' --values '11,12' --send-cc\n";

static int check_input(int argc, char *argv[], client_info_t *moc) {
  // returns zero on success
  moc->client_name = "kbd_midi";
  moc->show_key_pressed = 0;
  moc->velocity = 127;
  moc->send_cc = 0;
  char *keys = NULL;
  char *values = NULL;
  if (argc < 3) {
    fprintf(stderr, "%s", gHelpStr);
    return 1;
  }
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--test") == 0) {
      moc->show_key_pressed = 1;
    } else if (strcmp(argv[i], "--keys") == 0 && i + 1 < argc) {
      keys = argv[++i];
    } else if (strcmp(argv[i], "--values") == 0 && i + 1 < argc) {
      values = argv[++i];
    } else if (strcmp(argv[i], "--client") == 0 && i + 1 < argc) {
      moc->client_name = argv[++i];
    } else if (strcmp(argv[i], "--velocity") == 0 && i + 1 < argc) {
      moc->velocity = atoi(argv[++i]);
      if (moc->velocity < 1 || moc->velocity > 127) {
        fprintf(stderr, "Invalid MIDI velocity: %s\n", argv[i++]);
        return 1;
      }
    } else if (strcmp(argv[i], "--channel") == 0 && i + 1 < argc) {
      moc->channel = atoi(argv[++i]);
      if (moc->channel < 0 || moc->channel > 15) {
        fprintf(stderr, "Invalid MIDI channel: %s\n", argv[i++]);
        return 1;
      }
    } else if (strcmp(argv[i], "--send-cc") == 0) {
      moc->send_cc = 1;
    } else if (strcmp(argv[i], "--help") == 0) {
      fprintf(stderr, "%s", gHelpStr);
      return 1;
    } else {
      fprintf(stderr, "Unknown argument: %s\nFor help use: %s --help\n", argv[i], argv[0]);
      return 1;
    }
  }

  if (check_input_client_exists(moc->client_name)) {
    fprintf(stderr, "Quit because client already exists: %s\n", moc->client_name);
    return 1;
  }

  if (keys == NULL) {
    fprintf(stderr, "Missing --keys argument\n");
    fprintf(stderr, "%s", gHelpStr);
    return 1;
  }

  if (values == NULL) {
    fprintf(stderr, "Missing --values argument\n");
    fprintf(stderr, "%s", gHelpStr);
    return 1;
  }

  const size_t max_arr_size = 32;
  str_view_t k_arr[max_arr_size], v_arr[max_arr_size];

  int k_size = str_view_array_fill(k_arr, max_arr_size, str_view(keys, 512), STR_VIEW(","));
  if (k_size < 0) {
    fprintf(stderr, "Failed to split keys: %s\n", keys);
    return 1;
  }

  int v_size = str_view_array_fill(v_arr, max_arr_size, str_view(values, 512), STR_VIEW(","));
  if (v_size < 0) {
    fprintf(stderr, "Failed to split values: %s\n", values);
    return 1;
  }

  if (k_size != v_size) {
    fprintf(stderr, "Keys and values have different sizes\n");
    return 1;
  }
  for (int i = 0; i < k_size; i++) {
    str_view_t k = k_arr[i];
    str_view_t v1 = v_arr[i];
    int v = atoi(v1.data);
    kv_t kv = (kv_t){.key = k, .value = v};
    list_kv_add(&moc->list_kv, kv);
  }

  list_kv_reset_iter(&moc->list_kv);
  while (list_kv_has_next(&moc->list_kv)) {
    kv_t kv = list_kv_get_next(&moc->list_kv);
    str_buf_t buf = STR_BUF(32);
    str_buf_append(&buf, kv.key);
    fprintf(stderr, "key: %s, value: %d\n", buf.data, kv.value);
  }

  fprintf(stderr, "--velocity: %d\n", moc->velocity);
  fprintf(stderr, "--channel: %d\n", moc->channel);
  fprintf(stderr, "--send-cc: %d\n", moc->send_cc);

  return 0;
}

static int open_kbd(int *fd) {
  // returns zero on success

  const char *device_path = find_keyboard_device();
  if (device_path == NULL) {
    fprintf(stderr, "No keyboard device found\n");
    return 1;
  }

  *fd = open(device_path, O_RDONLY);
  if (*fd == -1) {
    fprintf(stderr, "Failed to open device: %s\n", device_path);
    return 1;
  }
  return 0;
}

void stop_kbd(int *fd, client_info_t *moc) {
  if (*fd > 0) {
    close(*fd);
  }
  snd_seq_close(moc->seq_handle);
  exit(0);
}

int main(int argc, char *argv[]) {

  client_info_t moc = {0}; // struct to keep MIDI client
  int fd;                  // kbd file descriptor
  int err = 0;
  if ((err = check_input(argc, argv, &moc)) != 0) {
    return err;
  }

  if ((err = open_kbd(&fd)) != 0) {
    return err;
  }

  if ((err = open_midi_out_port(&moc.seq_handle, &moc.port_id, moc.client_name)) != 0) {
    fprintf(stderr, "Failed to create MIDI OUT port: %s\n", moc.client_name);
    return 1;
  }

  char byte1 = 0;
  char byte2 = 0;

  while (1) {
    int result = read_kbd(&moc, &byte1, &byte2, fd);
    if (result == -1) {
      break; // ESC key or error
    }

    if (result == 1) {
      send_midi_event(moc, byte1, byte2);
    }
  }

  stop_kbd(&fd, &moc);
  return 0;
}

void start_kbd(char *keys, char *values) {

  char *argv[7];
  argv[0] = "kbd_midi_tsr";
  argv[1] = "--keys";
  argv[2] = keys;
  argv[3] = "--values";
  argv[4] = values;

  main(5, argv);
}
