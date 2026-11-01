#ifndef LIB_VERSION_VERSION_H
#define LIB_VERSION_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define FREERTRTOS_VERSION_MAJOR 0
#define FREERTRTOS_VERSION_MINOR 1
#define FREERTRTOS_VERSION_PATCH 0
#define FREERTRTOS_VERSION_STRING "0.1.0-scaffold"

#ifndef FREERTRTOS_GIT_HASH
#define FREERTRTOS_GIT_HASH "unknown"
#endif

const char *version_string(void);
const char *version_git_hash(void);

#ifdef __cplusplus
}
#endif

#endif /* LIB_VERSION_VERSION_H */
