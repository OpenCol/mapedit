/* fileio.c -- fileio_c.ASM's fileio_exist, and env_1.c's env_exist, which only jumps to it.
 *
 * fileio_exist builds the data file's path (env_get_path) and opens it for reading, with its
 * own critical-error handler in place so a missing floppy fails instead of asking Abort,
 * Retry, Fail; -1 when it opened. */
#include "msc.h"

char *env_get_path(char *dst, char *name);

i16 fileio_exist(char *name)
{
    char copy[80], path[0x51];
    i16 h;
    u16 i;
    for (i = 0; i < sizeof copy - 1 && name[i]; i++)
        copy[i] = name[i];
    copy[i] = 0;
    env_get_path(path, copy);
    h = msc_open(path, 0);
    if (h < 0)
        return 0;
    msc_close(h);
    return -1;
}

/* env_exist -- 0297:0142 */
i16 env_exist(char *name)
{
    return fileio_exist(name);
}
