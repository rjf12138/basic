#include "byte_buffer.h"
#include "logger.h"

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
    vector<int>::iterator str;
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

ssize_t ByteBuffer::set_extern_buffer(buffptr exbuf, int buff_size)
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
    bytebuffer_iterator tmp(this, start_read_pos_);
    if (this->data_size() <= 0) {
        return this->end();
    }

    return (tmp + (this->data_size() - 1));
}

ByteBuffer::iterator
ByteBuffer::cbegin(void) const
{
    return iterator(this, start_read_pos_);
}

ByteBuffer::iterator
ByteBuffer::cend(void) const
{
    return iterator(this, start_write_pos_);
}

ByteBuffer::iterator
ByteBuffer::clast_data(void) const
{
    iterator tmp(this, start_read_pos_);
    if (this->data_size() <= 0) {
        return this->cend();
    }

    return (tmp + (this->data_size() - 1));
}

ssize_t ByteBuffer::copy_data_to_buffer(const void *data, ssize_t size)
{
    if (data == nullptr || size <= 0) {
        return 0;
    }

    if (this->idle_size() <= size) {
        int ret = this->resize(max_buffer_size_ + size);
        if (ret == -1) {
           return 0;
        }
    }

    // if (this->idle_size() < size) {
    //     fprintf(stderr, "ByteBuffer remain idle space(%ld) is less than size(%ld)!\n", this->idle_size(), size);
    //     return 0;
    // }

    ssize_t copy_size = size;
    buffptr data_ptr = (buffptr)data;
    
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
   
    // if (this->data_size() < size) {
    //     fprintf(stderr, "ByteBuffer remain data(%ld) is less than size(%ld)!\n", this->data_size(), size);
    //     return 0;
    // }

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
ByteBuffer::read_string(string &str, ssize_t str_size)
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
        return 0;
    }
    str_ptr[str_size] = '\0';
    str = str_ptr;
    delete[] str_ptr;

    return str.length();
}

ssize_t 
ByteBuffer::read_bytes(void *buf, ssize_t buf_size, bool match)
{
    if (buf == nullptr) {
        return 0;
    }

    return this->copy_data_from_buffer(buf, buf_size);
}

ssize_t 
ByteBuffer::read_only(ssize_t start_pos, void *buf, ssize_t buf_size, bool match)
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
    std::string str;
    this->read_string(str);

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
ByteBuffer::write_string(const string &str, ssize_t str_size)
{
    return this->copy_data_to_buffer(str.c_str(), str.length());
}

ssize_t ByteBuffer::write_bytes(const void *buf, ssize_t buf_size, bool match)
{
    if (buf == NULL) {
        return 0;
    }

    return this->copy_data_to_buffer(buf, buf_size);
}

int ByteBuffer::read_int16_ntoh(int16_t &val)
{
    int ret = this->read_int16(val);
    if (ret == 0) {
        return 0;
    }
    val = ntohs(val);

    return ret;
}

int ByteBuffer::read_int32_ntoh(int32_t &val)
{
    int ret = this->read_int32(val);
    if (ret == 0) {
        return 0;
    }
    val = ntohl(val);

    return ret;
}

int ByteBuffer::write_int16_hton(const int16_t &val)
{
    int16_t tmp = val;
    tmp = htons(val);
    int ret = this->write_int16(tmp);

    return ret;
}

int ByteBuffer::write_int32_hton(const int32_t &val)
{
    int32_t tmp = val;
    tmp = htonl(val);
    int  ret = this->write_int32(tmp);

    return ret;
}

