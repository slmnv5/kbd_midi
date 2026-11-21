#define _GNU_SOURCE

#include <alsa/asoundlib.h>

#include "handy_string.h"
#include "midi_utils.h"
// #include <alloca.h>
// #include <alsa/seqmid.h>
// #include <alsa/asoundlib.h>

// Fallback if compilation environment has an older version of ALSA
#ifndef SND_SEQ_PORT_CAP_INACTIVE
#define SND_SEQ_PORT_CAP_INACTIVE (1 << 8)
#endif

void send_midi_event(client_info_t moc, char byte1, char byte2) {
  snd_seq_event_t ev;
  snd_seq_ev_clear(&ev);

  if (moc.send_cc) {
    snd_seq_ev_set_controller(&ev, moc.channel, byte1, byte2);
  } else {
    if (byte2)
      snd_seq_ev_set_noteon(&ev, moc.channel, byte1, byte2);
    else
      snd_seq_ev_set_noteoff(&ev, moc.channel, byte1, byte2);
  }

  snd_seq_ev_set_source(&ev, moc.port_id);
  snd_seq_ev_set_subs(&ev);
  snd_seq_ev_set_direct(&ev);
  snd_seq_event_output(moc.seq_handle, &ev);
  snd_seq_drain_output(moc.seq_handle);
}

int open_midi_out_port(snd_seq_t **seq_handle, int *port_id, const char *name) {
  // returns zero on success
  if (snd_seq_open(seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    return 1;
  }

  snd_seq_set_client_name(*seq_handle, name);

  //  Create a port for writing MIDI IN for external devices
  *port_id = snd_seq_create_simple_port(
      *seq_handle, "out", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
      SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_SOFTWARE);

  if (port_id < 0) {
    fprintf(stderr, "Error creating MIDI port.\n");
    snd_seq_close(*seq_handle);
    return 1;
  }

  fprintf(stderr, "Created MIDI client: %s, client id: %d, port id: %d\n", name,
          snd_seq_client_id(*seq_handle), *port_id);
  return 0;
}

int check_input_client_exists(const char *partOfName) {
  // returns zero on success
  if (!partOfName) return 0;

  snd_seq_t *seq_handle;
  snd_seq_client_info_t *client_info;
  snd_seq_port_info_t *port_info;
  int is_fount = 0;

  if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    return -1;
  }

  snd_seq_client_info_alloca(&client_info);
  snd_seq_port_info_alloca(&port_info);

  snd_seq_client_info_set_client(client_info, -1);

  while (snd_seq_query_next_client(seq_handle, client_info) >= 0) {
    const char *client_name = snd_seq_client_info_get_name(client_info);

    if (client_name && strstr(client_name, partOfName) != NULL) {
      int client_id = snd_seq_client_info_get_client(client_info);
      // Client matched the name, query its internal ports.
      snd_seq_port_info_set_client(port_info, client_id);
      snd_seq_port_info_set_port(port_info, -1);

      while (snd_seq_query_next_port(seq_handle, port_info) >= 0) {
        unsigned int poer_capabilities = snd_seq_port_info_get_capability(port_info);
        unsigned int portType = snd_seq_port_info_get_type(port_info);

        // Verify port emits MIDI, verify it's a hardware
        if ((poer_capabilities & SND_SEQ_PORT_CAP_READ) &&
            (poer_capabilities & SND_SEQ_PORT_CAP_SUBS_READ) &&
            !(portType & SND_SEQ_PORT_CAP_INACTIVE)) {
          is_fount = 1;
          break;
        }
      }
    }
    if (is_fount) {
      break;
    }
  }

  snd_seq_close(seq_handle);
  return is_fount;
}
