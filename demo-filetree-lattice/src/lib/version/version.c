#include <lib/version/version.h>

const char *version_string(void)
{
    return FREERTRTOS_VERSION_STRING;
}

const char *version_git_hash(void)
{
    return FREERTRTOS_GIT_HASH;
}
