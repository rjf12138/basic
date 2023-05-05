#include "err_handle.h"

namespace basic {

ErrHandle::ErrHandle(void)
:err_code_(0),
callback_(nullptr)
{

}

ErrHandle::~ErrHandle(void)
{

}

void 
ErrHandle::set_err_code(int err_code, void* data, const std::string &str_error)
{
    err_code_ = err_code;

    if (callback_ != nullptr) {
        callback_(err_code_, data, str_error);
    }
}

int 
ErrHandle::new_errno(void)
{
    return err_code_;
}

void 
ErrHandle::add_err_handle_callback(error_handle_callback callback)
{
    callback_ = callback;
}


}