#ifndef KUKSA_CLIENT_H
#define KUKSA_CLIENT_H

int kuksa_get_current_value(const char *target, const char *token, const char *path);
int kuksa_set_current_value(const char *target, const char *token, const char *path, const char *value);

#endif
