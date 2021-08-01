#ifndef __JSON_TYPE_H__
#define __JSON_TYPE_H__

#include "basic_head.h"
#include "byte_buffer.h"
#include "logger.h"

namespace basic {
// json中重要的分割字符
const vector<char> sperate_chars = {' ', '\r', '\n','\t','{', '}','[', ']',',',':','"'};

class JsonValue;

enum ValueType {
    JSON_UNKNOWN_TYPE = -1000,
    JSON_NUMBER_TYPE = 10001,
    JSON_BOOL_TYPE = 10002,
    JSON_NULL_TYPE = 10003,
    JSON_STRING_TYPE = 10004,
    JSON_ARRAY_TYPE = 10005,
    JSON_OBJECT_TYPE = 10006
};

enum NumberType {
    NONE_TYPE = -1001,
    INT32_TYPE = 10030,
    DOUBLE_TYPE = 10031,
};


class JsonType : public MsgRecord {
public:
    JsonType(void) {}
    virtual ~JsonType(void) {}

    // 检查当前位置的字符来判断接下来的是什么类型，具体参考doc中的资料
    static ValueType check_value_type(ByteBuffer::iterator &iter);

    // 解析遇到的类型，具体取决于check_value_type()返回的类型和继承该类的实现
    virtual ByteBuffer::iterator parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos){return ByteBuffer::iterator();}
    
    // 将json值反序列化为字符串输出， 没有格式化
    virtual string generate(void) { return "";}
};

// json 数值类型
class JsonNumber : public JsonType {
    friend ostream& operator<<(ostream &os, JsonNumber &rhs);
public:
    explicit JsonNumber(void);
    explicit JsonNumber(const double &val);
    ~JsonNumber(void);

    virtual ByteBuffer::iterator parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos) override;
    virtual string generate(void) override;

    double value(void) const  {return value_;}
    operator double() {return value_;}

    bool operator==(const JsonNumber& rhs) const;
    bool operator!=(const JsonNumber& rhs) const;
    bool operator==(const double& rhs) const;
    bool operator!=(const double& rhs) const;
    JsonNumber& operator=(JsonNumber rhs);

private:
    double        value_;
};

// json 布尔类型
class JsonBool : public JsonType {
    friend ostream& operator<<(ostream &os, JsonBool &rhs);
public:
    explicit JsonBool(void);
    explicit JsonBool(bool val);
    ~JsonBool(void);

    virtual ByteBuffer::iterator parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos) override;
    virtual string generate(void) override;

    bool value(void) const  {return value_;}
    operator bool() {return value_;}

    bool operator==(const JsonBool& rhs) const;
    bool operator!=(const JsonBool& rhs) const;
    bool operator==(const bool& rhs) const;
    bool operator!=(const bool& rhs) const;
    JsonBool& operator=(JsonBool rhs);

private:
    bool value_;
};

// json null 类型
class JsonNull : public JsonType {
    friend ostream& operator<<(ostream &os, JsonNull &rhs);
public:
    explicit JsonNull(void);
    explicit JsonNull(string val);
    ~JsonNull(void);

    virtual ByteBuffer::iterator parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos) override;
    virtual string generate(void) override;

    bool operator==(const JsonNull& rhs) const;
    bool operator!=(const JsonNull& rhs) const;
    JsonNull& operator=(JsonNull rhs);

private:
    string value_;
};

// json 字符串类型
class JsonString : public JsonType {
    friend ostream& operator<<(ostream &os, JsonString &rhs);
public:
    explicit JsonString(void);
    explicit JsonString(string val);
    explicit JsonString(const char *val);
    ~JsonString(void);

    virtual ByteBuffer::iterator parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos) override;
    virtual string generate(void) override;

    string value(void) const  {return value_;}
    operator std::string() {return value_;}

    bool operator==(const JsonString& rhs) const;
    bool operator!=(const JsonString& rhs) const;
    bool operator==(const string& rhs) const;
    bool operator!=(const string& rhs) const;
    JsonString& operator=(JsonString rhs);

private:
    string value_;
};

