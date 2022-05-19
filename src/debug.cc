#include "debug.h"
#include "logger.h"

namespace basic {
std::string dump_stack(void)
{
    int btnum = 0;
    void *btbuf[512];
    char **btstrings = NULL;
    int i;

    /* Get backtrace */
    std::ostringstream ostr;
    btnum = backtrace(btbuf, 512);
    btstrings = backtrace_symbols(btbuf, btnum);
    if (btstrings == NULL) {
        ostr << GLOBAL_GET_MSG(LOG_LEVEL_DEBUG, "Backtrace failed: %d:%s\n", errno, strerror(errno));
    } else {
        ostr << GLOBAL_GET_MSG(LOG_LEVEL_DEBUG, "Backtraces, total %d items\n", btnum);
        for (i = 0; i < btnum; i++) {
            ostr << GLOBAL_GET_MSG(LOG_LEVEL_DEBUG, "%s\n", btstrings[i]);
        }

        free(btstrings);
    }

    return ostr.str();
}
}