#include "kbd_utils.h"
#include "handy_string.h"
#include "kbd_codes.h"

#define BITS_PER_LONG (8 * sizeof(long))
#define NBITS(x) ((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x) ((x) % BITS_PER_LONG)
#define BIT(x) (1UL << OFF(x))
#define LONG(x) ((x) / BITS_PER_LONG)
#define IS_SET(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)
#define BUF_SIZE 256

const char *find_keyboard_device() {
  char *path = malloc(BUF_SIZE);
  char name[BUF_SIZE] = "Unknown";
  unsigned long bitmask[NBITS(KEY_MAX)];

  for (int i = 0; i < 32; i++) {
    snprintf(path, BUF_SIZE, "/dev/input/event%d", i);

    int fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
      continue;
    }

    memset(bitmask, 0, sizeof(bitmask));
    if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bitmask)), bitmask) >= 0) {
      if (IS_SET(KEY_A, bitmask)) {
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);
        fprintf(stderr, "Keyboard found: %s, name: %s\n", path, name);
        close(fd);
        return path;
      }
    }
    close(fd);
  }
  free(path);
  return NULL;
}

int read_kbd(client_info_t *moc, char *byte1, char *byte2, int fd) {
  struct input_event ev;

  ssize_t bytes_read = read(fd, &ev, sizeof(ev));

  if (bytes_read < (ssize_t)sizeof(ev)) {
    if (bytes_read == 0) return 0; // EOF / Device disconnected
    if (errno == EINTR) return 0;  // Interrupted by system signal, retry later
    perror("Error reading event");
    return -1;
  }

  if (!(ev.type == EV_KEY && (ev.value == 0 || ev.value == 1))) return 0;

  if (ev.code == 1) {
    printf("Esc pressed, stop monitoring keyboard\n");
    return -1;
  }

  const char *key_name = getKeyName(ev.code);
  list_kv_reset_iter(&moc->list_kv);
  while (list_kv_has_next(&moc->list_kv)) {
    kv_t kv = list_kv_get_next(&moc->list_kv);
    if (!strncmp(kv.key.data, key_name, kv.key.len) == 0) continue;

    *byte1 = kv.value;
    *byte2 = (ev.value == 1) ? moc->velocity : 0; // Press = velocity, Release = 0

    if (moc->show_key_pressed) {
      fprintf(stdout, "Pressed key: %s, value: %d\n", key_name, *byte1);
    }
    return 1; // Successfully populated MIDI
  }

  if (moc->show_key_pressed) {
    fprintf(stdout, "Pressed key: %s, value: None\n", key_name);
  }

  return 0; // no key mapping
}
