#### WeJson 用法

1. 解析输入json字符数据，创建json数组和对象
```
enum ValueType {
    JSON_UNKNOWN_TYPE = -1000,
    JSON_NUMBER_TYPE = 10001,
    JSON_BOOL_TYPE = 10002,
    JSON_NULL_TYPE = 10003,
    JSON_STRING_TYPE = 10004,
    JSON_ARRAY_TYPE = 10005,
    JSON_OBJECT_TYPE = 10006
};

WeJson(void);  // 默认类型是 JSON_NULL_TYPE
WeJson(const std::string &json); // 根据解析结果分为 JSON_ARRAY_TYPE 或 JSON_OBJECT_TYPE
WeJson(const ByteBuffer &data); // 根据解析结果分为 JSON_ARRAY_TYPE 或 JSON_OBJECT_TYPE

// 解析保存在ByteBuffer的数据
virtual int parse(const ByteBuffer &data); // 根据解析结果分为 JSON_ARRAY_TYPE 或 JSON_OBJECT_TYPE
// 解析保存在string中的数据
virtual int parse(const string &data); // 根据解析结果分为 JSON_ARRAY_TYPE 或 JSON_OBJECT_TYPE

解析错误处理：
try {
    WeJson json("{}");
} catch (exception &e) {
    std::cout << e.what() << std::endl;
}

// 创建成json object
void create_object(void);
// 创建成json array
void create_array(void);
```

2. 获取对象/数组
```
// 返回当前对象类型
ValueType get_type(void)；

// 构建成object
JsonObject& get_object(void);
// 构建成array
JsonArray& get_array(void);

错误处理：
try {
    WeJson json("{}"); // 这是一个 object 对象
    json.get_array();  // 想要得到一个 array 对象（抛出异常）
} catch (exception &e) {
    std::cout << e.what() << std::endl;
}
```

3. JsonNumber (类型: JSON_NUMBER_TYPE)
```
// 转为字符串
string to_string(void) override;

// 获取double类型数据
double to_double(void)；
// 获取int类型数据，会丢失精度
int to_int(void)；
// 支持与 double 类型转换
operator double()；

// 比较
bool operator==(const JsonNumber& rhs) const;
bool operator!=(const JsonNumber& rhs) const;
bool operator==(const double& rhs) const;
bool operator!=(const double& rhs) const;

// 赋值
JsonNumber& operator=(JsonNumber rhs);
```

4. JsonBool (JSON_BOOL_TYPE)
```
// 转成字符串
string to_string(void) const;
// 获取 bool 值
bool to_bool(void) const

// 比较
bool operator==(const JsonBool& rhs) const;
bool operator!=(const JsonBool& rhs) const;
bool operator==(const bool& rhs) const;
bool operator!=(const bool& rhs) const;

// 赋值
JsonBool& operator=(JsonBool rhs);
```

5. JsonNull (JSON_NULL_TYPE)
```
string to_string(void) const; // 返回字符串 "null"

// 比较
bool operator==(const JsonNull& rhs) const;
bool operator!=(const JsonNull& rhs) const;

// 赋值
JsonNull& operator=(JsonNull rhs);

```

6. JsonString (JSON_STRING_TYPE)
```
// 转为字符串
virtual string to_string(void) const;

// 比较
bool operator==(const JsonString& rhs) const;
bool operator!=(const JsonString& rhs) const;
bool operator==(const string& rhs) const;
bool operator!=(const string& rhs) const;

// 赋值
JsonString& operator=(JsonString rhs);
```

7. 