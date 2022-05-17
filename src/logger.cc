#include "logger.h"

namespace basic {
//////////////////////////////////////////////
// 默认标准输出函数
int output_to_stdout(const std::string &msg)
{
    std::cout << msg << std::endl;

    return 0;
}

// 默认标准出错函数
int output_to_stderr(const std::string &msg)
{
    std::cerr << msg << std::endl;

    return 0;
}

// 设置颜色
std::string 
set_string_color(const std::string &str, StringColor color)
{
    std::string output_str;
    switch (color)
    {
        case StringColor_Blue:
        {
            output_str = "\033[34m";
        } break;
        case StringColor_Green:
        {
            output_str = "\033[32m";
        } break;
        case StringColor_Red: 
        {
            output_str = "\033[31m";
        } break;
        case StringColor_Yellow:
        {
            output_str = "\033[33m";
        } break;
        case StringColor_Cyan:
        {
            output_str = "\033[36m";
        } break;
        case StringColor_Magenta:
        {
            output_str = "\033[35m";
        } break;
        default:
            return str;
    }

    output_str += str + "\033[0m";

    return output_str;
}

Logger Logger::g_log_msg;

Logger::Logger(void)
: print_level_(LOG_LEVEL_LOW),
  msg_to_stream_trace_(output_to_stdout),
  msg_to_stream_debug_(output_to_stdout),
  msg_to_stream_info_(output_to_stdout),
  msg_to_stream_warn_(output_to_stderr),
  msg_to_stream_error_(output_to_stderr),
  msg_to_stream_fatal_(output_to_stderr)
{

}

Logger::~Logger(void)
{
}

void 
Logger::set_stream_func(InfoLevel level, msg_to_stream_callback func)
{
    switch (level) {
        case LOG_LEVEL_TRACE:
            msg_to_stream_trace_ = func; break;
        case LOG_LEVEL_DEBUG:
            msg_to_stream_debug_ = func; break;
        case LOG_LEVEL_INFO:
            msg_to_stream_info_ = func; break;
        case LOG_LEVEL_WARN:
            msg_to_stream_warn_ = func; break;
        case LOG_LEVEL_ERROR:
            msg_to_stream_error_ = func; break;
        case LOG_LEVEL_FATAL:
            msg_to_stream_fatal_ = func; break;
        default: 
        {
            output_to_stderr("Logger::set_stream_func: Unknown option!");
        } break;
    }
}

msg_to_stream_callback 
Logger::get_stream_func(InfoLevel level)
{
    switch (level) {
        case LOG_LEVEL_TRACE:
            return msg_to_stream_trace_;
        case LOG_LEVEL_DEBUG:
            return msg_to_stream_debug_ ;
        case LOG_LEVEL_INFO:
            return msg_to_stream_info_;
        case LOG_LEVEL_WARN:
            return msg_to_stream_warn_;
        case LOG_LEVEL_ERROR:
            return msg_to_stream_error_;
        case LOG_LEVEL_FATAL:
            return msg_to_stream_fatal_;
        default: 
        {
            output_to_stderr("Logger::set_stream_func: Unknown option!");
        } break;
    }

    return nullptr;
}

void 
Logger::set_print_level(InfoLevel level)
{
    print_level_ = level;
    
    return ;
}

void
Logger::print_msg(InfoLevel level, int line, std::string file_name, std::string func, const char *format, ...)
{
    if (level < print_level_) { // 低于 print_level 的将不会被输出
        return ;
    }

    char msg_buff[4096] = {0};

    va_list arg_ptr;
    va_start(arg_ptr,format);
    vsnprintf(msg_buff, 4096, format, arg_ptr);
    va_end(arg_ptr);

    char strtime[65] = {0};
    time_t curr_time = time(NULL);
    strftime(strtime, sizeof(strtime), "%Y-%m-%d %H:%M:%S", localtime(&curr_time));

    MsgContent tmp_msg;
    tmp_msg.info_level = level;
    tmp_msg.when = strtime;
    tmp_msg.msg_info = msg_buff;
    tmp_msg.msg_func = func;
    tmp_msg.which_line = line;
    tmp_msg.which_file = basename(file_name.c_str());

    msg_to_stream_callback output_callback = this->get_stream_func(level);

    std::ostringstream ostr; // 输出到终端的是带有颜色设置的
    if (output_callback == output_to_stderr || output_callback == output_to_stdout) {
        this->assemble_msg(ostr, tmp_msg, true);
    } else {
        this->assemble_msg(ostr, tmp_msg, false);
    }

    if (output_callback != nullptr) {
        output_callback(ostr.str());
    } else {
        output_to_stderr(set_string_color("Logger ouput callback function is nullptr!", StringColor_Yellow));
    }
    
    return ;
}

std::string 
Logger::get_msg(InfoLevel level, int line, std::string file_name, std::string func, const char *format, ...)
{
    char *msg_buff = new char[4096];
    memset(msg_buff, 0, 4096);

    va_list arg_ptr;
    va_start(arg_ptr,format);
    vsnprintf(msg_buff, 4096,format, arg_ptr);
    va_end(arg_ptr);

    char strtime[65] = {0};
    time_t curr_time = time(NULL);
    strftime(strtime, sizeof(strtime), "%Y-%m-%d %H:%M:%S", localtime(&curr_time));

    MsgContent tmp_msg;
    tmp_msg.info_level = level;
    tmp_msg.when = strtime;
    tmp_msg.msg_info = msg_buff;
    tmp_msg.msg_func = func;
    tmp_msg.which_line = line;
    tmp_msg.which_file = basename(file_name.c_str());

    std::ostringstream ostr;
    this->assemble_msg(ostr, tmp_msg, false);

    delete[] msg_buff;
    msg_buff = nullptr;

    return ostr.str();
}

void 
Logger::assemble_msg(std::ostringstream &ostr, const MsgContent &msg, bool is_color_enable)
{
    if (is_color_enable) {
        ostr << "\033[36m[" << msg.when << "]\033[0m";
        ostr << "\033[35m[" << msg.msg_func << "]";
        ostr << "[" << msg.which_file << ":" << msg.which_line << "]";
        ostr << "[" << level_convert(msg.info_level) << "\033[35m]:\033[0m ";

        switch (msg.info_level) {
            case LOG_LEVEL_TRACE:
                ostr << set_string_color(msg.msg_info, StringColor_None);
                break;
            case LOG_LEVEL_DEBUG:
                ostr << set_string_color(msg.msg_info, StringColor_Blue);
                break;
            case LOG_LEVEL_INFO:
                ostr << set_string_color(msg.msg_info, StringColor_Green);
                break;
            case LOG_LEVEL_WARN:
                ostr << set_string_color(msg.msg_info, StringColor_Yellow);
                break;
            case LOG_LEVEL_ERROR:
                ostr <<  set_string_color(msg.msg_info, StringColor_Red);
                break;
            case LOG_LEVEL_FATAL:
                ostr <<  set_string_color(msg.msg_info, StringColor_Red);
                break;
            default:
                ostr << "Unknown Log Level" << std::endl;
        }
    } else {
        ostr << "[" << msg.when << "]";
        ostr << "[" << msg.msg_func << "]";
        ostr << "[" << msg.which_file << ":" << msg.which_line << "]";
        ostr << "[" << level_convert(msg.info_level) << "]: ";
        ostr << msg.msg_info;
    }
    
    return ;
}

std::string 
Logger::level_convert(enum InfoLevel level)
{
    switch(level)
    {
        case LOG_LEVEL_TRACE:
            return "\033[32mTRACE\033[0m";
        case LOG_LEVEL_DEBUG:
            return "\033[32mDEBUG\033[0m";
        case LOG_LEVEL_INFO:
            return "\033[32mINFO\033[0m";
        case LOG_LEVEL_WARN:
            return "\033[33mWARN\033[0m";
        case LOG_LEVEL_ERROR:
            return "\033[31mERROR\033[0m";
        case LOG_LEVEL_FATAL:
            return "\033[31;1mFATAL\033[0m";
        default:
            return "Unknown Log Level";
    }

    return "Unknown Log Level";
}

}