// json 对象类型
class JsonObject : public JsonType {
public:
    friend ostream& operator<<(ostream &os, JsonObject &rhs);
    typedef std::map<string, JsonValue>::iterator iterator;
public:
    explicit JsonObject(void);
    ~JsonObject(void);

    // 序列化和反序列化
    virtual ByteBuffer::iterator parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos) override;
    virtual std::string generate(void) override;
    
    // 查找元素
    JsonObject::iterator find(const string &key);
    // 删除元素
    int erase(const std::string &key);
    JsonObject::iterator erase(JsonObject::iterator &remove_iter);
    // 当前类型为对象时添加元素
    int add(const std::string &key, const JsonValue &value);
    // 返回元素数量
    int size(void) const {return value_.size();}
    // 清空元素
    void clear(void) {value_.clear();}

    // 重载操作符
    bool operator==(const JsonObject& rhs) const;
    bool operator!=(const JsonObject& rhs) const;
    JsonObject& operator=(JsonObject rhs);
    JsonValue& operator[](const string &key);

public:
    map<string, JsonValue> value_;
};

// json 数组类型
class JsonArray : public JsonType {
public:
    friend ostream& operator<<(ostream &os, JsonArray &rhs);
    typedef std::vector<JsonValue>::iterator iterator;
public:
    explicit JsonArray(void);
    ~JsonArray(void);

    // 序列化和反序列化
    virtual ByteBuffer::iterator parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos) override;
    virtual string generate(void) override;

    // 数组或是对象删除元素
    iterator erase(const int &index);
    iterator erase(const iterator &remove_iter);
    // 当前添加元素
    int add(const JsonValue &value);
    // 返回元素数量
    int size(void) const {return value_.size();}
    // 清空元素
    void clear(void) {value_.clear();}

    // 重载操作符
    JsonValue& operator[](size_t key);
    const JsonValue& operator[](const size_t key) const;

    bool operator==(const JsonArray& rhs) const;
    bool operator!=(const JsonArray& rhs) const; 

    JsonArray& operator=(JsonArray rhs);

public:
    vector<JsonValue> value_;
};

// json中转类型：可以安装当前存储的类型输出或是接收不同的类型
class JsonValue : public JsonType {
    friend JsonObject;
    friend JsonArray;
public:
    JsonValue(void);
    JsonValue(const JsonBool &value);
    JsonValue(const JsonNumber &value);
    JsonValue(const JsonString &value);
    JsonValue(const JsonObject &value);
    JsonValue(const JsonArray &value);
    JsonValue(const JsonNull &value);
    
    JsonValue(const bool &value);
    JsonValue(const int &value);
    JsonValue(const double &value);
    JsonValue(const string &value);
    JsonValue(const char *value);

    ~JsonValue(void);

    operator JsonNumber();
    operator JsonString();
    operator JsonBool();
    operator JsonObject();
    operator JsonArray();
    operator JsonNull();

    JsonValue& operator=(const JsonBool &val);
    JsonValue& operator=(const JsonNumber &val);
    JsonValue& operator=(const JsonString &val);
    JsonValue& operator=(const JsonObject &val);
    JsonValue& operator=(const JsonArray &val);
    JsonValue& operator=(const JsonNull &val);
    JsonValue& operator=(const JsonValue &val);

    bool operator==(const JsonValue& rhs) const;
    bool operator!=(const JsonValue& rhs) const;

    ValueType type(void) const {return type_;}
public:
    ValueType type_;
    JsonType *value_;
};

class WeJson : public JsonValue 
{
public:
    WeJson(void);
    WeJson(const string &json);
    WeJson(ByteBuffer &data);
    virtual ~WeJson(void);

    // 解析保存在ByteBuffer的数据
    virtual int parse(ByteBuffer &data);
    // 解析保存在string中的数据
    virtual int parse(const string &data);

    // 非格式化输出 json
    virtual string generate(void);
    // 格式化输出 json
    virtual string format_json(void);

    // 构建成object
    void create_object(void);
    // 构建成array
    void create_array(void);

    // 构建成object
    JsonObject& get_object(void);
    // 构建成array
    JsonArray& get_array(void);

private:
    virtual ByteBuffer::iterator parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos);
};

}

#endif