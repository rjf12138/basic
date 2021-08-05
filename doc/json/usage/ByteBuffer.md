### ByteBuffer 用法
#### 功能
```
// 下面六个函数从缓存开始位置中读取指定数据， 同时缓存中将删除被读取的数据
ssize_t read_int8(int8_t &val);
ssize_t read_int16(int16_t &val);
ssize_t read_int32(int32_t &val);
ssize_t read_int64(int64_t &val);
ssize_t read_string(string &str, ssize_t str_size = -1);
ssize_t read_bytes(void *buf, ssize_t buf_size, bool match = false);

// 在缓存的末尾添加数据
ssize_t write_int8(int8_t val);
ssize_t write_int16(int16_t val);
ssize_t write_int32(int32_t val);
ssize_t write_int64(int64_t val);
ssize_t write_string(const string &str, ssize_t str_size = -1);
ssize_t write_bytes(const void *buf, ssize_t buf_size, bool match = false);
```

```
// 返回缓存的一些属性
bool empty(void) const;         // 缓存是否为空
ssize_t data_size(void) const;  // 返回缓存内数据的大小
ssize_t idle_size(void) const;  // 返回缓存内空余空间的大小
ssize_t clear(void);            // 清空缓存

#define MAX_BUFFER_SIZE     1073741824 // 1*1024*1024*1024 (1GB)， 可分配的最大空间
#define MAX_DATA_SIZE       1073741823 // 多的一个字节用于防止，缓存写满时，start_write 和 start_read 重合而造成分不清楚是写满了还是没写

// 重新分配缓冲区大小(只能向上增长), size表示重新分配缓冲区的下限, 最大不能超过 MAX_BUFFER_SIZE
ssize_t resize(ssize_t size);
```

```
// 重载操作符
ByteBuffer& operator+(const ByteBuffer &rhs); // 两个缓存内容进行拼接
ByteBuffer& operator+=(const ByteBuffer &rhs);

bool operator==(const ByteBuffer &rhs) const; // 判断缓存是否相等
bool operator!=(const ByteBuffer &rhs) const;

ByteBuffer& operator=(const ByteBuffer& src); // 缓存赋值
bufftype& operator[](ssize_t index); // 使用下标访问缓存字节

// 返回起始结束迭代器
iterator begin(void);
iterator end(void);
iterator last_data(void);

// 返回const 起始结束迭代器
const_iterator begin(void) const;
const_iterator end(void) const;
const_iterator last_data(void) const;

// 判断 patten 是不是 bytebuffer 从 iter 开始的子串, size: -1 表示匹配全部, 否则指定具体大小
bool bytecmp(ByteBufferIterator &iter, ByteBuffer &patten, ssize_t size = -1);
```

```
// 只读取但是不修改原来的数据
// 将ByteBuffer中数据以字符串形式返回
std::string str();
// 获取 ByteBuffer 迭代器指定范围的数据
ssize_t get_data(ByteBuffer &out, ByteBufferIterator &copy_start, ssize_t copy_size);
// 只读不修改读位置
ssize_t read_only(ssize_t start_pos, void *buf, ssize_t buf_size, bool match = false);

//////////////////////////////////////////////////
// 允许外部程序直接通过指针来添加数据，防止二次拷贝
// 向外面直接提供 buffer_ 指针，它们写是直接写入指针，避免不必要的拷贝
buffptr get_write_buffer_ptr(void) const;
buffptr get_read_buffer_ptr(void) const;

// ByteBuffer 是循环队列，读写不一定是连续的
ssize_t get_cont_write_size(void) const;
ssize_t get_cont_read_size(void) const;

// 更新读写数据和剩余的缓冲大小
ssize_t update_write_pos(ssize_t offset);
ssize_t update_read_pos(ssize_t offset);

/////////////////////////////////////////////////////////
// 用例
ByteBuffer buffer;
while(true) {
    int read_count = read(fd, buffer.get_write_buffer_ptr(), buffer.get_cont_write_size());
    if (read_count > 0) {
        buffer.update_write_pos(read_count);
    } else if (read_count < 0){
        printf("Error");
        break;
    } else {
        break;
    }
}
/////////////////////////////////////////////////////////////////////
ByteBuffer buffer;
while(true) {
    int read_count = write(fd, buffer.get_read_buffer_ptr(), buffer.get_cont_read_size());
    if (read_count > 0) {
        buffer.update_read_pos(read_count);
    } else if (read_count < 0){
        printf("Error");
        break;
    } else {
        break;
    }
}
```

```
// 操作缓存
// 返回 ByteBuffer 中所有匹配 buff 的迭代器
std::vector<ByteBufferIterator> find(const ByteBuffer &buff);

// 根据 buff 分割 ByteBuffer
vector<ByteBuffer> split(const ByteBuffer &buff);

// 将 Bytebuffer 中 buf1 替换为 buf2
ByteBuffer replace(ByteBuffer buf1, const ByteBuffer &buf2, ssize_t index = -1);

// 移除 ByteBuff 中匹配 buff 的子串
// index 指定第几个匹配的子串， index 超出范围时，删除所有匹配子串, index 从0 开始计数
ByteBuffer remove(const ByteBuffer &buff, ssize_t index = -1);

// 在 ByteBuff 指定迭代器前/后插入子串 buff
ssize_t insert_front(ByteBufferIterator &insert_iter, const ByteBuffer &buff);
ssize_t insert_back(ByteBufferIterator &insert_iter, const ByteBuffer &buff);

// 返回符合模式 regex 的子串(使用正则表达式)
vector<ByteBuffer> match(ByteBuffer &regex);

1. match(regex) 用法
patten.write_string("<(.*)>(.*)</(\\1)>");
buff.write_string("123<xml>value</xml>456<widget>center</widget>hahaha<vertical>window</vertical>the end");

auto ret = buff.match(patten);
ASSERT_EQ(ret.size(), static_cast<std::size_t>(3));
ASSERT_EQ(ret[0].str(), std::string("<xml>value</xml>"));
ASSERT_EQ(ret[1].str(), std::string("<widget>center</widget>"));
ASSERT_EQ(ret[2].str(), std::string("<vertical>window</vertical>"));
```