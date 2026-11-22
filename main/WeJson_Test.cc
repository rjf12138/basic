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
        unsigned char ch = static_cast<unsigned char>(rand() % 256);
        
        if (ch == '"') {
            buff1.write_int8('\\'); buff2.write_int8('\\');
            buff1.write_int8('"');  buff2.write_int8('"');
        } else if (ch == '\\') {
            buff1.write_int8('\\'); buff2.write_int8('\\');
            buff1.write_int8('\\'); buff2.write_int8('\\');
        } else if (ch < 0x20) {
            // 控制字符按 \uXXXX 转义写入(与 escape_json_string 规则一致)
            char buf[8] = {0};
            snprintf(buf, sizeof(buf), "\\u%04x", ch);
            for (char *p = buf; *p != '\0'; ++p) {
                buff1.write_int8(static_cast<int8_t>(*p));
                buff2.write_int8(static_cast<int8_t>(*p));
            }
        } else {
            buff1.write_int8(static_cast<int8_t>(ch));
            buff2.write_int8(static_cast<int8_t>(ch));
        }
    }
    buff1.write_int8('"');

    return ;
}

bool str_compare(ByteBuffer &buff1, ByteBuffer &buff2)
{
    std::string str;
    buff2.read_string(str); // 期望的转义文本

    ByteBuffer::iterator bbegin = buff1.begin();
    ByteBuffer::iterator bend = buff1.end();

    // 解析 buff1(带引号的 JSON 字符串),再序列化,应与转义文本一致
    JsonString js_str_t3;
    js_str_t3.parse(bbegin, bend);
    bool ret = (js_str_t3.to_string() == str);
    if (ret == false) {
        LOG_GLOBAL_DEBUG("Test_Str: %s", str.c_str());
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
    }

    void TearDown() override {
    }
};

