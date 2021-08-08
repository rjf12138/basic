#### Logger

```
// 继承 Logger 的类可以在类内直接调用下面的函数
// 设置输出等级，低于该等级的日志将不会被输出
#define SET_PRINT_LEVEL(x)  this->set_print_level(x)
// 根据日志等级输出日志
#define LOG_TRACE(...)      this->print_msg(LOG_LEVEL_TRACE, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LOG_DEBUG(...)      this->print_msg(LOG_LEVEL_DEBUG, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...)       this->print_msg(LOG_LEVEL_INFO, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LOG_WARN(...)       this->print_msg(LOG_LEVEL_WARN, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...)      this->print_msg(LOG_LEVEL_ERROR, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LOG_FATAL(...)      this->print_msg(LOG_LEVEL_FATAL, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)

// 设置对应等级的输出函数。函数格式： typedef int (*msg_to_stream_callback)(const string &);
#define SET_CALLBACK(LEVEL, FUNC) this->set_stream_func(LEVEL, FUNC)
// 返回日志字符串消息
#define GET_MSG(LEVEL, ...)  this->get_msg(LEVEL, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
```

```
// 全局的日志输出函数，哪里都可以使用
// 设置输出等级，低于该等级的日志将不会被输出
#define SET_GLOBAL_PRINT_LEVEL(x) Logger::g_log_msg.set_print_level(x)

// 根据日志等级输出日志
#define LOG_GLOBAL_TRACE(...)  Logger::g_log_msg.print_msg(LOG_LEVEL_TRACE, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LOG_GLOBAL_DEBUG(...)  Logger::g_log_msg.print_msg(LOG_LEVEL_DEBUG, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LOG_GLOBAL_INFO(...)   Logger::g_log_msg.print_msg(LOG_LEVEL_INFO, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LOG_GLOBAL_WARN(...)   Logger::g_log_msg.print_msg(LOG_LEVEL_WARN, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LOG_GLOBAL_ERROR(...)  Logger::g_log_msg.print_msg(LOG_LEVEL_ERROR, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LOG_GLOBAL_FATAL(...)  Logger::g_log_msg.print_msg(LOG_LEVEL_FATAL, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)

// 设置对应等级的输出函数。函数格式： typedef int (*msg_to_stream_callback)(const string &);
#define SET_GLOBAL_CALLBACK(LEVEL, FUNC)  Logger::g_log_msg.set_stream_func(LEVEL, FUNC)
// 返回日志字符串消息
#define GLOBAL_GET_MSG(LEVEL, ...)  Logger::g_log_msg.get_msg(LEVEL, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
```