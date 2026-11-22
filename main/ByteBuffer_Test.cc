#include "byte_buffer.h"
#include "gtest/gtest.h"

using namespace basic;

namespace my {
namespace project {
namespace {

#define MUTITHREAD 1
#define TEST_SPCE_INCREASE 26

#define TEST_THREAD_NUM 1000
#define TEST_COUNT 8000

////////////////////////////测试工具函数//////////////////////////////////
std::vector<ByteBuffer>
random_str(ByteBuffer &src, ByteBuffer &patten, int gap)
{
    ByteBuffer front_str, back_str;
    std::vector<ByteBuffer> ret;

    int j = 0;
    for (auto iter = src.begin(); iter != src.end(); ++iter) {
        front_str.write_int8(*iter);
        back_str.write_int8(*iter);

        if (j >= gap) {
            ByteBuffer::iterator last_iter = front_str.last_data();
            front_str.insert_front(last_iter, patten);

            last_iter = back_str.last_data();
            back_str.insert_back(last_iter, patten);
            j = 0;
            continue;
        }
        ++j;
    }

    ret.push_back(front_str);
    ret.push_back(back_str);

    return ret;
}


////////////////////////////////////////////////////////////////////////////////

//#define TEST_WRITE_ALL_BUFF 

class ByteBuffer_Test : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// 读写指定字节数据
struct test_stru {
    int8_t i8;
    int16_t i16;
    char str[34];
    char buf[16];
};

// 对非锁的读写函数循环测试，功能测试
TEST_F(ByteBuffer_Test, ByteBuff_none_lock_read_write) 
{
    int buff_size = 64;
    int test_cnt = 900000;
    ByteBuffer buff(buff_size);
    // 读写8位数据
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(buff.write_int8('a'), 1);
        ASSERT_EQ(buff.data_size(), 1);
        int8_t val_8;
        ASSERT_EQ(buff.read_int8(val_8), 1);
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_8, 'a');
    }
    // 读写16位数据
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(buff.write_int16(6536), 2);
        ASSERT_EQ(buff.data_size(), 2);
        int16_t val_16;
        ASSERT_EQ(buff.read_int16(val_16), 2);
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_16, 6536);
    }
    // 读写32位数据
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(buff.write_int32(655536), 4);
        ASSERT_EQ(buff.data_size(), 4);
        int32_t val_32;
        ASSERT_EQ(buff.read_int32(val_32), 4);
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_32, 655536);
    }
    // 读写64位数据
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(buff.write_int64(65566536), 8);
        ASSERT_EQ(buff.data_size(), 8);
        int64_t val_64;
        ASSERT_EQ(buff.read_int64(val_64), 8);
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_64, 65566536);
    }
    // 读写字符串
    std::string str = "Hello, world";
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(static_cast<std::size_t>(buff.write_string(str)), str.length());
        ASSERT_EQ(static_cast<std::size_t>(buff.data_size()), str.length());
        std::string val_str;
        ASSERT_EQ(static_cast<std::size_t>(buff.read_string(val_str)), str.length());
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_str, str);
    }

    struct test_stru stru = {'b', 12345, "Nice to meet you", "hello"};
    for (int i = 0; i < test_cnt; ++i) {
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(static_cast<std::size_t>(buff.write_bytes((reinterpret_cast<void*>(&stru)), sizeof(stru))) , sizeof(stru));
        ASSERT_EQ(static_cast<std::size_t>(buff.data_size()), sizeof(stru));
        struct test_stru val_stru;
        ASSERT_EQ(static_cast<std::size_t>(buff.read_bytes(&val_stru, sizeof(stru))), sizeof(stru));
        ASSERT_EQ(buff.data_size(), 0);
        ASSERT_EQ(val_stru.i8, stru.i8);
        ASSERT_EQ(val_stru.i16, stru.i16);
        ASSERT_EQ(strcmp(val_stru.str, stru.str), 0);
        ASSERT_EQ(strcmp(val_stru.buf, stru.buf), 0);
    }

    char read_buf[4] = {'\0'};
    str = "Nice to meet you";
    for (int i = 0; i < test_cnt; ++i) {
        buff.write_bytes(str.c_str(), str.length());
        
        for (std::size_t k = 1; k < 4; ++k) {
            for (std::size_t j = 1; j < str.length(); ++j) {
                ssize_t ret = buff.read_only(j, read_buf, k);
                ASSERT_EQ(static_cast<size_t>(ret), strlen(read_buf));
                ASSERT_EQ(memcmp(str.c_str() + j, read_buf, ret), 0);
                memset(read_buf, 0, 4);
            }
        }

        std::string tmpstr;
        buff.read_string(tmpstr);
    }
}

