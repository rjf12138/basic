#include "byte_buffer.h"
#include "logger.h"
#include "debug.h"

namespace basic {

ByteBuffer::ByteBuffer(ssize_t size)
: buffer_(nullptr),
  start_read_pos_(0), 
  start_write_pos_(0), 
  used_data_size_(0),
  free_data_size_(0),
  max_buffer_size_(0)
{
    if (size <= 0)
    {
        max_buffer_size_ = 0;
        free_data_size_ = 0;
        buffer_ = nullptr;
    }
    else
    {
        max_buffer_size_ = 2 * size;
        if (max_buffer_size_ >= MAX_BUFFER_SIZE) {
            max_buffer_size_ = MAX_BUFFER_SIZE;
        }

        free_data_size_ = max_buffer_size_ - 1;
        buffer_ = new bufftype[max_buffer_size_];
    }
}

ByteBuffer::ByteBuffer(const ByteBuffer &buff)
: buffer_(nullptr),
  start_read_pos_(0), 
  start_write_pos_(0), 
  used_data_size_(0),
  free_data_size_(0),
  max_buffer_size_(0)
{
    start_read_pos_ = buff.start_read_pos_;
    start_write_pos_ = buff.start_write_pos_;
    used_data_size_ = buff.used_data_size_;
    free_data_size_ = buff.free_data_size_;
    max_buffer_size_ = buff.max_buffer_size_;

    if (buff.buffer_ != nullptr && buff.max_buffer_size_ > 0) {
        buffer_ = new bufftype[buff.max_buffer_size_];
        memmove(buffer_, buff.buffer_, buff.max_buffer_size_);
    } else {
        this->clear();
    }
}

ByteBuffer::ByteBuffer(const std::string &str)
: buffer_(nullptr),
  start_read_pos_(0), 
  start_write_pos_(0), 
  used_data_size_(0),
  free_data_size_(0),
  max_buffer_size_(0)
{
    this->write_string(str);
}

ByteBuffer::ByteBuffer(const buffptr data, ssize_t size)
: buffer_(nullptr),
  start_read_pos_(0), 
  start_write_pos_(0), 
  used_data_size_(0),
  free_data_size_(0),
  max_buffer_size_(0)
{
    this->write_bytes(data, size);
}

ByteBuffer::~ByteBuffer()
{
    this->clear();
}

ssize_t ByteBuffer::clear(void)
{
    if (buffer_ != nullptr) {
        delete[] buffer_;
        buffer_ = nullptr;
    }

    used_data_size_ = 0;
    free_data_size_ = 0;
    start_read_pos_ = 0;
    start_write_pos_ = 0;
    max_buffer_size_ = 0;

    return 0;
}

ssize_t ByteBuffer::set_extern_buffer(buffptr exbuf, ssize_t buff_size)
{
    if (exbuf == nullptr || buff_size <= 0) {
        return 0;
    }

    this->clear();
    max_buffer_size_ = buff_size;
    free_data_size_ = max_buffer_size_ - 1;
    buffer_ = exbuf;

    return buff_size;
}

void ByteBuffer::next_read_pos(int offset)
{
    start_read_pos_ = (start_read_pos_ + offset) % max_buffer_size_;
}

void ByteBuffer::next_write_pos(int offset)
{
    
    start_write_pos_ = (start_write_pos_ + offset) % max_buffer_size_;
}

ssize_t ByteBuffer::data_size(void) const
{
    return used_data_size_;
}

ssize_t ByteBuffer::idle_size() const 
{
    // -1 是为了留出一位，防止写满和为空的时候，
    // start_write和start_read都指向同一个位置，无法辨认
    return free_data_size_;
}

ssize_t 
ByteBuffer::resize(ssize_t size)
{
    // 重新分配的空间不能比当前小
    if (size < 0 || size <= max_buffer_size_)
    {
        return 0;
    }

    ssize_t new_size = 2 * size;
    if (new_size > MAX_BUFFER_SIZE) {
        new_size = MAX_BUFFER_SIZE;
    }

    ssize_t tmp_buffer_size = this->data_size();
    buffptr tmp_buffer = nullptr;
    if (tmp_buffer_size > 0) {
        tmp_buffer = new bufftype[tmp_buffer_size];
        this->read_bytes(tmp_buffer, tmp_buffer_size);
    }

    buffptr new_buffer = new bufftype[new_size];
    this->set_extern_buffer(new_buffer, new_size);
    if (tmp_buffer_size > 0) {
        this->write_bytes(tmp_buffer, tmp_buffer_size);
        delete[] tmp_buffer;
    }

    return max_buffer_size_;
}


bool ByteBuffer::empty(void) const
{
    
    return this->data_size() == 0 ? true : false;
}

ByteBuffer::iterator
ByteBuffer::begin(void)
{
    return iterator(this, start_read_pos_);
}

ByteBuffer::iterator
ByteBuffer::end(void)
{
    return iterator(this, start_write_pos_);
}

ByteBuffer::iterator
ByteBuffer::last_data(void)
{
    ByteBufferIterator tmp(this, start_read_pos_);
    if (this->data_size() <= 0) {
        return this->end();
    }

    return (tmp + (this->data_size() - 1));
}

ByteBuffer::const_iterator
ByteBuffer::begin(void) const
{
    return iterator(this, start_read_pos_);
}

ByteBuffer::const_iterator
ByteBuffer::end(void) const
{
    return iterator(this, start_write_pos_);
}

ByteBuffer::const_iterator
ByteBuffer::last_data(void) const
{
    iterator tmp(this, start_read_pos_);
    if (this->data_size() <= 0) {
        return this->end();
    }

    return (tmp + (this->data_size() - 1));
}

ssize_t ByteBuffer::copy_data_to_buffer(const void *data, ssize_t size)
{
    if (data == nullptr || size <= 0) {
        return 0;
    }

    if (this->idle_size() <= size) {
        ssize_t ret = this->resize(max_buffer_size_ + size);
        if (ret == -1) {
           return 0;
        }
    }

    // if (this->idle_size() < size) {
    //     fprintf(stderr, "ByteBuffer remain idle space(%ld) is less than size(%ld)!\n", this->idle_size(), size);
    //     return 0;
    // }

    ssize_t copy_size = size;
    buffptr data_ptr = reinterpret_cast<buffptr>(const_cast<void*>(data));
    
    // 检查buff数组后面是否有连续的内存可以写
    while (true)
    {
        ssize_t write_size = this->get_cont_write_size() > copy_size ? copy_size : this->get_cont_write_size();
        memmove(this->get_write_buffer_ptr(), data_ptr, write_size);
        this->update_write_pos(write_size);
        data_ptr = data_ptr + write_size;

        copy_size -= write_size;
        if (copy_size <= 0 || this->idle_size() == 0) {
            break;
        }
    }
    

    return size - copy_size;
}

ssize_t ByteBuffer::copy_data_from_buffer(void *data, ssize_t size)
{
    if (data == nullptr  || size <= 0) {
        return 0;
    }
    
    ssize_t copy_size = size;
    buffptr data_ptr = static_cast<buffptr>(data);
    
    // 检查buff数组后面是否有连续的内存可以读
    while (true)
    {
        ssize_t read_size = this->get_cont_read_size() > copy_size ? copy_size : this->get_cont_read_size();
        memmove(data_ptr, this->get_read_buffer_ptr(), read_size);
        this->update_read_pos(read_size);
        data_ptr = data_ptr + read_size;
        
        copy_size -= read_size;
        if (copy_size <= 0 || this->data_size() == 0) {
            break;
        }
    }

    return size - copy_size;
}

ssize_t
ByteBuffer::read_int8(int8_t &val)
{
    return this->copy_data_from_buffer(&val, sizeof(int8_t));
}

ssize_t
ByteBuffer::read_int16(int16_t &val)
{
    return this->copy_data_from_buffer(&val, sizeof(int16_t));
}

ssize_t
ByteBuffer::read_int32(int32_t &val)
{
    return this->copy_data_from_buffer(&val, sizeof(int32_t));
}

ssize_t
ByteBuffer::read_int64(int64_t &val)
{
    return this->copy_data_from_buffer(&val, sizeof(ssize_t));
}

// 字符串是以 ‘\0’ 结尾的
ssize_t
ByteBuffer::read_string(std::string &str, ssize_t str_size)
{
    if (this->empty()) {
        return 0;
    }

    if (str_size == -1) {
        str_size = this->data_size();
    }

    char *str_ptr = new char[str_size + 1];
    ssize_t ret =  this->copy_data_from_buffer(str_ptr, str_size);
    if (ret == 0) {
        delete[] str_ptr;
        return 0;
    }
    str_ptr[str_size] = '\0';
    str = str_ptr;
    delete[] str_ptr;

    return str.length();
}

ssize_t 
ByteBuffer::read_bytes(void *buf, ssize_t buf_size)
{
    return this->copy_data_from_buffer(buf, buf_size);
}

ssize_t 
ByteBuffer::read_only(ssize_t start_pos, void *buf, ssize_t buf_size)
{
    if (buf == nullptr) {
        return 0;
    }

    ssize_t old_read_pos = this->start_read_pos_;
    ssize_t old_data_size = this->used_data_size_;
    ssize_t old_free_size = this->free_data_size_;

    ssize_t ret = this->update_read_pos(start_pos);
    if (ret == -1) {
        return 0;
    }
    ret = this->copy_data_from_buffer(buf, buf_size);

    this->start_read_pos_ = old_read_pos;
    this->used_data_size_ = old_data_size;
    this->free_data_size_ = old_free_size;

    return ret;
}

std::string 
ByteBuffer::str()
{
    bufftype *buffer = new bufftype[data_size() + 1];
    memset(buffer, 0, data_size() + 1);
    read_only(0, buffer, data_size());
    std::string str = buffer;
    delete[] buffer;
    return str;
}

ssize_t
ByteBuffer::write_int8(int8_t val)
{
    return this->copy_data_to_buffer(&val, sizeof(int8_t));
}

ssize_t
ByteBuffer::write_int16(int16_t val)
{
    return this->copy_data_to_buffer(&val, sizeof(int16_t));
}

ssize_t
ByteBuffer::write_int32(int32_t val)
{
    return this->copy_data_to_buffer(&val, sizeof(int32_t));
}

ssize_t
ByteBuffer::write_int64(int64_t val)
{
    return this->copy_data_to_buffer(&val, sizeof(ssize_t));
}

ssize_t
ByteBuffer::write_string(const std::string &str, ssize_t str_size)
{
    return this->copy_data_to_buffer(str.c_str(), str.length());
}

ssize_t ByteBuffer::write_bytes(const void *buf, ssize_t buf_size)
{
    return this->copy_data_to_buffer(buf, buf_size);
}

ssize_t
ByteBuffer::get_data(ByteBuffer &out, ByteBufferIterator &copy_start, ssize_t copy_size)
{
    if (this->buffer_ == nullptr || copy_size <= 0) {
        return 0;
    }

    if (copy_start.buff_->buffer_ != this->buffer_) {
        return 0;
    }

    if (copy_start == end()) {
        return 0;
    }

    ssize_t i = 0;
    ByteBufferIterator tmp = copy_start;
    for (; i < copy_size && tmp != this->end(); ++i) {
        out.write_int8(*tmp);
        ++tmp;
    }

    return i;
}

//////////////////////// 重载操作符 /////////////////////////

ByteBuffer& 
ByteBuffer::operator+(const ByteBuffer &rhs)
{
    ssize_t rhs_read_size = rhs.get_cont_read_size();
    buffptr rhs_read_ptr = rhs.get_read_buffer_ptr();

    this->copy_data_to_buffer(rhs_read_ptr, rhs_read_size);
    if (rhs_read_size < rhs.data_size()) {
        rhs_read_size = rhs.get_cont_read_size();
        rhs_read_ptr = rhs.get_read_buffer_ptr();

        this->copy_data_to_buffer(rhs_read_ptr, rhs_read_size);
    }

    return *this;
}

ByteBuffer& 
ByteBuffer::operator+=(const ByteBuffer &rhs)
{
    ssize_t rhs_read_size = rhs.get_cont_read_size();
    buffptr rhs_read_ptr = rhs.get_read_buffer_ptr();

    this->copy_data_to_buffer(rhs_read_ptr, rhs_read_size);
    if (rhs_read_size < rhs.data_size()) {
        rhs_read_size = rhs.get_cont_read_size();
        rhs_read_ptr = rhs.get_read_buffer_ptr();

        this->copy_data_to_buffer(rhs_read_ptr, rhs_read_size);
    }

    return *this;
}

bool 
ByteBuffer::operator==(const ByteBuffer &rhs) const
{
    if (this->data_size() != rhs.data_size()) {
        return false;
    }

    auto lhs_iter = this->begin();
    auto rhs_iter = rhs.begin();

    while (true) {
        if (lhs_iter == this->end() && rhs_iter == rhs.end()) {
            return true;
        } else if (lhs_iter != this->end() && rhs_iter == rhs.end()) {
            return false;
        } else if (lhs_iter == this->end() && rhs_iter != rhs.end()) {
            return false;
        }

        if (*lhs_iter != *rhs_iter) {
            return false;
        }

        ++lhs_iter;
        ++rhs_iter;
    }

    return false;
}

bool 
ByteBuffer::operator!=(const ByteBuffer &rhs) const
{
    return !(*this == rhs);
}

ByteBuffer& 
ByteBuffer::operator=(const ByteBuffer& src)
{
    if (src == *this) { // 当赋值对象是自己时，直接返回
        return *this;
    }
    this->clear();

    start_read_pos_ = src.start_read_pos_;
    start_write_pos_ = src.start_write_pos_;
    used_data_size_ = src.used_data_size_;
    free_data_size_ = src.free_data_size_;
    max_buffer_size_ = src.max_buffer_size_;

    if (src.buffer_ != nullptr && src.max_buffer_size_ > 0) {
        buffer_ = new bufftype[src.max_buffer_size_];
        memmove(buffer_, src.buffer_, src.max_buffer_size_);
    } else {
        this->clear();
    }
    return *this;
}

bufftype& 
ByteBuffer::operator[](ssize_t index)
{
    ssize_t size = this->data_size();
    if (size <= 0 || index >= size) {
        dump_stack();
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR, "Out of range.[index: %d]", index));
    }

    index = (this->start_read_pos_ + index) %  max_buffer_size_;

    return buffer_[index];
}

