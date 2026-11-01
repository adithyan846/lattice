#ifndef MAIN_VERSION_H
#define MAIN_VERSION_H

#include <lib/version/version.h>

#define APP_VERSION FREERTRTOS_VERSION_STRING

const char *app_version(void);
const char *app_git_hash(void);

#endif /* MAIN_VERSION_H */
