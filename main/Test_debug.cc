#include "debug.h"
#include "logger.h"

using namespace basic;

void func_debug(void)
{
    LOG_GLOBAL_DEBUG("\n%s", basic::dump_stack().c_str());
}

int main()
{
    auto i = 0;
    ++i;
    func_debug();
    return 0;
}