bufftype& 
ByteBuffer::operator[](const ssize_t &index) const
{
    ssize_t size = this->data_size();
    if (size <= 0 || index >= size) {
        dump_stack();
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR, "Out of range.[index: %d]", index));
    }

    ssize_t new_index = (this->start_read_pos_ + index) %  max_buffer_size_;

    return buffer_[new_index];
}

bool 
ByteBuffer::bytecmp(ByteBufferIterator &iter, ByteBuffer &patten, ssize_t size)
{
    if (iter.buff_->buffer_ != this->buffer_ || iter == this->end())
    {
        return false;
    }

    ssize_t max_size = (size == -1 ? patten.data_size() : size);
    ssize_t index = 0;
    for (auto tmp_iter = iter; tmp_iter != this->end() && index < max_size; ++tmp_iter, ++index) {
        if (*tmp_iter != patten[index]) {
            return false;
        }
    }

    if (index == max_size) {
        return true;
    }

    return false;
}

buffptr 
ByteBuffer::get_write_buffer_ptr(void) const
{
    return buffer_ != nullptr ? buffer_ + start_write_pos_ : nullptr;
}

buffptr 
ByteBuffer::get_read_buffer_ptr(void) const
{
    return buffer_ != nullptr ? buffer_ + start_read_pos_ : nullptr;
}