// 测试 int64 读写（验证 sizeof(int64_t) 修正）
TEST_F(ByteBuffer_Test, read_write_int64_full_range)
{
    ByteBuffer buff(1024);
    // 测试正负数边界
    int64_t test_values[] = {
        0, 1, -1,
        INT64_MAX, INT64_MIN,
        0x7FFFFFFFFFFFFFFFLL, static_cast<int64_t>(0x8000000000000000ULL),
        12345678901234LL, -12345678901234LL
    };
    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); ++i) {
        ASSERT_EQ(buff.write_int64(test_values[i]), 8);
        int64_t val;
        ASSERT_EQ(buff.read_int64(val), 8);
        ASSERT_EQ(val, test_values[i]);
        ASSERT_EQ(buff.data_size(), 0);
    }
}

// 测试 write_string str_size 参数
TEST_F(ByteBuffer_Test, write_string_str_size_param)
{
    ByteBuffer buff(100);
    std::string str = "Hello, World! This is a test.";
    
    // str_size == -1 时写完整字符串
    ASSERT_EQ(buff.write_string(str, -1), static_cast<ssize_t>(str.length()));
    ASSERT_EQ(buff.str(), str);
    buff.clear();

    // str_size 指定部分长度
    std::string partial = str.substr(0, 5);
    ASSERT_EQ(buff.write_string(str, 5), 5);
    ASSERT_EQ(buff.str(), partial);
    buff.clear();

    // str_size 为 0
    ASSERT_EQ(buff.write_string(str, 0), 0);
    ASSERT_EQ(buff.str(), "");
}

// 测试 operator+ 值语义（不修改原对象）
TEST_F(ByteBuffer_Test, operator_plus_value_semantics)
{
    ByteBuffer a("Hello");
    ByteBuffer b(" World");
    ByteBuffer original_a = a;
    
    ByteBuffer c = a + b;
    
    ASSERT_EQ(a, original_a);  // a 不应被修改
    ASSERT_EQ(c.str(), "Hello World");
}

// 测试 str() 在空 buffer 时安全返回（之前会除零崩溃）
TEST_F(ByteBuffer_Test, str_on_empty_buffer)
{
    ByteBuffer empty;
    ASSERT_EQ(empty.str(), "");
    
    ByteBuffer cleared(100);
    cleared.write_string("test");
    ASSERT_EQ(cleared.str(), "test");
    cleared.clear();
    ASSERT_EQ(cleared.str(), "");
    ASSERT_EQ(cleared.data_size(), 0);
    ASSERT_EQ(cleared.empty(), true);
}

// 测试数据长度为0时读取不会崩溃（之前会除零崩溃），安全返回0
TEST_F(ByteBuffer_Test, read_on_empty_buffer)
{
    ByteBuffer empty;
    int8_t  v8  = 0;
    int16_t v16 = 0;
    int32_t v32 = 0;
    int64_t v64 = 0;
    ASSERT_EQ(empty.read_int8(v8),   0);
    ASSERT_EQ(empty.read_int16(v16), 0);
    ASSERT_EQ(empty.read_int32(v32), 0);
    ASSERT_EQ(empty.read_int64(v64), 0);

    char buf[16] = {0};
    ASSERT_EQ(empty.read_bytes(buf, 8), 0);
    ASSERT_EQ(empty.read_only(0, buf, 4), 0);

    std::string s;
    ASSERT_EQ(empty.read_string(s), 0);
    ASSERT_EQ(empty.str(), "");
    ASSERT_EQ(empty.data_size(), 0);

    // 读取完数据后的 buffer 继续读取也应安全
    ByteBuffer drained("abc");
    std::string content;
    ASSERT_EQ(drained.read_string(content), 3);
    ASSERT_EQ(drained.data_size(), 0);
    ASSERT_EQ(drained.read_int8(v8), 0);
    ASSERT_EQ(drained.read_bytes(buf, 4), 0);
    ASSERT_EQ(drained.read_only(0, buf, 4), 0);
}

// 测试 operator[] 负索引抛异常（之前会越界访问内存）
TEST_F(ByteBuffer_Test, operator_index_negative)
{
    ByteBuffer b("abc");
    bool thrown = false;
    try { auto c = b[-1]; (void)c; } catch (std::exception &e) { thrown = true; }
    ASSERT_TRUE(thrown);
}

// 测试 read_string 参数钳制、match 空模式安全返回
TEST_F(ByteBuffer_Test, read_match_edge_cases)
{
    // str_size 超过数据量时应钳制，不应读入垃圾
    ByteBuffer b("ab");
    std::string s;
    b.read_string(s, 64);
    ASSERT_EQ(s, "ab");

    // 空模式/空数据 match 应返回空而非死循环
    ByteBuffer pat;
    ByteBuffer content("abc");
    auto res = content.match(pat);
    ASSERT_EQ(res.size(), 0);

    ByteBuffer empty;
    auto res2 = empty.match(pat);
    ASSERT_EQ(res2.size(), 0);
}

