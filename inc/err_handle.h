#ifndef __ERR_HANDLE_H__
#define __ERR_HANDLE_H__

#include <list>
#include <string>
#include <utility>

typedef void (*error_handle_callback)(int err_no, std::string err_record, void* data);

class ErrHandle {
public:
    ErrHandle(void);
    ~ErrHandle(void);

    // 获取最新的错误码
    int error_no(void);
    // 获取最新的错误记录
    std::string error_record(void);

    // 加入一条错误记录
    void add_err_record(int err_no, std::string err_record);

    // 增加错误回调，当错误发生时候进行处理
    void add_err_handle_callback(error_handle_callback callback);


private:
    error_handle_callback callback_;

    int max_err_count; // 最大能保存多少条错误记录
    std::list<std::pair<int, std::string>> err_list_;
};

#endif