ssize_t 
ByteBuffer::get_cont_write_size(void) const
{
    if (free_data_size_ <= 0) {
        return 0;
    }

    if (start_read_pos_ > start_write_pos_) {
        return free_data_size_;
    } else if (start_read_pos_ <= start_write_pos_) {
        if (start_read_pos_ == 0) {
            return free_data_size_;
        } else {
            return max_buffer_size_ - start_write_pos_;
        }
    }

    return 0;
}

ssize_t 
ByteBuffer::get_cont_read_size(void) const
{
    if (used_data_size_ <= 0) {
        return 0;
    }

    if (start_read_pos_ > start_write_pos_) {
        return max_buffer_size_ - start_read_pos_;
    } else if (start_write_pos_ > start_read_pos_) {
        return used_data_size_;
    }

    return 0;
}

ssize_t 
ByteBuffer::update_write_pos(ssize_t offset)
{
    if (offset < 0 || offset > free_data_size_) {
        return -1;
    }

    used_data_size_ += offset;
    free_data_size_ -= offset;
    start_write_pos_ = (start_write_pos_ + offset) % max_buffer_size_;

    return 0;
}

ssize_t 
ByteBuffer::update_read_pos(ssize_t offset)
{
    if (offset < 0 || offset > used_data_size_) {
        return -1;
    }

    used_data_size_ -= offset;
    free_data_size_ += offset;
    start_read_pos_ = (start_read_pos_ + offset) % max_buffer_size_;

    return 0;
}

