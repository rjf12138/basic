#include "logger.h"

using namespace basic;

class MsgTest : public Logger {
public:
    MsgTest()
    {
        LOG_INFO("Start");
    }

    virtual ~MsgTest()
    {
        LOG_INFO("End");
    }

    void log_print(void)
    {
        LOG_INFO("print");
    }
};

void print_msg(void)
{
    for (int i = 0; i < 3; ++i) {
        LOG_GLOBAL_TRACE("Hello, world! - %d", i);
    }

    for (int i = 0; i < 3; ++i) {
        LOG_GLOBAL_DEBUG("Hello, world! - %d", i);
    }

    for (int i = 0; i < 3; ++i) {
        LOG_GLOBAL_INFO("Hello, world! - %d", i);
    }

    for (int i = 0; i < 3; ++i) {
        LOG_GLOBAL_WARN("Hello, world! - %d", i);
    }

    for (int i = 0; i < 3; ++i) {
        LOG_GLOBAL_ERROR("Hello, world! - %d", i);
    }

    for (int i = 0; i < 3; ++i) {
        LOG_GLOBAL_FATAL("Hello, world! - %d", i);
    }
}

int main(void)
{
    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_LOW);
    std::cout << "===============LOG_LEVEL_LOW=======================" << std::endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_TRACE);
    std::cout << "===============LOG_LEVEL_TRACE=======================" << std::endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_DEBUG);
    std::cout << "===============LOG_LEVEL_DEBUG=======================" << std::endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_INFO);
    std::cout << "===============LOG_LEVEL_INFO=======================" << std::endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_ERROR);
    std::cout << "===============LOG_LEVEL_ERROR=======================" << std::endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_FATAL);
    std::cout << "===============LOG_LEVEL_FATAL=======================" << std::endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_MAX);
    std::cout << "===============LOG_LEVEL_MAX=======================" << std::endl;
    print_msg();

    MsgTest m_test;
    m_test.log_print();

    return 0;
}