ssize_t
ByteBuffer::get_data(ByteBuffer &out, bytebuffer_iterator &copy_start, ssize_t copy_size)
{
    out.clear();
    if (this->buffer_ == nullptr || copy_size <= 0) {
        return 0;
    }

    if (copy_start.buff_->buffer_ != this->buffer_) {
        return 0;
    }

    ssize_t i = 0;
    bytebuffer_iterator tmp = copy_start;
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

    auto lhs_iter = this->cbegin();
    auto rhs_iter = rhs.cbegin();

    while (true) {
        if (lhs_iter == this->cend() && rhs_iter == rhs.cend()) {
            return true;
        } else if (lhs_iter != this->cend() && rhs_iter == rhs.cend()) {
            return false;
        } else if (lhs_iter == this->cend() && rhs_iter != rhs.cend()) {
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
        ostringstream ostr;
        ostr << "Line: " << __LINE__ << " out of range.";
        throw runtime_error(ostr.str());
    }

    index = (this->start_read_pos_ + index) %  max_buffer_size_;

    return buffer_[index];
}

bool 
ByteBuffer::bytecmp(bytebuffer_iterator &iter, ByteBuffer &patten, ssize_t size)
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
std::vector<bytebuffer_iterator>
ByteBuffer::find(ByteBuffer patten)
{
    std::vector<bytebuffer_iterator> result;
    if (patten.data_size() == 0 || this->data_size() == 0) {
        return result;
    }

    int patten_index = 0;
    for (int i = 0; i < this->data_size(); ++i) {
        if (patten[patten_index] == (*this)[i]) {
            for (int j = i; j < this->data_size();) {
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
ByteBuffer::split(ByteBuffer buff)
{
    std::vector<ByteBuffer> result;
    if (buff.data_size() <= 0 || this->data_size() <= 0) {
        result.push_back(*this);
        return result;
    }

    std::vector<bytebuffer_iterator> find_buff = this->find(buff);

    ByteBuffer tmp;
    ssize_t copy_size;
    bytebuffer_iterator start_copy_pos = this->begin();
    for (std::size_t i = 0; i < find_buff.size(); ++i) {
        copy_size = find_buff[i] - start_copy_pos;

        this->get_data(tmp, start_copy_pos, copy_size);
        start_copy_pos = find_buff[i] + buff.data_size();
        
        if (tmp.data_size() <= 0) {
            continue;
        }

        result.push_back(tmp);
    }

    copy_size = (this->end() - start_copy_pos); // 保存剩余的字符
    if (copy_size > 0) {
        this->get_data(tmp, start_copy_pos, copy_size);
        if (tmp.data_size() > 0) {
            result.push_back(tmp);
        }
    }

    return result;
}


ByteBuffer 
ByteBuffer::replace(ByteBuffer buf1, ByteBuffer buf2, ssize_t index)
{
    if (buf1.data_size() <= 0 || this->data_size() <= 0) {
        return *this;
    }

    ssize_t copy_size = 0;
    ByteBuffer result, tmp;
    bytebuffer_iterator copy_pos_iter = this->begin();
    std::vector<bytebuffer_iterator> find_buff = this->find(buf1);
    if (find_buff.size() == 0) {
        return *this;
    }

    if (index < 0 || index >= (ssize_t)find_buff.size()) { // 替换所有
        for (std::size_t i = 0; i < find_buff.size(); ++i) {
            copy_size = find_buff[i] - copy_pos_iter;
            if (copy_size > 0) {
                this->get_data(tmp, copy_pos_iter, copy_size);
                result = result + tmp;
                result = result + buf2;
                copy_pos_iter = find_buff[i] + buf1.data_size();
            }
        }

        copy_size = this->end() - copy_pos_iter; // 保存剩余的字符
        if (copy_size > 0) {
            this->get_data(tmp, copy_pos_iter, copy_size);
            result = result + tmp;
        }
    } else {
        copy_size = find_buff[index] - copy_pos_iter;
        if (copy_size > 0) {
            this->get_data(tmp, copy_pos_iter, copy_size);
            result = result + tmp;
            result = result + buf2;
            copy_pos_iter = find_buff[index] + buf1.data_size();
        }
    }

    return result;
}


ByteBuffer 
ByteBuffer::remove(ByteBuffer buff, ssize_t index)
{
    if (buff.data_size() <= 0 || this->data_size() <= 0) {
        return *this;
    }
    
    ByteBuffer tmp_buf;
    std::vector<bytebuffer_iterator> find_buff = this->find(buff);
    if (index < 0 || index >= (ssize_t)find_buff.size()) {
        index = -1;
    }

    if (index == -1) {
        std::vector<ByteBuffer> ret = this->split(buff);
        for (std::size_t i = 0; i < ret.size(); ++i) {
            tmp_buf = tmp_buf + ret[i];
        }
    } else {
        bytebuffer_iterator begin_iter = this->begin();
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
ByteBuffer::insert_front(bytebuffer_iterator &insert_iter, ByteBuffer buff)
{
    ByteBuffer tmp_buf, result;
    if (!(insert_iter >= this->begin() && 
            insert_iter < this->end())) {
        return -1;
    }

    bytebuffer_iterator begin_iter = this->begin();
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
ByteBuffer::insert_back(bytebuffer_iterator &insert_iter, ByteBuffer buff)
{
    ByteBuffer tmp_buf, result;
    if (!(insert_iter >= this->begin() && 
            insert_iter < this->end())) {
        return -1;
    }

    bytebuffer_iterator begin_iter = this->begin();
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
vector<ByteBuffer> 
ByteBuffer::match(ByteBuffer regex_str)
{
    vector<ByteBuffer> ret_match_str;
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
bytebuffer_iterator::bytebuffer_iterator(void)
    : buff_(nullptr), curr_pos_(0) 
{}

bytebuffer_iterator::bytebuffer_iterator(const bytebuffer_iterator &iter)
    : buff_(iter.buff_), curr_pos_(iter.curr_pos_)
{}

bytebuffer_iterator::bytebuffer_iterator(const ByteBuffer *buffer, const ssize_t &pos)
        : buff_(buffer), curr_pos_(pos)
{}

bytebuffer_iterator::~bytebuffer_iterator(void)
{}

bufftype 
bytebuffer_iterator::operator*()
{
    if (this->check_iterator() == false) {
        throw runtime_error(GLOBAL_GET_MSG("Msg: out of range. Info:\n%s", this->debug_info().c_str()));
    }
    return buff_->buffer_[curr_pos_];
}

bytebuffer_iterator 
bytebuffer_iterator::operator+(ssize_t inc)
{
    bytebuffer_iterator tmp_iter = *this;
    this->move_postion(inc, tmp_iter.curr_pos_);

    return tmp_iter;
}

bytebuffer_iterator 
bytebuffer_iterator::operator-(int des) 
{
    bytebuffer_iterator tmp_iter = *this;
    this->move_postion(-1 * des, tmp_iter.curr_pos_);

    return tmp_iter;
}

ssize_t 
bytebuffer_iterator::operator-(bytebuffer_iterator &rhs)
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
bytebuffer_iterator& 
bytebuffer_iterator::operator++()
{
    this->move_postion(1, this->curr_pos_);

    return *this;
}

// 后置++
bytebuffer_iterator 
bytebuffer_iterator::operator++(int)
{
    bytebuffer_iterator tmp_iter = *this;
    this->move_postion(1, this->curr_pos_);

    return tmp_iter;
}

// 前置--
bytebuffer_iterator& 
bytebuffer_iterator::operator--()
{
    this->move_postion(-1, this->curr_pos_);

    return *this;
}

// 后置--
bytebuffer_iterator 
bytebuffer_iterator::operator--(int)
{
    bytebuffer_iterator tmp_iter = *this;
    this->move_postion(-1, this->curr_pos_);

    return tmp_iter;
}

// +=
bytebuffer_iterator& 
bytebuffer_iterator::operator+=(ssize_t inc)
{
    this->move_postion(inc, this->curr_pos_);

    return *this;
}

bytebuffer_iterator& 
bytebuffer_iterator::operator-=(ssize_t des)
{
    this->move_postion(-1 * des, this->curr_pos_);

    return *this;
}

// 只支持 == ,!= , = 其他的比较都不支持
bool 
bytebuffer_iterator::operator==(const bytebuffer_iterator& iter) const 
{
    return (curr_pos_ == iter.curr_pos_ && buff_ == iter.buff_);
}

bool 
bytebuffer_iterator::operator!=(const bytebuffer_iterator& iter) const 
{
    return (curr_pos_ != iter.curr_pos_ || buff_ != iter.buff_);
}

bool 
bytebuffer_iterator::operator>(const bytebuffer_iterator& iter) const 
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
bytebuffer_iterator::operator>=(const bytebuffer_iterator& iter) const 
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
bytebuffer_iterator::operator<(const bytebuffer_iterator& iter) const 
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
bytebuffer_iterator::operator<=(const bytebuffer_iterator& iter) const 
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
bytebuffer_iterator& 
bytebuffer_iterator::operator=(const bytebuffer_iterator& src)
{
    if (src != *this) {
        buff_ = src.buff_;
        curr_pos_ = src.curr_pos_;
    }

    return *this;
}

string 
bytebuffer_iterator::debug_info(void) 
{
    ostringstream ostr;

    ostr << std::endl << "--------------debug_info-----------------------" << std::endl;
    ostr << "curr_pos: " << curr_pos_ << std::endl;
    ostr << "begin_pos: " << buff_->cbegin().curr_pos_ << std::endl;
    ostr << "end_pos: " << buff_->cend().curr_pos_ << std::endl;
    ostr << "buff_length: "  << buff_->data_size() << std::endl;
    ostr << "------------------------------------------------" << std::endl;

    return ostr.str();
}

bool 
bytebuffer_iterator::check_iterator(void) 
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
bytebuffer_iterator::move_postion(ssize_t distance, ssize_t &new_postion)
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