///////////////////////////// 操作 ByteBuffer /////////////////////////////
std::vector<ByteBufferIterator>
ByteBuffer::find(const ByteBuffer &patten)
{
    std::vector<ByteBufferIterator> result;
    if (patten.data_size() == 0 || this->data_size() == 0) {
        return result;
    }

    int patten_index = 0;
    for (ssize_t i = 0; i < this->data_size(); ++i) {
        if (patten[patten_index] == (*this)[i]) {
            for (ssize_t j = i; j < this->data_size();) {
                if (patten_index < patten.data_size() && patten[patten_index] == (*this)[j]) {
                    ++patten_index;
                    ++j;
                } else {
                    break;
                }
            }
            
            if (patten_index == patten.data_size()) {
                result.push_back(this->begin() + i);
                i += patten.data_size();
            }
            patten_index = 0;
        }
    }

    return result;
}

std::vector<ByteBuffer> 
ByteBuffer::split(const ByteBuffer &buff)
{
    std::vector<ByteBuffer> result;
    if (buff.data_size() <= 0 || this->data_size() <= 0) {
        result.push_back(*this);
        return result;
    }

    std::vector<ByteBufferIterator> find_buff = this->find(buff);

    ByteBuffer tmp;
    ssize_t copy_size;
    ByteBufferIterator start_copy_pos = this->begin();
    for (std::size_t i = 0; i < find_buff.size(); ++i) {
        copy_size = find_buff[i] - start_copy_pos;

        this->get_data(tmp, start_copy_pos, copy_size);
        start_copy_pos = find_buff[i] + buff.data_size();
        
        if (tmp.data_size() > 0) {
            result.push_back(tmp);
            tmp.clear();
        }
    }

    copy_size = (this->end() - start_copy_pos); // 保存剩余的字符
    if (copy_size > 0) {
        this->get_data(tmp, start_copy_pos, copy_size);
        if (tmp.data_size() > 0) {
            result.push_back(tmp);
            tmp.clear();
        }
    }

    return result;
}


