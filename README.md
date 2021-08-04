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

// 重新分配缓冲区大小(只能向上增长), size表示重新分配缓冲区的下限
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

// 其他方式读取/修改数据
```
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
// 添加用法
```