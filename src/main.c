#include "kuksa_client.h"

#include <stdio.h>
#include <string.h>

static void usage(const char *prog) {
  fprintf(stderr,
          "Usage:\n"
          "  %s get <host:port> <path> [token]\n"
          "  %s set <host:port> <path> <value> [token]\n",
          prog,
          prog);
}

int main(int argc, char **argv) {
  if (argc < 4) {
    usage(argv[0]);
    return 1;
  }

  const char *cmd = argv[1];
  const char *target = argv[2];

  if (strcmp(cmd, "get") == 0) {
    const char *path = argv[3];
    const char *token = argc > 4 ? argv[4] : "";
    return kuksa_get_current_value(target, token, path);
  }

  if (strcmp(cmd, "set") == 0) {
    if (argc < 5) {
      usage(argv[0]);
      return 1;
    }
    const char *path = argv[3];
    const char *value = argv[4];
    const char *token = argc > 5 ? argv[5] : "";
    return kuksa_set_current_value(target, token, path, value);
  }

  usage(argv[0]);
  return 1;
}
