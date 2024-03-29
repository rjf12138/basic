#include "wejson.h"
#include "gtest/gtest.h"

using namespace basic;
//////////////////////////////////////////////////
void random_str(int setlen, ByteBuffer &buff1, ByteBuffer &buff2)
{
    buff1.clear();
    buff2.clear();

    int length = rand() % setlen +1;
    buff1.write_int8('"');

    for (int i = 0; i < length - 1; ++i) {
        int8_t ch = static_cast<int8_t>(rand() % 256);
        
        if (ch == 0) {
            buff1.write_int8('0');
            buff2.write_int8('0');
        } else if (ch == '"') {
            buff1.write_int8('\\');
            buff2.write_int8('\\');
            buff1.write_int8('"');
            buff2.write_int8('"');
        } else if (ch == '\\') {
            buff1.write_int8('\\');
            buff2.write_int8('\\');
            buff1.write_int8('\\');
            buff2.write_int8('\\');
        } else {
            buff1.write_int8(ch);
            buff2.write_int8(ch);
        }
    }
    buff1.write_int8('"');

    return ;
}

bool str_compare(ByteBuffer &buff1, ByteBuffer &buff2)
{
    std::string str;
    buff2.read_string(str);
    JsonString js_str_t1(str);
    bool ret = (str.length() == js_str_t1.to_string().length());
    if (ret == true) {
        ret = (str == js_str_t1.to_string());
    }

    if (ret == false) {
        LOG_GLOBAL_DEBUG("Test_Str1: %s", str.c_str());
        return false;
    }

    JsonString js_str_t2(str.c_str());
    ret = (str.length() == js_str_t2.to_string().length());
    if (ret == true) {
        ret = (str == js_str_t2.to_string());
    }
    
    if (ret == false) {
        LOG_GLOBAL_DEBUG("Test_Str2: %s", str.c_str());
        return false;
    }

    JsonString js_str_t3;
    
    ByteBuffer::iterator bbegin = buff1.begin();
    ByteBuffer::iterator bend = buff1.end();

    js_str_t3.parse(bbegin, bend);
    ret = (str.length() == js_str_t3.to_string().length());
    if (ret == true) {
        ret = (str == js_str_t3.to_string());
    }
    
    if (ret == false) {
        LOG_GLOBAL_DEBUG("Test_Str3: %s", str.c_str());
        return false;
    }

    return ret;
}

//////////////////////////////////////////////////

