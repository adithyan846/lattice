#include <main/version.h>

const char *app_version(void)
{
    return version_string();
}

const char *app_git_hash(void)
{
    return version_git_hash();
}