ByteBuffer 
ByteBuffer::replace(ByteBuffer buf1, const ByteBuffer &buf2, ssize_t index)
{
    if (buf1.data_size() <= 0 || this->data_size() <= 0) {
        return *this;
    }

    ssize_t copy_size = 0;
    ByteBuffer result, tmp;
    ByteBufferIterator copy_pos_iter = this->begin();
    std::vector<ByteBufferIterator> find_buff = this->find(buf1);
    if (find_buff.size() == 0) {
        return *this;
    }

    if (index >= static_cast<ssize_t>(find_buff.size()) || index < 0) {
        return *this;
    }
    
    for (std::size_t i = index; i < find_buff.size(); ++i) {
        copy_size = find_buff[i] - copy_pos_iter;
        if (copy_size > 0) {
            this->get_data(tmp, copy_pos_iter, copy_size);
            result += tmp;
            result += buf2;
            copy_pos_iter = find_buff[i] + buf1.data_size();
            tmp.clear();
        }
    }

    copy_size = this->end() - copy_pos_iter; // 保存剩余的字符
    if (copy_size > 0) {
        this->get_data(tmp, copy_pos_iter, copy_size);
        result += tmp;
        tmp.clear();
    }

    return result;
}


ByteBuffer 
ByteBuffer::remove(const ByteBuffer &buff, ssize_t index)
{
    if (buff.data_size() <= 0 || this->data_size() <= 0) {
        return *this;
    }
    
    ByteBuffer tmp_buf;
    std::vector<ByteBufferIterator> find_buff = this->find(buff);
    if (index < 0 || index >= static_cast<ssize_t>(find_buff.size())) {
        index = -1;
    }

    if (index == -1) {
        std::vector<ByteBuffer> ret = this->split(buff);
        for (std::size_t i = 0; i < ret.size(); ++i) {
            tmp_buf = tmp_buf + ret[i];
        }
    } else {
        ByteBufferIterator begin_iter = this->begin();
        ssize_t copy_size = find_buff[index] - begin_iter;

        ByteBuffer out;
        this->get_data(out, begin_iter, copy_size);
        tmp_buf = tmp_buf + out;

        find_buff[index] = find_buff[index] + buff.data_size();
        copy_size = this->end() - find_buff[index] - 1;
        this->get_data(out, find_buff[index], copy_size);
        tmp_buf = tmp_buf + out;
    }

    *this = tmp_buf;

    return tmp_buf;
}