// 测试环形缓冲数据环绕时 find/split/remove/replace 与迭代器距离的正确性
TEST_F(ByteBuffer_Test, operations_on_wraparound_data)
{
    ByteBuffer buff(8); // max_buffer_size = 16
    buff.write_string("abcdefghijklmn"); // 14 字节
    std::string tmp;
    buff.read_string(tmp, 10); // 读掉前 10 字节,剩 "klmn"
    ASSERT_EQ(buff.str(), "klmn");

    buff.write_string("opqrstuvwx"); // 再写 10 字节,数据环绕
    ASSERT_EQ(buff.data_size(), 14);
    ASSERT_EQ(buff.str(), "klmnopqrstuvwx");

    // 环绕状态下迭代器距离
    auto biter = buff.begin();
    auto eiter = buff.end();
    ASSERT_EQ(eiter - biter, 14);
    ASSERT_EQ(biter - eiter, 14);

    // 后区匹配删除
    buff.remove(ByteBuffer("op"));
    ASSERT_EQ(buff.str(), "klmnqrstuvwx");

    // 前区匹配替换
    ByteBuffer rep = buff.replace(ByteBuffer("uv"), ByteBuffer("UV"));
    ASSERT_EQ(rep.str(), "klmnqrstUVwx");
}

// 测试 buffer 空间增长，压力测试
TEST_F(ByteBuffer_Test, ByteBuffer_increase)
{
    ByteBuffer buff(1);
    ssize_t n = 2;
    for (int i = 0; i < TEST_SPCE_INCREASE; ++i) {
        for (ssize_t j = 0; j < n / 2; ++j) {
            buff.write_int8('a');
        }
        
        ASSERT_EQ(buff.data_size(), n / 2);
        int8_t out;
        for (ssize_t j = 0; j < n / 2; ++j) {
            buff.read_int8(out);
            ASSERT_EQ(buff.data_size(), n / 2 - j - 1);
        }

        ASSERT_EQ(buff.empty(), true);
        n = n * 2;
    }

#ifdef TEST_WRITE_ALL_BUFF
    // 将空间写满,再读出来
    ssize_t write_cnt = 0, read_cnt = 0;
    ByteBuffer max_buff;
    // 写 8 位数据
    int8_t data8 = 'b';
    while(true) {
        ssize_t ret = max_buff.write_int8(data8);
        if (ret != sizeof(int8_t)) {
            break;
        }
        write_cnt++;
    }
    ASSERT_EQ(max_buff.data_size(), MAX_DATA_SIZE);
    ASSERT_EQ(max_buff.idle_size(), 0);

    // 读 8 位数据
    while(true) {
        int8_t data;
        ssize_t ret = max_buff.read_int8(data);
        if (ret != sizeof(int8_t) || data != data8) {
            break;
        }
        read_cnt++;
    }
    ASSERT_EQ(max_buff.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(max_buff.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);
    // 同一缓存再次写满
    while(true) {
        ssize_t ret = max_buff.write_int8(data8);
        if (ret != sizeof(int8_t)) {
            break;
        }
        write_cnt++;
    }
    ASSERT_EQ(max_buff.data_size(), MAX_DATA_SIZE);
    ASSERT_EQ(max_buff.idle_size(), 0);

    // 读 8 位数据
    while(true) {
        int8_t data;
        ssize_t ret = max_buff.read_int8(data);
        if (ret != sizeof(int8_t) || data != data8) {
            break;
        }
        read_cnt++;
    }
    ASSERT_EQ(max_buff.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(max_buff.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);

//////////////////////////////////////// 16 /////////////////////////////////////
    write_cnt = 0, read_cnt = 0;
    ByteBuffer buff16;
    // 写 16 位数据
    int16_t data16 = 23456;
    while(true) {
        ssize_t ret = buff16.write_int16(data16);
        if (ret != sizeof(int16_t)) {
            break;
        }
        write_cnt++;
    }
    
    ASSERT_GT((ssize_t)sizeof(int16_t), MAX_DATA_SIZE - buff16.data_size());
    ASSERT_LE(buff16.idle_size(), (ssize_t)sizeof(int16_t));

    // 读 16 位数据
    while(true) {
        int16_t data;
        ssize_t ret = buff16.read_int16(data);
        if (ret != sizeof(int16_t) || data != data16) {
            break;
        }
        read_cnt++;
    }
    ASSERT_EQ(buff16.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buff16.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);
    write_cnt = read_cnt = 0;
    // 再次写满
    while(true) {
        ssize_t ret = buff16.write_int16(data16);
        if (ret != sizeof(int16_t)) {
            break;
        }
        write_cnt++;
    }
    ASSERT_GT((ssize_t)sizeof(int16_t), MAX_DATA_SIZE - buff16.data_size());
    ASSERT_LE(buff16.idle_size(), (ssize_t)sizeof(int16_t));
    // 读 16 位数据
    while(true) {
        int16_t data;
        ssize_t ret = buff16.read_int16(data);
        if (ret != sizeof(int16_t) || data != data16) {
            break;
        }
        read_cnt++;
    }
    ASSERT_EQ(buff16.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buff16.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);
    ////////////////////////////////// 16 ////////////////////////////////////////
    // 读 std::string 数据
    write_cnt = 0, read_cnt = 0;
    ByteBuffer buffbyte;

    // 写 std::string 数据
    std::string str = "sadjklfafks78934729374^*&^&%^&$^%$^%";
    while(true) {
        size_t ret = buffbyte.write_string(str);
        if (ret != str.length()) {
            break;
        }
        write_cnt++;
    }
    ASSERT_GT((ssize_t)str.length(), MAX_DATA_SIZE - buffbyte.data_size());
    ASSERT_LE(buffbyte.idle_size(), (ssize_t)str.length());

    while(true) {
        std::string read_str;
        size_t ret = buffbyte.read_string(read_str, str.length());
        if (ret != str.length() || read_str != str) {
            break;
        }
        read_cnt++;
    }

    ASSERT_EQ(buffbyte.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buffbyte.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);

    write_cnt = 0, read_cnt = 0;
    // 再次写满
    while(true) {
        size_t ret = buffbyte.write_string(str);
        if (ret != str.length()) {
            break;
        }
        write_cnt++;
    }
    ASSERT_GT((ssize_t)str.length(), MAX_DATA_SIZE - buffbyte.data_size());
    ASSERT_LE(buffbyte.idle_size(), (ssize_t)str.length());


    // 再次读 std::string 位数据
    while(true) {
        std::string read_str;
        size_t ret = buffbyte.read_string(read_str, str.length());
        if (ret != str.length() || str != read_str) {
            break;
        }
        read_cnt++;
    }
    ASSERT_EQ(buffbyte.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buffbyte.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);

    //////////////////////////////////// 多字节读写 ////////////////////////////////////////////
    // 读 多字节 数据
    write_cnt = 0, read_cnt = 0;
    buffbyte.clear();

    // 写 多字节 数据
    struct test_stru stru = {'b', 12345, "Nice to meet you", "hello"};
    ssize_t data_size = sizeof(stru);
    while(true) {
        ssize_t ret = buffbyte.write_bytes(&stru, data_size);
        if (ret != data_size) {
            break;
        }
        write_cnt++;
    }
    ASSERT_GT(data_size, MAX_DATA_SIZE - buffbyte.data_size());
    ASSERT_LE(buffbyte.idle_size(), data_size);

    while(true) {
        struct test_stru read_stru = {'a', 0, "", ""};
        ssize_t ret = buffbyte.read_bytes(&read_stru, data_size);
        if (ret != data_size || memcmp(&stru, &read_stru, sizeof(stru)) != 0) {
            break;
        }
        read_cnt++;
    }

    ASSERT_EQ(buffbyte.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buffbyte.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);

    write_cnt = 0, read_cnt = 0;
    // 再次写满
    while(true) {
        ssize_t ret = buffbyte.write_bytes(&stru, data_size);
        if (ret != data_size) {
            break;
        }
        write_cnt++;
    }
    ASSERT_GT(data_size, MAX_DATA_SIZE - buffbyte.data_size());
    ASSERT_LE(buffbyte.idle_size(), data_size);


    // 再次读 多字节 位数据
    while(true) {
        struct test_stru read_stru;
        ssize_t ret = buffbyte.read_bytes(&read_stru, data_size);
        if (ret != data_size || memcmp(&stru, &read_stru, sizeof(stru)) != 0) {
            break;
        }
        read_cnt++;
    }

    ASSERT_EQ(buffbyte.idle_size(), MAX_DATA_SIZE);
    ASSERT_EQ(buffbyte.data_size(), 0);
    ASSERT_EQ(write_cnt, read_cnt);
#endif
}

TEST_F(ByteBuffer_Test, boundary_test)
{
    ByteBuffer buff(-2);
    ASSERT_EQ(buff.data_size(), 0);
    ASSERT_EQ(buff.empty(), true);
    ASSERT_GE(buff.idle_size(), 0);

    test_stru test;
    buff.write_bytes(reinterpret_cast<void*>(&test), sizeof(test));
    ASSERT_EQ(static_cast<std::size_t>(buff.data_size()), sizeof(test));
    ASSERT_EQ(buff.empty(), false);
    
    buff.clear();
    ASSERT_EQ(buff.data_size(), 0);
    ASSERT_EQ(buff.empty(), true);
}

TEST_F(ByteBuffer_Test, copy_test)
{
    ByteBuffer src, dest;
    int start_size = 1000, end_size = 10000;
    for (int i = start_size;i < end_size; i += 100) {
        for (int j = 0;j < i; ++j) {
            src.write_int8(static_cast<int8_t>(j % 256));
        }
        dest = src;
        ASSERT_EQ(src, dest);
        dest.clear();
        src.clear();
    }
}

TEST_F(ByteBuffer_Test, iterator)
{
    ByteBuffer buff;

    std::string str = "Hello, world! Everyone";
    std::string str2= "Good Morning! Everyone...";

    // 两个字符串交叉写
    for (int i = 0; i < 5000; ++i) {
        if (i % 2 == 0) {
            buff.write_string(str);
        } else {
            buff.write_string(str2);
        }
    }

    size_t read_cnt = 0;
    bool choose_read_str = false;
    ByteBuffer::iterator iter = buff.begin();
    for (size_t i = 0; i < static_cast<size_t>(buff.data_size()); ++i) {
        if (choose_read_str == false) {
            ASSERT_EQ(*(iter + i), str[read_cnt]);
            read_cnt++;
            if (read_cnt == str.length()) {
                read_cnt = 0;
                choose_read_str = true;
            }
        } else {
            ASSERT_EQ(*(iter + i), str2[read_cnt]);
            read_cnt++;
            if (read_cnt == str2.length()) {
                read_cnt = 0;
                choose_read_str = false;
            }
        }
    }

    std::string read_str;
    read_cnt = str.length();
    choose_read_str = false;
    for (iter = buff.begin(); iter != buff.end(); ++iter) {
        read_str += *iter;
        read_cnt--;
        if (read_cnt == 0) {
            if (choose_read_str == false) {
                ASSERT_EQ(read_str, str);
                read_cnt = str2.length();
                choose_read_str = true;
                read_str = "";
            } else {
                ASSERT_EQ(read_str, str2);
                read_cnt = str.length();
                choose_read_str = false;
                read_str = "";
            }
        }
    }
    buff.read_string(read_str);

    // 单个字符串循环写
    for (int i = 0;i < 36000; ++i) {
        read_str = "";
        buff.write_string(str);
        for (iter = buff.begin(); iter != buff.end(); ++iter) {
            read_str += *iter;
        }
        ASSERT_EQ(str, read_str);
        buff.read_string(read_str);
    }

    // 测试判等
    bool throw_error = false;
    ByteBuffer buff1, buff2;
    ByteBuffer::iterator iter1, iter2;

    iter1 = buff1.begin();
    iter2 = buff2.begin();

    ASSERT_NE(iter1, iter2);
    iter2 = iter1;
    ASSERT_EQ(iter1, iter2);

    //测试异常抛出
    throw_error = false;
    try {
        char ch = *iter1;
        ch = ch + 1;
    } catch (std::runtime_error &e) {
        throw_error = true;
    }
    ASSERT_EQ(throw_error, true);

    // 测试前置，后置加减
    ByteBuffer buff_plus;
    for (int i = 0; i < 36000; ++i) {
        buff_plus.write_string("helxo world");

        iter = buff_plus.begin();
        ASSERT_EQ(*iter, 'h');
        iter++;
        ASSERT_EQ(*iter, 'e');
        ByteBuffer::iterator iter_back_plus = iter++;
        ASSERT_EQ(*iter_back_plus, 'e');
        ASSERT_EQ(*iter, 'l');
        ByteBuffer::iterator iter_front_plus = ++iter;
        ASSERT_EQ(*iter, 'x');
        ASSERT_EQ(*iter_front_plus, 'x');

        ByteBuffer::iterator iter_back_des = iter--;
        ASSERT_EQ(*iter_back_des, 'x');
        ASSERT_EQ(*iter, 'l');
        ByteBuffer::iterator iter_front_des = --iter;
        ASSERT_EQ(*iter, 'e');
        ASSERT_EQ(*iter_front_des, 'e');

        buff_plus.read_string(read_str);
    }

    // 测试加/减法
    buff.clear();

    for (int i = 0; i < 3600; ++i) {
        buff.write_string(str);
        iter1 = buff.begin();
        iter2 = buff.end();
        for (ssize_t j = buff.data_size(); j > 0; --j) {
            auto tmp_iter1 = iter2 - static_cast<int>(j);
            auto tmp_iter2 = iter1 + (buff.data_size() - j);
            ASSERT_EQ(*tmp_iter1, *tmp_iter2);
            ASSERT_EQ(str[buff.data_size() - j], *tmp_iter1);
        }
        buff.read_string(read_str);
    }

    // 测试两个位置距离
    buff.clear();

    for (int j = 0; j < 3600; ++j) {
        buff.write_string(str);
        iter1 = buff.begin();
        iter2 = buff.end();
        for (ssize_t i = buff.data_size(); i > 0; --i) {
            ssize_t dis1 = iter2 - iter1;
            ssize_t dis2 = iter1 - iter2;
            ++iter1;
            ASSERT_EQ(dis1, i);
            ASSERT_EQ(dis1, dis2);
        }

        iter1 = buff.begin();
        iter2 = buff.end();
        for (ssize_t i = buff.data_size(); i > 0; --i) {
            ssize_t dis1 = iter2 - iter1;
            ssize_t dis2 = iter1 - iter2;
            --iter2;
            ASSERT_EQ(dis1, i);
            ASSERT_EQ(dis1, dis2);
        }
        buff.read_string(read_str);
    }
}

// 测试迭代器比较运算在循环队列回绕场景下正确性
TEST_F(ByteBuffer_Test, iterator_comparison_wraparound)
{
    ByteBuffer buff(16);
    // 写入足够数据使循环队列回绕
    for (int cycle = 0; cycle < 10; ++cycle) {
        // 写入 12 字节
        for (int i = 0; i < 12; ++i) {
            buff.write_int8(static_cast<int8_t>('a' + i));
        }
        // 读取 8 字节（留下 4 字节，使下一轮写入时回绕）
        int8_t val;
        for (int i = 0; i < 8; ++i) {
            buff.read_int8(val);
        }

        // 验证 begin < end
        auto begin = buff.begin();
        auto end = buff.end();
        ASSERT_LT(begin, end);
        ASSERT_LE(begin, end);
        ASSERT_GT(end, begin);
        ASSERT_GE(end, begin);
        ASSERT_EQ(begin, begin);
    }
}

// 测试 == 和 != 操作符
TEST_F(ByteBuffer_Test, equality_test)
{
    ByteBuffer a("Hello World");
    ByteBuffer b("Hello World");
    ByteBuffer c("Hello");
    ByteBuffer d("Hello World!");
    ByteBuffer empty1;
    ByteBuffer empty2;

    ASSERT_EQ(a, b);
    ASSERT_NE(a, c);
    ASSERT_NE(a, d);
    ASSERT_EQ(empty1, empty2);

    // 测试自己赋值给自己
    a = a;
    ASSERT_EQ(a, b);
}

// 测试 remove 函数（包含之前 "多减1" 的修复验证）
TEST_F(ByteBuffer_Test, remove_test)
{
    // 删除全部匹配
    ByteBuffer data("abcaabbcc");
    data.remove(ByteBuffer("bc"));
    ASSERT_EQ(data, ByteBuffer("aaabc"));
    
    data.clear();
    data.write_string("aaaa");
    data.remove(ByteBuffer("a"));
    ASSERT_EQ(data.data_size(), 0);

    // 删除指定索引
    data.clear();
    data.write_string("one two three two four");
    data.remove(ByteBuffer("two"), 1);  // 删除第二个 "two"
    ASSERT_EQ(data, ByteBuffer("one two three  four"));
    
    // 测试空模式
    data.clear();
    data.write_string("hello");
    ByteBuffer result = data.remove(ByteBuffer(""));
    ASSERT_EQ(result, data);
}

// 测试 replace 函数
TEST_F(ByteBuffer_Test, replace_test)
{
    ByteBuffer data("hello world hello");
    ByteBuffer result = data.replace(ByteBuffer("hello"), ByteBuffer("hi"));
    ASSERT_EQ(result, ByteBuffer("hi world hi"));
    
    // index 超出范围时替换所有匹配子串
    data.clear();
    data.write_string("hello world hello");
    result = data.replace(ByteBuffer("hello"), ByteBuffer("hi"), 99);
    ASSERT_EQ(result, ByteBuffer("hi world hi"));

    // 替换部分
    data.clear();
    data.write_string("aaa bbb ccc");
    result = data.replace(ByteBuffer("bbb"), ByteBuffer("ddd"), 0);
    ASSERT_EQ(result, ByteBuffer("aaa ddd ccc"));
}

// 测试 insert_front / insert_back
TEST_F(ByteBuffer_Test, insert_operations)
{
    ByteBuffer src("abc");
    auto iter = src.begin();

    src.insert_front(iter, ByteBuffer("123"));
    ASSERT_EQ(src, ByteBuffer("123abc"));

    src.clear();
    src.write_string("abc");
    iter = src.begin();
    src.insert_back(iter, ByteBuffer("123"));
    ASSERT_EQ(src, ByteBuffer("a123bc"));
}

// 测试正则匹配 match
TEST_F(ByteBuffer_Test, match_regex)
{
    ByteBuffer buff;
    buff.write_string("abc123 def456 ghi789");
    ByteBuffer patten("\\d+");
    auto results = buff.match(patten);
    ASSERT_EQ(results.size(), 3);
    ASSERT_EQ(results[0].str(), "123");
    ASSERT_EQ(results[1].str(), "456");
    ASSERT_EQ(results[2].str(), "789");

    // 空 buffer match
    ByteBuffer empty;
    auto empty_results = empty.match(patten);
    ASSERT_EQ(empty_results.size(), 0);
}

// 测试 find 函数
TEST_F(ByteBuffer_Test, find_test)
{
    ByteBuffer data("hello world, hello cpp, hello test");
    auto results = data.find(ByteBuffer("hello"));
    ASSERT_EQ(results.size(), 3);
    
    // 无匹配
    auto no_match = data.find(ByteBuffer("xyz"));
    ASSERT_EQ(no_match.size(), 0);
    
    // 空模式
    auto empty_pattern = data.find(ByteBuffer(""));
    ASSERT_EQ(empty_pattern.size(), 0);
}

// 测试 split 函数
TEST_F(ByteBuffer_Test, split_test)
{
    ByteBuffer data("a,b,c,d");
    auto parts = data.split(ByteBuffer(","));
    ASSERT_EQ(parts.size(), 4);
    ASSERT_EQ(parts[0], ByteBuffer("a"));
    ASSERT_EQ(parts[1], ByteBuffer("b"));
    ASSERT_EQ(parts[2], ByteBuffer("c"));
    ASSERT_EQ(parts[3], ByteBuffer("d"));
    
    // 空分割符
    auto no_split = data.split(ByteBuffer(""));
    ASSERT_EQ(no_split.size(), 1);
    ASSERT_EQ(no_split[0], data);
}

// 测试 get_data
TEST_F(ByteBuffer_Test, get_data_test)
{
    ByteBuffer src("Hello World");
    ByteBuffer dest;
    auto begin = src.begin();
    auto end = src.begin();
    end += 5;
    
    ssize_t copied = src.get_data(dest, begin, 5);
    ASSERT_EQ(copied, 5);
    ASSERT_EQ(dest, ByteBuffer("Hello"));
}

// 测试 bytecmp
TEST_F(ByteBuffer_Test, bytecmp_test)
{
    ByteBuffer data("Hello World");
    ByteBuffer patten("Hello");
    
    auto iter = data.begin();
    ASSERT_EQ(data.bytecmp(iter, patten), true);
    
    iter += 6;
    patten.clear();
    patten.write_string("World");
    ASSERT_EQ(data.bytecmp(iter, patten), true);
    
    patten.clear();
    patten.write_string("Worlx");
    ASSERT_EQ(data.bytecmp(iter, patten), false);
}

#define RANDOM_RANGE 256
//#define RANDOM_RANGE (('z' - 'a')) + 'a')
TEST_F(ByteBuffer_Test, operate_buffer)
{
    // 测试 remove 
    ByteBuffer data("abcabc");
    data.remove(ByteBuffer("ca"));
    ASSERT_EQ(data, ByteBuffer("abbc"));

    data.remove(ByteBuffer(""));
    ASSERT_EQ(data, ByteBuffer("abbc"));
    // 测试 insert_front 和 insert_back
    ByteBuffer src_buf("Hello");
    std::vector<std::string> ins_bufs = {"", "1", "12", "12345"};
    
    auto beg_iter = src_buf.begin();
    src_buf.insert_front(beg_iter, ByteBuffer("12"));
    ASSERT_EQ(src_buf, ByteBuffer("12Hello"));
    
    beg_iter = src_buf.begin();
    src_buf.insert_back(beg_iter, ByteBuffer("12"));
    ASSERT_EQ(src_buf, ByteBuffer("1122Hello"));

    src_buf.clear();
    src_buf.write_string("Hello");
    beg_iter = src_buf.begin() + 2;
    src_buf.insert_front(beg_iter, ByteBuffer("12"));
    ASSERT_EQ(src_buf, ByteBuffer("He12llo"));

    beg_iter = src_buf.begin() + 2;
    src_buf.insert_back(beg_iter, ByteBuffer("12"));
    ASSERT_EQ(src_buf, ByteBuffer("He1122llo"));
    
    // 测试分割和替换
    int max_str_len = 50;
    for (int i = 0; i < 1000; ++i) {
        int patten_len = rand() % max_str_len;
        int src_len = rand() % max_str_len;
        int replace_len = rand() % max_str_len;

        ByteBuffer patten, src, replace_str;
        for (int j = 0; j < patten_len; ++j) {
            bufftype value = static_cast<bufftype>(rand() % RANDOM_RANGE);
            patten.write_int8(value);
        }

        for (int j = 0; j < src_len; ++j) {
            bufftype value = static_cast<bufftype>(rand() % RANDOM_RANGE);
            src.write_int8(value);
        }

        for (int j = 0;j < replace_len; ++j) {
            bufftype value = static_cast<bufftype>(rand() % RANDOM_RANGE);
            replace_str.write_int8(value);
        }

        if (src.find(patten).size() > 0) { // 确保src中不存在符合patten的子串
            src.remove(patten);
        }

        if (src.find(replace_str).size() > 0) { // 确保src中不存在符合replace_str的子串
            src.remove(replace_str);
        }

        for (int gap = 1; gap < 10; ++gap) {
            std::vector<ByteBuffer> ret = random_str(src, patten, gap);
            std::vector<ByteBuffer> split_1 = ret[0].split(patten);
            std::vector<ByteBuffer> split_2 = ret[1].split(patten);

            ssize_t split1_len = 0, split2_len = 0;
            for (std::size_t is = 0; is < split_1.size(); ++is) {
                split1_len += split_1[is].data_size();
            }
            for (std::size_t is = 0; is < split_2.size(); ++is) {
                split2_len += split_2[is].data_size();
            }
            //cout << "i: " << i << " gap: " << gap << endl;
            ASSERT_EQ(split1_len, src.data_size());
            ASSERT_EQ(split2_len, src.data_size()); // 计算分割后总的字符串是否与源字符串等长

            auto iter = src.begin(); // 比较分割后的内容与原内容是否一致
            for (std::size_t ib = 0; ib < split_1.size(); ++ib) {
                for (ssize_t is = 0; is < split_1[ib].data_size(); ++is) {
                    ASSERT_EQ(split_1[ib][is], *iter);
                    ++iter; 
                }
            }
            
            iter = src.begin();
            for (std::size_t ib = 0; ib < split_2.size(); ++ib) {
                for (ssize_t is = 0; is < split_2[ib].data_size(); ++is) {
                    ASSERT_EQ(split_2[ib][is], *iter);
                    ++iter; 
                }
            }

            // 测试替换
            ByteBuffer rep1 = ret[0].replace(patten, replace_str);
            ByteBuffer rep2 = ret[1].replace(patten, replace_str);

            split_1 = rep1.split(replace_str);
            split_2 = rep2.split(replace_str);
            
            split1_len = 0, split2_len = 0;
            for (std::size_t is = 0; is < split_1.size(); ++is) {
                split1_len += split_1[is].data_size();
            }
            for (std::size_t is = 0; is < split_2.size(); ++is) {
                split2_len += split_2[is].data_size();
            }
            ASSERT_EQ(split1_len, src.data_size());
            ASSERT_EQ(split2_len, src.data_size()); // 计算分割后总的字符串是否与源字符串等长
            
            iter = src.begin(); // 比较分割后的内容与原内容是否一致
            for (std::size_t ib = 0; ib < split_1.size(); ++ib) {
                for (ssize_t is = 0; is < split_1[ib].data_size(); ++is) {
                    ASSERT_EQ(split_1[ib][is], *iter);
                    ++iter; 
                }
            }
            
            iter = src.begin();
            for (std::size_t ib = 0; ib < split_2.size(); ++ib) {
                for (ssize_t is = 0; is < split_2[ib].data_size(); ++is) {
                    ASSERT_EQ(split_2[ib][is], *iter);
                    ++iter; 
                }
            }

        }
    }
    // 正则表达式测试
    ByteBuffer buff, patten;

    buff.clear();
    patten.clear();
    patten.write_string("<(.*)>(.*)</(\\1)>");
    buff.write_string("123<xml>value</xml>456<widget>center</widget>hahaha<vertical>window</vertical>the end");

    auto ret = buff.match(patten);
    ASSERT_EQ(ret.size(), static_cast<std::size_t>(3));
    ASSERT_EQ(ret[0].str(), std::string("<xml>value</xml>"));
    ASSERT_EQ(ret[1].str(), std::string("<widget>center</widget>"));
    ASSERT_EQ(ret[2].str(), std::string("<vertical>window</vertical>"));
}

}  // namespace
}  // namespace project
}  // namespace my

// ./test1 -h 查看参数说明
// ./test1 --gtest_list_tests 查看用例
// ./test1 运行所有用例
// ./test1 --gtest_filter=TestClass.Testname1 运行指定用例
// ./test1 --gtest_filter='TestClass.*' 使用通配符
// ./test1 --gtest_filter=-TestClass.Testname1 排除指定用例

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    // int total_size = ::testing::UnitTest::GetInstance()->total_test_suite_count();
    // ::testing::UnitTest::GetInstance() ;
    // return 0;
    return RUN_ALL_TESTS();
}