bool test_number(double val, std::string str_val)
{
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

// 测试 JsonNumber::parse 对边界情况的处理（确保不越界读取）
TEST_F(WeJson_Test, NumberParseBoundary)
{
    ByteBuffer buff;
    JsonNumber num;
    
    // 单独的 "0"
    buff.write_string("0");
    auto begin0 = buff.begin();
    auto end0 = buff.end();
    num.parse(begin0, end0);
    ASSERT_EQ(num.to_double(), 0.0);
    
    // "0." 后面无更多内容
    buff.clear();
    buff.write_string("0.");
    auto begin = buff.begin();
    auto end = buff.end();
    num.parse(begin, end);
    ASSERT_EQ(num.to_double(), 0.0);
    
    // 负数零
    buff.clear();
    buff.write_string("-0.5");
    auto begin_neg = buff.begin();
    auto end_neg = buff.end();
    num.parse(begin_neg, end_neg);
    ASSERT_EQ(num.to_double(), -0.5);
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
    
    // 测试 operator==
    JsonBool jbt(true), jbf(false);
    ASSERT_EQ(jbt, true);
    ASSERT_EQ(jbf, false);
    ASSERT_NE(jbt, false);
    ASSERT_NE(jbf, true);
    ASSERT_EQ(jbt, JsonBool(true));
    ASSERT_NE(jbt, JsonBool(false));
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
    
    // 测试 JsonNull::operator== （修复后）
    JsonNull jn2, jn3;
    ASSERT_EQ(jn1, jn2);
    ASSERT_EQ(jn2, jn3);
}

// 测试 JsonArray::operator== (修复后不应所有非空数组都判等)
TEST_F(WeJson_Test, ArrayEqualityTest)
{
    WeJson arr1("[]");
    WeJson arr2("[]");
    ASSERT_EQ(arr1, arr2);
    
    WeJson arr3("[1, 2, 3]");
    WeJson arr4("[1, 2, 3]");
    ASSERT_EQ(arr3, arr4);
    
    WeJson arr5("[1, 2, 3]");
    WeJson arr6("[1, 2, 4]");
    ASSERT_NE(arr5, arr6);  // 这应该在修复后正常工作
    
    WeJson arr7("[1, 2, 3]");
    WeJson arr8("[1, 2]");
    ASSERT_NE(arr7, arr8);
    
    WeJson arr9("[\"hello\", true]");
    WeJson arr10("[\"hello\", false]");
    ASSERT_NE(arr9, arr10);
    
    WeJson arr11("[1, \"two\", true, null]");
    WeJson arr12("[1, \"two\", true, null]");
    ASSERT_EQ(arr11, arr12);
}

TEST_F(WeJson_Test, EmptyObjectArrayTest)
{
    WeJson js("{}");
    WeJson arr("[]");
    JsonObject obj = js.get_object();
    obj.add("Array", arr);
    JsonArray test_arr = obj["Array"];
}

// 测试带注释的 JSON 解析
TEST_F(WeJson_Test, JsonWithComments)
{
    std::string json_with_comments = 
        "{\n"
        "  // This is a comment\n"
        "  \"name\": \"John\",\n"
        "  \"age\": 30,  // inline comment\n"
        "  // This is another comment\n"
        "  \"city\": \"NYC\"\n"
        "}";
    WeJson js(json_with_comments);
    ASSERT_EQ(js.get_object()["name"], "John");
    ASSERT_EQ(js.get_object()["age"], 30);
    ASSERT_EQ(js.get_object()["city"], "NYC");
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
    ASSERT_EQ(js.get_object()["test-obj"]["tnull"], JsonNull());

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

    obj1.get_object().erase("str"); // 对像移除关键字为str的元素
    arr1.get_array().erase(3); // 数组移除下标是 3 的元素

    WeJson data("{\"arr\":[\"Hello\",123,false,null,{\"bool\":false,\"int\":123,\"null\":null,\"str\":\"Hello\"}],\"bool\":false}");
    std::cout << data.format_json() << std::endl;

    JsonObject test_data_object = data.get_object();
    for (auto iter = test_data_object.begin(); iter != test_data_object.end(); ++iter) {
        std::cout << "obj_key: " << iter->first << std::endl;
        if (iter->second.type() == JSON_ARRAY_TYPE) {
            for (auto iter_arr = iter->second.to_array().begin(); iter_arr != iter->second.to_array().end(); ++iter_arr) {
                std::cout << "arr_val: " << iter_arr->to_string() << std::endl; 
            }
        } else {
            std::cout << "obj_val: " << iter->second.to_string() << std::endl; 
        }
    }
}

// 测试嵌套 JSON 解析后序列化一致
TEST_F(WeJson_Test, NestedJsonRoundTrip)
{
    std::string json_str = 
        "{\"name\":\"test\","
        "\"data\":{\"items\":[1,2,3],\"active\":true},"
        "\"tags\":[\"fast\",\"reliable\"]}";
    
    WeJson js(json_str);
    std::string output = js.to_string();
    
    // 重新解析并比较
    WeJson js2(output);
    ASSERT_EQ(js, js2);
}

// 测试 create_object / create_array
TEST_F(WeJson_Test, CreateObjectArray)
{
    WeJson js;
    js.create_object();
    ASSERT_EQ(js.get_type(), JSON_OBJECT_TYPE);
    js.get_object().add("key", "value");
    ASSERT_EQ(js.get_object()["key"], "value");
    
    WeJson arr;
    arr.create_array();
    ASSERT_EQ(arr.get_type(), JSON_ARRAY_TYPE);
    arr.get_array().add(123);
    ASSERT_EQ(arr.get_array()[0], 123);
}

// 测试 JsonValue 类型转换操作符
TEST_F(WeJson_Test, JsonValueTypeConversion)
{
    JsonValue val;
    
    // null 转换
    JsonNull null_val = val;
    ASSERT_EQ(null_val.to_string(), "null");
    
    // 类型不匹配时抛异常
    val = 12.34;
    bool exception_thrown = false;
    try {
        JsonBool bad = val;  // val 是 number 不是 bool
    } catch (std::runtime_error &e) {
        exception_thrown = true;
    }
    ASSERT_TRUE(exception_thrown);
}

// 测试 JsonObject 查找和擦除
TEST_F(WeJson_Test, JsonObjectFindErase)
{
    WeJson js("{}");
    js.get_object().add("a", 1);
    js.get_object().add("b", 2);
    js.get_object().add("c", 3);
    
    auto it = js.get_object().find("b");
    ASSERT_NE(it, js.get_object().end());
    ASSERT_EQ(it->first, "b");
    ASSERT_EQ(it->second, 2);
    
    auto not_found = js.get_object().find("d");
    ASSERT_EQ(not_found, js.get_object().end());
    
    int erased = js.get_object().erase("b");
    ASSERT_EQ(erased, 1);
    ASSERT_EQ(js.get_object().size(), 2);
    ASSERT_EQ(js.get_object().find("b"), js.get_object().end());
}

// 测试 JsonValue to_int / to_double
TEST_F(WeJson_Test, JsonValueToNumber)
{
    JsonValue val(12.34);
    ASSERT_EQ(val.to_int(), 12);
    ASSERT_EQ(val.to_double(), 12.34);
    
    // 非 number 类型抛异常
    JsonValue str_val("hello");
    bool throw_on_int = false;
    try { str_val.to_int(); } catch (std::exception &) { throw_on_int = true; }
    ASSERT_TRUE(throw_on_int);
    
    bool throw_on_double = false;
    try { str_val.to_double(); } catch (std::exception &) { throw_on_double = true; }
    ASSERT_TRUE(throw_on_double);
}

// 测试 format_json 在嵌套结构上正确格式化
TEST_F(WeJson_Test, FormatJsonComplex)
{
    std::string json_str = "{\"name\":\"John\",\"age\":30,\"hobbies\":[\"reading\",\"coding\"]}";
    WeJson js(json_str);
    std::string formatted = js.format_json();
    ASSERT_NE(formatted, "");
    ASSERT_NE(formatted.find("John"), std::string::npos);
    ASSERT_NE(formatted.find("reading"), std::string::npos);
}

// 测试字符串转义/反转义往返一致,序列化输出为合法 JSON
TEST_F(WeJson_Test, StringEscapeRoundTrip)
{
    WeJson js("{}");
    js.get_object().add("quote", "a\"b\\c");
    js.get_object().add("ctrl", "line1\nline2\ttab");
    js.get_object().add("unicode", std::string("\x01\x02\x1f", 3));

    std::string out = js.to_string();

    // 序列化输出不应包含未转义的控制字符
    for (char c : out) {
        ASSERT_TRUE(static_cast<unsigned char>(c) >= 0x20);
    }

    // 重新解析应得到相等的对象
    WeJson js2(out);
    ASSERT_EQ(js2, js);
    ASSERT_EQ(js2.get_object()["quote"], "a\"b\\c");
    ASSERT_EQ(js2.get_object()["ctrl"], "line1\nline2\ttab");
    ASSERT_EQ(js2.get_object()["unicode"], std::string("\x01\x02\x1f", 3));
}

// 测试非法 JSON 输入能被检测出来(不再静默接受)
TEST_F(WeJson_Test, InvalidJsonDetection)
{
    bool thrown = false;

    // 非法数字:两个小数点
    thrown = false;
    try { WeJson js("{\"n\":1.2.3}"); } catch (std::exception &e) { thrown = true; }
    ASSERT_TRUE(thrown);

    // 键后缺冒号和值
    thrown = false;
    try { WeJson js("{\"x\"}"); } catch (std::exception &e) { thrown = true; }
    ASSERT_TRUE(thrown);

    // 冒号后缺值
    thrown = false;
    try { WeJson js("{\"a\":}"); } catch (std::exception &e) { thrown = true; }
    ASSERT_TRUE(thrown);

    // 未闭合对象
    thrown = false;
    try { WeJson js("{\"a\":1"); } catch (std::exception &e) { thrown = true; }
    ASSERT_TRUE(thrown);

    // 尾部残留
    thrown = false;
    try { WeJson js("{} abc"); } catch (std::exception &e) { thrown = true; }
    ASSERT_TRUE(thrown);

    // 行注释到 EOF(无换行)不应崩溃,应正常解析
    thrown = false;
    try { WeJson js("{} // trailing comment"); } catch (std::exception &e) { thrown = true; }
    ASSERT_FALSE(thrown);
}

// 测试 JsonValue 类型错误访问抛异常而非崩溃
TEST_F(WeJson_Test, JsonValueTypeSafety)
{
    // 非对象类型调用 operator[] 应抛异常
    JsonValue jv(1);
    bool thrown = false;
    try { jv["k"] = 2; } catch (std::exception &e) { thrown = true; }
    ASSERT_TRUE(thrown);

    thrown = false;
    try { jv.to_object(); } catch (std::exception &e) { thrown = true; }
    ASSERT_TRUE(thrown);

    thrown = false;
    try { jv.to_array(); } catch (std::exception &e) { thrown = true; }
    ASSERT_TRUE(thrown);

    // 自赋值安全
    JsonValue s("hello");
    s = s;
    ASSERT_EQ(s.to_string(), "hello");

    // nullptr 字符串构造安全
    JsonValue nv(static_cast<const char*>(nullptr));
    ASSERT_EQ(nv.to_string(), "");
}

}
}
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}