ssize_t 
ByteBuffer::insert_front(ByteBufferIterator &insert_iter, const ByteBuffer &buff)
{
    ByteBuffer tmp_buf, result;
    if (!(insert_iter >= this->begin() && 
            insert_iter < this->end())) {
        return -1;
    }

    ByteBufferIterator begin_iter = this->begin();
    ssize_t copy_front_size = insert_iter - begin_iter;
    this->get_data(result, begin_iter, copy_front_size);
    
    result = result + buff;
    copy_front_size = this->end() - insert_iter;
    this->get_data(tmp_buf, insert_iter, copy_front_size);
    result = result + tmp_buf;

    *this = result;

    return 0;
}

ssize_t 
ByteBuffer::insert_back(ByteBufferIterator &insert_iter, const ByteBuffer &buff)
{
    ByteBuffer tmp_buf, result;
    if (!(insert_iter >= this->begin() && 
            insert_iter < this->end())) {
        return -1;
    }

    ByteBufferIterator begin_iter = this->begin();
    ssize_t copy_front_size = insert_iter - begin_iter + 1;
    this->get_data(result, begin_iter, copy_front_size);
    
    result = result + buff;
    copy_front_size = this->end() - insert_iter - 1;
    insert_iter++;
    this->get_data(tmp_buf, insert_iter, copy_front_size);
    result = result + tmp_buf;

    *this = result;

    return 0;
}

    // 返回符合模式 regex 的子串(使用正则表达式)
std::vector<ByteBuffer> 
ByteBuffer::match(ByteBuffer &regex_str)
{
    std::vector<ByteBuffer> ret_match_str;
    std::regex reg(regex_str.str());
    std::string content(this->str());
    std::smatch m;

    auto pos = content.cbegin();
    auto end = content.cend();
    for (; std::regex_search(pos, end, m, reg); pos = m.suffix().first)
    {
        ByteBuffer match_substr(m.str());
        ret_match_str.push_back(match_substr);
    }

    return ret_match_str;
}

