#include "logger.h"

using namespace basic;

class MsgTest : public MsgRecord {
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
    cout << "===============LOG_LEVEL_LOW=======================" << endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_TRACE);
    cout << "===============LOG_LEVEL_TRACE=======================" << endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_DEBUG);
    cout << "===============LOG_LEVEL_DEBUG=======================" << endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_INFO);
    cout << "===============LOG_LEVEL_INFO=======================" << endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_ERROR);
    cout << "===============LOG_LEVEL_ERROR=======================" << endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_FATAL);
    cout << "===============LOG_LEVEL_FATAL=======================" << endl;
    print_msg();

    SET_GLOBAL_PRINT_LEVEL(LOG_LEVEL_MAX);
    cout << "===============LOG_LEVEL_MAX=======================" << endl;
    print_msg();

    MsgTest m_test;
    m_test.log_print();

    return 0;
}