#include "debug.h"
#include "logger.h"

namespace basic {
void dump_stack(void)
{
    int btnum = 0;
    void *btbuf[512];
    char **btstrings = NULL;
    int i;

    /* Get backtrace */
    btnum = backtrace(btbuf, 512);
    btstrings = backtrace_symbols(btbuf, btnum);
    if (btstrings == NULL) {
        LOG_GLOBAL_FATAL("Backtrace failed: %d:%s\n", errno, strerror(errno));
    } else {
        LOG_GLOBAL_FATAL("Backtraces, total %d items\n", btnum);
        for (i = 0; i < btnum; i++) {
            LOG_GLOBAL_FATAL("%s\n", btstrings[i]);
        }

        free(btstrings);
    }
}
}