//////////////////////迭代器////////////////////////////////
ByteBufferIterator::ByteBufferIterator(void)
    : buff_(nullptr), curr_pos_(0) 
{}

ByteBufferIterator::ByteBufferIterator(const ByteBufferIterator &iter)
    : buff_(iter.buff_), curr_pos_(iter.curr_pos_)
{}

ByteBufferIterator::ByteBufferIterator(const ByteBuffer *buffer, const ssize_t &pos)
        : buff_(buffer), curr_pos_(pos)
{}

ByteBufferIterator::~ByteBufferIterator(void)
{}

bufftype 
ByteBufferIterator::operator*()
{
    if (this->check_iterator() == false) {
        dump_stack();
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR, "Msg: out of range. Info:\n%s", this->debug_info().c_str()));
    }
    return buff_->buffer_[curr_pos_];
}

ByteBufferIterator 
ByteBufferIterator::operator+(ssize_t inc)
{
    ByteBufferIterator tmp_iter = *this;
    this->move_postion(inc, tmp_iter.curr_pos_);

    return tmp_iter;
}

ByteBufferIterator 
ByteBufferIterator::operator-(int des) 
{
    ByteBufferIterator tmp_iter = *this;
    this->move_postion(-1 * des, tmp_iter.curr_pos_);

    return tmp_iter;
}

ssize_t 
ByteBufferIterator::operator-(ByteBufferIterator &rhs)
{
    if (this->buff_->buffer_ != rhs.buff_->buffer_) {
        return 0;
    }

    ssize_t max_size = buff_->max_buffer_size_;
    ssize_t start = this->buff_->start_read_pos_;

    ssize_t right_postion = (rhs.curr_pos_ < this->curr_pos_ ? this->curr_pos_ : rhs.curr_pos_);
    ssize_t left_postion = (right_postion == rhs.curr_pos_ ? this->curr_pos_ : rhs.curr_pos_);

    ssize_t diff = right_postion - left_postion;

    if (left_postion < start) {
        return max_size - diff;
    }
    
    return diff;
}

// 前置++
ByteBufferIterator& 
ByteBufferIterator::operator++()
{
    this->move_postion(1, this->curr_pos_);

    return *this;
}

// 后置++
ByteBufferIterator 
ByteBufferIterator::operator++(int)
{
    ByteBufferIterator tmp_iter = *this;
    this->move_postion(1, this->curr_pos_);

    return tmp_iter;
}

// 前置--
ByteBufferIterator& 
ByteBufferIterator::operator--()
{
    this->move_postion(-1, this->curr_pos_);

    return *this;
}

// 后置--
ByteBufferIterator 
ByteBufferIterator::operator--(int)
{
    ByteBufferIterator tmp_iter = *this;
    this->move_postion(-1, this->curr_pos_);

    return tmp_iter;
}

// +=
ByteBufferIterator& 
ByteBufferIterator::operator+=(ssize_t inc)
{
    this->move_postion(inc, this->curr_pos_);

    return *this;
}

ByteBufferIterator& 
ByteBufferIterator::operator-=(ssize_t des)
{
    this->move_postion(-1 * des, this->curr_pos_);

    return *this;
}

// 只支持 == ,!= , = 其他的比较都不支持
bool 
ByteBufferIterator::operator==(const ByteBufferIterator& iter) const 
{
    return (curr_pos_ == iter.curr_pos_ && buff_ == iter.buff_);
}

bool 
ByteBufferIterator::operator!=(const ByteBufferIterator& iter) const 
{
    return (curr_pos_ != iter.curr_pos_ || buff_ != iter.buff_);
}