namespace my {
namespace project {
namespace {

#define MUTITHREAD 1

class WeJson_Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }
};

bool test_number(double val, std::string str_val)
{
//    fprintf(stdout, "double: %lf, str_double: %s\n", val, str_val.c_str());
    JsonNumber json_number(val);
    std::ostringstream ostr;
    ostr << json_number;
    if (std::stod(ostr.str().c_str()) == std::stod(str_val.c_str())) {
        return true;
    }
    fprintf(stdout, "double: %lf, str_double: %s, convert_double: %s\n", val, str_val.c_str(), ostr.str().c_str());
    return false;
}

bool test_parse_number(double val, std::string str_val)
{
    ByteBuffer buff;
    buff.write_string(str_val, str_val.length());
    JsonNumber json_number;
    auto str_number_begin = buff.begin();
    auto str_number_end = buff.end();
    json_number.parse(str_number_begin, str_number_end);

    if (std::stod(json_number.to_string().c_str()) == std::stod(str_val.c_str())) {
        return true;
    }

    fprintf(stdout, "double: %lf, str_double: %s, read_double: %s\n", val, str_val.c_str(), json_number.to_string().c_str());
    return false;
}

TEST_F(WeJson_Test, NUMBER_TEST)
{
    JsonNumber json_number_test_1(1.1214);
    JsonNumber json_number_test_2(12);
    JsonNumber json_number_test_3(json_number_test_1.to_double());
    JsonNumber json_number_test_4(json_number_test_2.to_double());

    ASSERT_EQ(json_number_test_1.to_string(), "1.1214");
    ASSERT_EQ(json_number_test_2.to_string(), "12");
    ASSERT_EQ(json_number_test_1.to_string(), json_number_test_3.to_string());
    ASSERT_EQ(json_number_test_2.to_string(), json_number_test_4.to_string());

    ASSERT_EQ(test_number(-10000.001, "-10000.001"), true);
    ASSERT_EQ(test_number(-10000, "-10000"), true);
    ASSERT_EQ(test_number(-10000.0000, "-10000"), true);
    ASSERT_EQ(test_number(10000.0000, "10000"), true);
    ASSERT_EQ(test_number(10000.0000, "10000"), true);
    ASSERT_EQ(test_number(10000.00001, "10000.00001"), true);
    ASSERT_EQ(test_number(0.00001, "0.00001"), true);
    ASSERT_EQ(test_number(0, "0"), true);
    
    ASSERT_EQ(test_parse_number(-10000.001, "-10000.001"), true);
    ASSERT_EQ(test_parse_number(-10000, "-10000"), true);
    ASSERT_EQ(test_parse_number(-10000.0000, "-10000"), true);
    ASSERT_EQ(test_parse_number(10000.0000, "10000"), true);
    ASSERT_EQ(test_parse_number(10000.0000, "10000"), true);
    ASSERT_EQ(test_parse_number(10000.00001, "10000.00001"), true);
    ASSERT_EQ(test_parse_number(10000.0000, "+10000"), true);
    ASSERT_EQ(test_parse_number(10000.00001, "+10000.00001"), true);
    ASSERT_EQ(test_parse_number(0.00001, "0.00001"), true);
    ASSERT_EQ(test_parse_number(0.00001, "+0.00001"), true);
    ASSERT_EQ(test_parse_number(-0.00001, "-0.00001"), true);

    ASSERT_EQ(test_number(-0., "0"), true);
    ASSERT_EQ(test_number(+0000., "0"), true);
    ASSERT_EQ(test_number(+0000.00000000, "0"), true);

    // double 小数点后默认保留6位，最后一位会以第七位进一
    ASSERT_EQ(test_number(-0.123456789, "-0.123457"), true);
    ASSERT_EQ(test_number(+000.123456778, "+0.123457"), true);
    ASSERT_EQ(test_number(+0000.1234567890000, "+0.123457"), true);

    ASSERT_EQ(test_number(-0.123456789, "-0.123457"), true);
    try {
        ASSERT_EQ(test_parse_number(0, "03"), true);
    }catch (std::exception &e) {
        std::cout << "===============异常测试=================" << std::endl;
        std::cout << e.what() << std::endl;
        std::cout << "=======================================" << std::endl;
    }
}

TEST_F(WeJson_Test, STRING_TEST)
{
    ByteBuffer buff1, buff2;
    for (int i = 0;i < 190; ++i) {
        random_str(11560, buff1, buff2);
        ASSERT_EQ(str_compare(buff1, buff2), true);
    }
}

TEST_F(WeJson_Test, BooleanTest)
{
    ByteBuffer buff1;
    JsonBool jb1(true), jb2(false);

    ASSERT_EQ(jb1.to_string(), "true");
    ASSERT_EQ(jb2.to_string(), "false");

    buff1.write_string("true");
    auto bbegin = buff1.begin();
    auto bend = buff1.end();

    jb1.parse(bbegin, bend);
    ASSERT_EQ(jb1.to_string(), "true");

    buff1.clear();
    buff1.write_string("false");
    bbegin = buff1.begin();
    bend = buff1.end();

    jb1.parse(bbegin, bend);
    ASSERT_EQ(jb1.to_string(), "false");
}

TEST_F(WeJson_Test, NullTest)
{
    ByteBuffer buff1;
    JsonNull jn1;

    buff1.write_string("null");
    auto bbegin = buff1.begin();
    auto bend = buff1.end();

    jn1.parse(bbegin, bend);
    ASSERT_EQ(jn1.to_string(), "null");
}

TEST_F(WeJson_Test, EmptyObjectArrayTest)
{
    WeJson js("{}");
    WeJson arr("[]");
    JsonObject obj = js.get_object();
    obj.add("Array", arr);
    JsonArray test_arr = obj["Array"];
}

TEST_F(WeJson_Test, ObjectArrayTest)
{
    try {
        WeJson js("");
    } catch (std::exception &e) {
        std::cout << "===============异常测试=================" << std::endl;
        std::cout << e.what() << std::endl;
        std::cout << "=======================================" << std::endl;
    }

    WeJson js("{}"), obj("{\"name\":\"Hello, World!\", \"tnull\": null, \"num\": 12.34, \"bool\": true}"), arr("[true, \"Hello\", null, 12.45]");
    ASSERT_EQ((js.get_object().begin() == js.get_object().end()), true);

    js.get_object().add("st,r", "te\nst,val   ue");
    js.get_object().add("bo ol", false);
    js.get_object().add("double", 12.3455);
    js.get_object().add("int", 12300);
    js.get_object().add("null", JsonNull());

    ASSERT_EQ(js.get_object()["st,r"], "te\nst,val   ue");
    ASSERT_EQ(js.get_object()["bo ol"], false);
    ASSERT_EQ(js.get_object()["double"], 12.3455);
    ASSERT_EQ(js.get_object()["int"], 12300);
    ASSERT_EQ(js.get_object()["null"], JsonNull());
    
    arr.get_array().add(obj);
    obj.get_object().add("arr", arr.get_array());
    arr.get_array().add(obj);
    obj.get_object().add("arr2", arr.get_array());
    js.get_object().add("test-obj", obj);
    js.get_object().add("test-arr", arr);

    ASSERT_EQ(js.get_object()["test-obj"]["num"], 12.34);
    ASSERT_EQ(js.get_object()["test-obj"]["bool"], true);
    ASSERT_EQ(js.get_object()["test-obj"]["name"], "Hello, World!");
    ASSERT_EQ(js.get_object()["test-obj"]["tnull"], "null");

    ASSERT_EQ(js.get_object()["test-obj"]["arr"][0], true);
    ASSERT_EQ(js.get_object()["test-obj"]["arr"][1], "Hello");
    ASSERT_EQ(js.get_object()["test-obj"]["arr"][2], JsonNull());
    ASSERT_EQ(js.get_object()["test-obj"]["arr"][3], 12.45);
    ASSERT_EQ(js.get_object()["test-obj"]["arr"][4]["name"], "Hello, World!");

    JsonNull jnval = js.get_object()["test-obj"]["tnull"];
    ASSERT_EQ(jnval.to_string(), "null");

    JsonNumber jnumval = js.get_object()["test-obj"]["num"]; // 不能直接将double类型的json值赋给double
    ASSERT_EQ(jnumval, 12.34);
    double numval = jnumval.to_double();
    ASSERT_EQ(numval, 12.34);

    JsonString jstrval = js.get_object()["test-obj"]["name"];
    ASSERT_EQ(jstrval, "Hello, World!");
    std::string strval = jstrval.to_string();
    ASSERT_EQ(strval, "Hello, World!");

    JsonBool jbval = js.get_object()["test-obj"]["bool"];
    ASSERT_EQ(jbval, true);
    bool bval = jbval.to_bool();
    ASSERT_EQ(bval, true);

    // 格式化以及非格式话json重新解析， 然后比较结果
    WeJson tmp1(js.to_string());
    WeJson tmp2(js.format_json());

    ASSERT_EQ(tmp1, tmp2);
    ASSERT_EQ(tmp1.to_string(), tmp2.to_string());

    WeJson cpy_js;
    cpy_js = js;
    
    ASSERT_EQ(cpy_js, js);
    cpy_js.get_object()["test-obj"]["num"] = 12.3;
    ASSERT_NE(cpy_js.get_object()["test-obj"]["num"], 12.34);
    ASSERT_EQ(cpy_js.get_object()["test-obj"]["num"], 12.3);


    WeJson obj1("{}"), arr1("[]");

    // 方式1
    obj1.get_object().add("str", "Hello"); // 对象是键值对
    obj1.get_object().add("int", 123);
    obj1.get_object().add("bool", false);
    obj1.get_object().add("null", JsonNull());

    arr1.get_array().add("Hello"); // 数组直接添加值
    arr1.get_array().add(123);
    arr1.get_array().add(false);
    arr1.get_array().add(JsonNull());

    // 迭代器
    std::cout << "============== Object ==============" << std::endl;
    for (auto iter = obj1.get_object().begin(); iter != obj1.get_object().end(); ++iter) {
        std::cout << iter->second.to_string() << std::endl;
    }
    std::cout << "============== Array ==============" << std::endl;
    for (auto iter = arr1.get_array().begin(); iter != arr1.get_array().end(); ++iter) {
        std::cout << iter->to_string() << std::endl;
    }
    std::cout << "=============== End =================" << std::endl;
    arr1.get_array().add(obj1.get_object());
    obj1.get_object().add("arr", arr1);
    // obj1.to_string();  // 输出后时压缩的
    // obj1.format_json();// 输出后会格式化

    // std::cout << obj1.format_json() << std::endl;

    obj1.get_object().erase("str"); // 对像移除关键字为str的元素
    arr1.get_array().erase(3); // 数组移除下标是 3 的元素
    // std::cout << obj1.format_json() << std::endl;
    // std::cout << arr1.format_json() << std::endl;

    WeJson data("{\"arr\":[\"Hello\",123,false,null,{\"bool\":false,\"int\":123,\"null\":null,\"str\":\"Hello\"}],\"bool\":false}");
    std::cout << data.format_json();
}

}
}
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}