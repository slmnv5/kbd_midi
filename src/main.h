#ifndef KBD_APP_H
#define KBD_APP_H

#ifndef __linux__
#error "This app requires Linux"
#endif

#define DLL_PUBLIC __attribute__((visibility("default")))

DLL_PUBLIC void start_kbd(char *keys, char *values);
DLL_PUBLIC void stop_kbd();
DLL_PUBLIC int version() { return 20261021; };

#endif // KBD_APP_H