bool 
ByteBufferIterator::operator>(const ByteBufferIterator& iter) const 
{
    if (buff_ != iter.buff_) {
        return false;
    }
    if (curr_pos_ > iter.curr_pos_) {
        return true;
    } else if (curr_pos_ < iter.curr_pos_) {
        if (curr_pos_ < buff_->start_read_pos_) {
            return true;
        }
    }

    return false;
}
bool 
ByteBufferIterator::operator>=(const ByteBufferIterator& iter) const 
{
    if (buff_ != iter.buff_) {
        return false;
    }
    if (curr_pos_ >= iter.curr_pos_) {
        return true;
    } else if (curr_pos_ < iter.curr_pos_) {
        if (curr_pos_ < buff_->start_read_pos_) {
            return true;
        }
    }

    return false;
}
bool 
ByteBufferIterator::operator<(const ByteBufferIterator& iter) const 
{
    if (buff_ != iter.buff_) {
        return false;
    }
    if (curr_pos_ >= iter.curr_pos_) {
        return false;
    } else if (curr_pos_ < iter.curr_pos_) {
        if (curr_pos_ < buff_->start_read_pos_) {
            return false;
        }
    }

    return true;
}
bool 
ByteBufferIterator::operator<=(const ByteBufferIterator& iter) const 
{
    if (buff_ != iter.buff_) {
        return false;
    }
    if (curr_pos_ > iter.curr_pos_) {
        return false;
    } else if (curr_pos_ < iter.curr_pos_) {
        if (curr_pos_ < buff_->start_read_pos_) {
            return false;
        }
    }

    return true;
}
ByteBufferIterator& 
ByteBufferIterator::operator=(const ByteBufferIterator& src)
{
    if (src != *this) {
        buff_ = src.buff_;
        curr_pos_ = src.curr_pos_;
    }

    return *this;
}

std::string 
ByteBufferIterator::debug_info(void) 
{
    std::ostringstream ostr;

    ostr << std::endl << "--------------debug_info-----------------------" << std::endl;
    ostr << "curr_pos: " << curr_pos_ << std::endl;
    ostr << "begin_pos: " << buff_->begin().curr_pos_ << std::endl;
    ostr << "end_pos: " << buff_->end().curr_pos_ << std::endl;
    ostr << "buff_length: "  << buff_->data_size() << std::endl;
    ostr << "------------------------------------------------" << std::endl;

    return ostr.str();
}

bool 
ByteBufferIterator::check_iterator(void) 
{
    if (buff_ == nullptr || buff_->buffer_ == nullptr) {
        curr_pos_ = 0;
        return false;
    }

    if (buff_->start_write_pos_ >= buff_->start_read_pos_) {
        if (curr_pos_ < buff_->start_read_pos_ || curr_pos_ >= buff_->start_write_pos_) {
            curr_pos_ = buff_->start_write_pos_;
            return false;
        }
    }

    if (buff_->start_write_pos_ < buff_->start_read_pos_) {
        if (curr_pos_ < buff_->start_read_pos_ && curr_pos_ >= buff_->start_write_pos_) {
            curr_pos_ = buff_->start_write_pos_;
            return false;
        }
    }
    
    return true;
}

bool 
ByteBufferIterator::move_postion(ssize_t distance, ssize_t &new_postion)
{
    new_postion = this->curr_pos_ + distance;
    ssize_t start = this->buff_->start_read_pos_;
    ssize_t tail = this->buff_->start_write_pos_;
    ssize_t max_size = buff_->max_buffer_size_;

    if (start < tail) {
        if (start <= new_postion && new_postion < tail) {
            return true;
        }
    } else if (start > tail) {
        if (start <= new_postion && new_postion < max_size) {
            return true;
        } else if (0 <= new_postion && new_postion < tail) {
            return true;
        }

        if (new_postion >= max_size) {
            new_postion = new_postion - max_size;
            if (0 <= new_postion && new_postion < tail) {
                return true;
            }
        } else if (new_postion < 0) {
            new_postion = new_postion + max_size;
            if (start <= new_postion && new_postion <= max_size) {
                return true;
            }
        }
    }

    new_postion = this->buff_->start_write_pos_;
    return false;
}

}