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
    WeJson json("dfsfs"); // 解析失败抛出异常
} catch (exception &e) {
    std::cout << e.what() << std::endl;
}

// 创建成json object
void create_object(void);
// 创建成json array
void create_array(void);

// 返回当前对象类型
ValueType get_type(void)；

// 获取 json object
JsonObject& get_object(void);
// 获取 json array
JsonArray& get_array(void);

错误处理：
try {
    WeJson json("{}"); // 这是一个 object 对象
    json.get_array();  // 想要得到一个 array 对象（抛出异常）
} catch (exception &e) {
    std::cout << e.what() << std::endl;
}
```

2. JsonValue
```
JsonValue 是通用值类型，主要用于 JsonObject 和 JsonArray 
中保存 Json 中不同类型的值。

它可以和上面的值相互转换：
a. JsonObject, JsonNumber ... ---> JsonValue
条件: 不管 JsonValue 原先的值如何，直接覆盖

b. JsonValue ---> JsonObject, JsonNumber ...
条件： JsonValue 当前类型必须与要转换的类型是一致的。
例子：
try {
    JsonValue jval; // 默认是 JSON_NULL_TYPE 类型的
    JsonObject jobj = jval; // 报出异常
} catch (exception &e) {
    std::cerr << e.what() << std::endl;
}
```


3. JsonNumber (类型: JSON_NUMBER_TYPE)
```
// 转为字符串
string to_string(void) override;

// 获取double类型数据
double to_double(void)；
// 获取int类型数据，如果是浮点数会丢失精度
int to_int(void)；

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
// 返回字符串 "null"
string to_string(void) const;

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

6. JsonObject (JSON_OBJECT_TYPE)
```
// 以字符串形式返回
std::string to_string(void) override;

// 返回开始迭代器
JsonObject::iterator begin();
// 返回结束迭代器
JsonObject::iterator end();
// 查找元素
JsonObject::iterator find(const string &key);

// 删除元素
int erase(const std::string &key);
JsonObject::iterator erase(JsonObject::iterator &remove_iter);

// 添加元素
int add(const std::string &key, const JsonValue &value);

// 返回元素数量
int size(void);
// 清空元素
void clear(void);

// 重载操作符
// 判断是否相等
bool operator==(const JsonObject& rhs) const;
// 判断是否不想的
bool operator!=(const JsonObject& rhs) const;
// 赋值
JsonObject& operator=(const JsonObject &rhs);
// 根据key返回值
JsonValue& operator[](const string &key);

错误处理：
通过下标获取值:
try {
    JsonObject jobj;
    // 存入以个key： hello, value: hello的键值对
    jobj.add("first", "hello");
    // 访问不存在的 key，报错抛出异常
    JsonString jstr = jobj["second"];
} catch (exception &e) {
    std::cerr << e.what() << std::endl;
}
```

7. JsonArray (JSON_ARRAY_TYPE)
```
// 将 JsonArray 以字符串形式返回
string to_string(void) override;

// 返回开始迭代器
JsonArray::iterator begin();
// 返回结束迭代器
JsonArray::iterator end();

// 数组或是对象删除元素
JsonArray::iterator erase(const int &index);
JsonArray::iterator erase(const iterator &remove_iter);

// 当前添加元素
int add(const JsonValue &value);

// 返回元素数量
int size(void)；
// 清空元素
void clear(void)；

// 重载操作符
JsonValue& operator[](size_t key);
const JsonValue& operator[](const size_t key) const;
bool operator==(const JsonArray& rhs) const;
bool operator!=(const JsonArray& rhs) const; 
JsonArray& operator=(const JsonArray &rhs);

通过下标获取值:
try {
    JsonArray jarr;
    // 存入值 value: hello
    jobj.add("hello");
    // 访问不存在的下标，报错抛出异常
    JsonString jstr = jobj[23];
} catch (exception &e) {
    std::cerr << e.what() << std::endl;
}

```
