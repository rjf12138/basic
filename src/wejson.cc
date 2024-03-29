#include "wejson.h"
#include "debug.h"

namespace basic {

ValueType
JsonType::check_value_type(ByteBuffer::iterator &iter) 
{
    if (isdigit(*iter) || *iter == '-' || *iter == '+') {
        return JSON_NUMBER_TYPE;
    }

    switch (*iter)
    {
        case '{':
        {
            return JSON_OBJECT_TYPE;
        } break;
        case '[':
        {
            return JSON_ARRAY_TYPE;
        } break;
        case '"':
        {
            return JSON_STRING_TYPE;
        } break;
        case 't':
        {
            return JSON_BOOL_TYPE;
        }break;
        case 'f':
        {
            return JSON_BOOL_TYPE;
        }break;
        case 'n':
        {
            return JSON_NULL_TYPE;
        } break;
        default:
            break;
    }

    return JSON_UNKNOWN_TYPE;
}

std::string 
JsonType::value_type_to_string(ValueType type)
{
    switch(type)
    {
        case JSON_ARRAY_TYPE:
        {
            return "JSON_ARRAY_TYPE";
        } break;
        case JSON_BOOL_TYPE:
        {
            return "JSON_BOOL_TYPE";
        } break;
        case JSON_NULL_TYPE:
        {
            return  "JSON_NULL_TYPE";
        } break;
        case JSON_NUMBER_TYPE:
        {
            return "JSON_NUMBER_TYPE";
        } break;
        case JSON_OBJECT_TYPE:
        {
            return "JSON_OBJECT_TYPE";
        } break;
        case JSON_STRING_TYPE:
        {
            return "JSON_STRING_TYPE";
        } break;
        default:
        {
            return "JSON_UNKNOWN_TYPE";
        }
    }

    return "JSON_UNKNOWN_TYPE";
}
///////////////////////////////////////////////////////////

JsonNumber::JsonNumber(void)
: value_(0)
{

}

JsonNumber::JsonNumber(const double &val)
: value_(val)
{
}


JsonNumber::~JsonNumber(void) {}

ByteBuffer::iterator 
JsonNumber::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    ByteBuffer::iterator iter = value_start_pos;
    std::string str;
    bool is_negative = false;

    if (*iter == '-') {
        is_negative = true;
        ++iter;
    } else if (*iter == '+') {
        is_negative = false;
        ++iter;
    }

    if (iter != json_end_pos && *iter == '0') {
        if (isdigit(*(iter+1))) { // 除了小数和数值0之外，零不能作为第一个数
            throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR, "Zero can't be first number of integer!\n%s\n", dump_stack().c_str()));
        }
    }

    for (; iter != json_end_pos && *iter != ','; ++iter) {
        if (!isdigit(*iter) && *iter != '.') {
            break;
        }

        str += *iter;
    }

    value_ = stod(str) * (is_negative?-1:1);

    return iter;
}

std::string 
JsonNumber::to_string(void) const
{
    char buf[256] = {0};
    snprintf(buf, 256, "%lf", value_);

    int j = 255;
    for (; j >=0; --j) {
        if (buf[j] == '.') {
            break;
        }
    }

    for (int i = 255; j >= 0 && i >= j; --i) {
        if (buf[i] <= '9' && buf[i] > '0') {
            break;
        }
        buf[i] = '\0';
    }

    return buf;
}

std::ostream& 
operator<<(std::ostream &os, JsonNumber &rhs)
{
    os << rhs.to_string();

    return os;
}

bool
JsonNumber::operator==(const JsonNumber& rhs) const
{
   if (value_ == rhs.value_) {
       return true;
   }

    return false;
}
bool 
JsonNumber::operator!=(const JsonNumber& rhs) const
{
    return !(*this == rhs);
}

bool
JsonNumber::operator==(const double& rhs) const
{
   if (value_ == rhs) {
       return true;
   }

    return false;
}
bool 
JsonNumber::operator!=(const double& rhs) const
{
    return !(*this == rhs);
}

JsonNumber& 
JsonNumber::operator=(JsonNumber rhs)
{
    value_ = rhs.value_;
    return *this;
}

/////////////////////////////////////////////////////////////////

JsonBool::JsonBool(void)
{

}

JsonBool::JsonBool(bool val)
{
    value_ = val;
}
JsonBool::~JsonBool(void)
{

}

ByteBuffer::iterator 
JsonBool::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    ByteBuffer::iterator iter = value_start_pos;
    int read_size = 0;
    if (*iter == 't') {
        read_size = 4;
    } else if (*iter == 'f') {
        read_size = 5;
    }

    std::string str;
    for (int i = 0; iter != json_end_pos && i < read_size; ++iter, ++i) {
        str += *iter;
    }
    
    if (str == "true") {
        value_ = true;
    } else if (str == "false") {
        value_ = false;
    } else {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Expected true or false, but result is %s\n%s\n", str.c_str(), dump_stack().c_str()));
    }

    return iter;
}

std::string 
JsonBool::to_string(void) const
{
    return value_ == true? "true":"false";
}

std::ostream& operator<<(std::ostream &os, JsonBool &rhs)
{
    os << rhs.to_string();

    return os;
}

bool 
JsonBool::operator==(const JsonBool& rhs) const
{
    return value_ == rhs.value_?true:false;
}

bool 
JsonBool::operator!=(const JsonBool& rhs) const
{
    return !(*this == rhs);
}

bool 
JsonBool::operator==(const bool& rhs) const
{
    return value_ == rhs?true:false;
}

bool 
JsonBool::operator!=(const bool& rhs) const
{
    return !(*this == rhs);
}

JsonBool& 
JsonBool::operator=(JsonBool rhs)
{
    value_ = rhs.value_;

    return *this;
}


//////////////////////////////////////////////////////////////
JsonNull::JsonNull(void)
 : value_("null")
{

}

JsonNull::~JsonNull(void)
{}

ByteBuffer::iterator 
JsonNull::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    ByteBuffer::iterator iter = value_start_pos;
    int read_size = 4; // null is four bytes.

    std::string str;
    for (int i = 0; iter != json_end_pos && i < read_size; ++iter, ++i) {
        str += *iter;
    }

    if (str == "null") {
        value_ = str;
    } else {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Expected null, but result is %s\n%s\n", str.c_str(), dump_stack().c_str()));
    }


    return iter;
}

std::string 
JsonNull::to_string(void) const
{
    return "null";
}

std::ostream& operator<<(std::ostream &os, JsonNull &rhs)
{
    os << rhs.to_string();

    return os;
}

bool 
JsonNull::operator==(const JsonNull& rhs) const
{
    return (value_ == "null" ? true : false);
}

bool 
JsonNull::operator!=(const JsonNull& rhs) const
{
    return !(*this == rhs);
}


JsonNull& 
JsonNull::operator=(JsonNull rhs)
{
    value_ = rhs.value_;

    return *this;
}


///////////////////////////////////////////////////////////

JsonString::JsonString(void) {}
JsonString::JsonString(const std::string val): value_(val) {}
JsonString::JsonString(const char *val): value_(val) {};
JsonString::~JsonString(void) {}

ByteBuffer::iterator 
JsonString::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    ByteBuffer::iterator iter = value_start_pos;
    ++iter; // 此时 *iter 值为 ",指向字符串的第一个字符
    std::string str;
    str.clear();
    for (; iter != json_end_pos; ++iter) {
        if (*iter == '\\') { 
            str += *iter;
            ++iter; // '\' 为转义字符下一个字符不做解析
            str += *iter;
            continue;
        } else if (*iter == '"') {
            break;
        }   
        str += *iter;
    }
    
    if (iter == json_end_pos) // 因为字符解析是以'"'为结尾的，所以当遇到Buffer结尾时说明字符不是以'"'结尾的
    {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"String need to surround by \"\"\n%s\n", dump_stack().c_str()));
    }
    value_ = str;
    ++iter; //  iter 指向下一个字符
    return iter;
}

std::string 
JsonString::to_string(void) const
{
    return value_;
}

std::ostream& operator<<(std::ostream &os, JsonString &rhs)
{
    os << rhs.to_string();

    return os;
}

bool JsonString::operator==(const JsonString& rhs) const
{
    if (value_ != rhs.value_) {
        return false;
    }

    return true;
}

bool JsonString::operator!=(const JsonString& rhs) const
{
    return !(*this == rhs);
}

bool JsonString::operator==(const std::string& rhs) const
{
    if (value_ != rhs) {
        return false;
    }

    return true;
}

bool JsonString::operator!=(const std::string& rhs) const
{
    return !(*this == rhs);
}

JsonString& 
JsonString::operator=(JsonString rhs)
{
    value_ = rhs.value_;

    return *this;
}

///////////////////////////////////////////////////////////

JsonObject::JsonObject(void) {}
JsonObject::JsonObject(const JsonObject &jobj)
{
    *this = jobj;
}
JsonObject::~JsonObject(void) {}

ByteBuffer::iterator 
JsonObject::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    value_.clear();
    ByteBuffer::iterator iter = value_start_pos;
    ++iter; // 由指向'{'改到指向第一个键值对

    std::string value_name;
    bool flag = false;
    for (; iter != json_end_pos && *iter != '}'; ++iter) {
        ValueType ret_value_type = this->check_value_type(iter);
        if (ret_value_type == JSON_UNKNOWN_TYPE) {
            if (*iter != ',' && *iter != ':' && *iter != ']') {
                throw std::runtime_error( GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Unknown character in object: %c\n%s\n", *iter, dump_stack().c_str()));
            }
            continue;
        }

        JsonString json_str;
        if (flag == false && ret_value_type == JSON_STRING_TYPE) {
            iter = json_str.parse(iter, json_end_pos);
            value_name = json_str.to_string();
            flag = true;
            continue;
        }

        JsonValue vtc;
        switch (ret_value_type)
        {
            case JSON_OBJECT_TYPE:
            {
                JsonObject val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
            case JSON_ARRAY_TYPE:
            {
                JsonArray val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
            case JSON_STRING_TYPE:
            {
                JsonString val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
            case JSON_BOOL_TYPE:
            {
                JsonBool val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
            case JSON_NULL_TYPE:
            {
                JsonNull val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
            case JSON_NUMBER_TYPE:
            {
                JsonNumber val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
        default:
            break;
        }

        if (value_.find(value_name) == value_.end()) {
            value_[value_name] = vtc;
        } else {
            throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"The \"%s\" is already exists.\n%s\n", value_name.c_str(), dump_stack().c_str()));
        }
        flag = false;
        if (*iter == '}') { // 有些解析完就直接指向'}'， 如果不退出在回到循环会因值自增错过
            break;
        }
    }

    if (iter != json_end_pos && *iter == '}') {
        ++iter;
    }
    
    return iter;
}

std::string 
JsonObject::to_string(void) const
{
    std::ostringstream output_obj;
    output_obj << "{";
    for (auto iter = value_.begin(); iter != value_.end(); ++iter) {
        if (iter != value_.begin()) {
            output_obj << ",";
        }

        output_obj << "\"" << iter->first << "\"";
        output_obj << ":";
        switch (iter->second.type_)
        {
            case JSON_ARRAY_TYPE: {
                output_obj << dynamic_cast<JsonArray*>(iter->second.value_)->to_string();
            } break;
            case JSON_OBJECT_TYPE: {
                output_obj << dynamic_cast<JsonObject*>(iter->second.value_)->to_string();
            } break;
            case JSON_STRING_TYPE: {
                output_obj << "\"" << dynamic_cast<JsonString*>(iter->second.value_)->to_string() << "\"";
            } break;
            case JSON_NUMBER_TYPE: {
                output_obj << dynamic_cast<JsonNumber*>(iter->second.value_)->to_string();
            } break;
            case JSON_BOOL_TYPE: {
                output_obj << dynamic_cast<JsonBool*>(iter->second.value_)->to_string();
            } break;
            default: {
                output_obj << JsonNull().to_string();
            }
        }
    }
    output_obj << "}";

    return output_obj.str();
}

JsonObject::iterator
JsonObject::find(const std::string &key)
{
    auto iter = value_.find(key);
    return iter;
}

int JsonObject::erase(const std::string &key)
{
    return static_cast<int>(value_.erase(key));
}

JsonObject::iterator
JsonObject::erase(JsonObject::iterator &remove_iter)
{
    return value_.erase(remove_iter);
}

int JsonObject::add(const std::string &key, const JsonValue &value)
{
    value_[key] = value;
    return 0;
}

std::ostream& operator<<(std::ostream &os, JsonObject &rhs)
{
    os << rhs.to_string();

    return os;
}

bool 
JsonObject::operator==(const JsonObject& rhs) const
{
    if (this->value_.size() != rhs.value_.size()) {
        return false;
    }

    auto iter_this = this->value_.begin();
    auto iter_rhs = rhs.value_.begin();
    while (iter_rhs != rhs.value_.end() && iter_this != this->value_.end()) {
        if (iter_this->first != iter_rhs->first) {
            return false;
        }

        if (iter_this->second != iter_rhs->second) {
            return false;
        }
        iter_this++;
        iter_rhs++;
    }

    if (iter_rhs == rhs.value_.end() && iter_this == this->value_.end()) {
        return true;
    }

    return false;
}

bool 
JsonObject::operator!=(const JsonObject& rhs) const
{
    return !(*this == rhs);
}

JsonValue& 
JsonObject::operator[](const std::string &key)
{
    if (value_.find(key) == value_.end()) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"JsonObject: out of range.[key: %s]\n%s\n", key.c_str(), dump_stack().c_str()));
    }
    return value_[key];
}

JsonObject& 
JsonObject::operator=(const JsonObject &rhs)
{
    value_ = rhs.value_;

    return *this;
}

JsonObject::iterator 
JsonObject::begin()
{
    return value_.begin();
}

JsonObject::iterator 
JsonObject::end()
{
    return value_.end();
}

////////////////////////////////////////////////////////////

JsonArray::JsonArray(void){}
JsonArray::JsonArray(JsonArray &jarr)
{
    *this = jarr;
}
JsonArray::~JsonArray(void){}

ByteBuffer::iterator 
JsonArray::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    value_.clear();
    ByteBuffer::iterator iter = value_start_pos;
    ++iter;

    JsonValue ret_value;
    for (; iter != json_end_pos && *iter != ']'; ++iter) {
        ValueType ret_value_type = this->check_value_type(iter);
        if (ret_value_type == JSON_UNKNOWN_TYPE) {
            if (*iter != ',') {
                throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Unknown character in array: %c\n%s\n", *iter, dump_stack().c_str()));
            }
            continue;
        }
        JsonValue vtc;
        switch (ret_value_type)
        {
            case JSON_OBJECT_TYPE:
            {
                JsonObject val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
            case JSON_ARRAY_TYPE:
            {
                JsonArray val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
            case JSON_STRING_TYPE:
            {
                JsonString val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
            case JSON_BOOL_TYPE:
            {
                JsonBool val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
            case JSON_NULL_TYPE:
            {
                JsonNull val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
            case JSON_NUMBER_TYPE:
            {
                JsonNumber val;
                iter = val.parse(iter, json_end_pos);
                vtc = val;
            } break;
        default:
            break;
        }
        
        value_.push_back(vtc);
        if (iter != json_end_pos && *iter == ']') { // 有些解析玩就直接指向']'， 如果不退出在回到循环会因值自增错过
            break;
        }
    }
    if (iter == json_end_pos) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Array need to surround by []\n%s\n", dump_stack().c_str()));
    }
    ++iter;
    return iter;
}
std::string 
JsonArray::to_string(void) const
{
    std::ostringstream output_arr;
    output_arr << "[";
    for (auto iter = value_.begin(); iter != value_.end(); ++iter) {
        if (iter != value_.begin()) { // 每输出一个类型后跟一个','
            output_arr << ",";
        }
        switch (iter->type_)
        {
            case JSON_ARRAY_TYPE: {
                output_arr << dynamic_cast<JsonArray*>(iter->value_)->to_string();
            } break;
            case JSON_OBJECT_TYPE: {
                output_arr << dynamic_cast<JsonObject*>(iter->value_)->to_string();
            } break;
            case JSON_STRING_TYPE: {
                output_arr << "\"" << dynamic_cast<JsonString*>(iter->value_)->to_string() << "\"";
            } break;
            case JSON_NUMBER_TYPE: {
                output_arr << dynamic_cast<JsonNumber*>(iter->value_)->to_string();
            } break;
            case JSON_BOOL_TYPE: {
                output_arr << dynamic_cast<JsonBool*>(iter->value_)->to_string();
            } break;
            default: {
                output_arr << JsonNull().to_string();
            }
        }
    }

    output_arr <<  "]";
    return output_arr.str();
}


int JsonArray::add(const JsonValue &value)
{
    value_.push_back(value);

    return 1;
}

JsonArray::iterator 
JsonArray::erase(const int &index)
{
    auto remove_iter = value_.begin();
    // 移动迭代器index个距离
    advance(remove_iter, index);
    return value_.erase(remove_iter);
}

JsonArray::iterator 
JsonArray::erase(const JsonArray::iterator &remove_iter)
{
    return value_.erase(remove_iter);
}

std::ostream& operator<<(std::ostream &os, JsonArray &rhs)
{
    os << rhs.to_string();

    return os;
}

JsonValue& 
JsonArray::operator[](size_t key)
{
    if (key >= value_.size()) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"JsonArray: out of range[index: %d]\n%s", key, dump_stack().c_str()));
    }

    return value_[key];
}

bool 
JsonArray::operator==(const JsonArray& rhs) const
{
    if (value_.size() != rhs.value_.size()) {
        return false;
    }

    for (std::size_t i = 0; i > value_.size(); ++i) {
        if (value_[i].type_ != rhs.value_[i].type_) {
            return false;
        }
        if (value_[i] != rhs.value_[i]) {
            return false;
        }
    }

    return true;
}

bool JsonArray::operator!=(const JsonArray& rhs) const
{
    return !(*this == rhs);
}

JsonArray& 
JsonArray::operator=(const JsonArray &rhs)
{
    value_ = rhs.value_;

    return *this;
}

JsonArray::iterator 
JsonArray::begin()
{
    return value_.begin();
}

JsonArray::iterator 
JsonArray::end()
{
    return value_.end();
}

/////////////////////////////////////////////////////////////////

JsonValue::JsonValue(void) 
:type_(JSON_NULL_TYPE)
{
    value_ = new JsonNull();
}

JsonValue::JsonValue(const JsonBool &value)
    : type_(JSON_BOOL_TYPE)
{
    value_ = new JsonBool(value.to_bool());
}
    
JsonValue::JsonValue(const JsonNumber &value)
    : type_(JSON_NUMBER_TYPE)
{
    value_ = new JsonNumber(value.to_double());
}

JsonValue::JsonValue(const JsonString &value)
    : type_(JSON_STRING_TYPE)
{
    value_ = new JsonString(value.to_string());
}

JsonValue::JsonValue(const JsonObject &value)
    : type_(JSON_OBJECT_TYPE)
{
    JsonObject *object = new JsonObject();
    *object = value;
    value_ = object;
}

JsonValue::JsonValue(const JsonArray &value)
    : type_(JSON_ARRAY_TYPE)
{
    JsonArray *array = new JsonArray();
    *array = value;
    value_ = array;
}

JsonValue::JsonValue(const JsonNull &value)
    : type_(JSON_NULL_TYPE)
{
    value_ = new JsonNull();
}

JsonValue::JsonValue(const JsonValue &value)
    : type_(value.type_)
{
    this->copy(value, false);
}

JsonValue::JsonValue(const bool &value)
    :type_(JSON_BOOL_TYPE)
{
    value_ = new JsonBool(value);
}

JsonValue::JsonValue(const int &value)
    :type_(JSON_NUMBER_TYPE)
{
    value_ = new JsonNumber(value);
}

JsonValue::JsonValue(const double &value)
    :type_(JSON_NUMBER_TYPE)
{
    value_ = new JsonNumber(value);
}

JsonValue::JsonValue(const std::string &value)
    :type_(JSON_STRING_TYPE)
{
    value_ = new JsonString(value);
}

JsonValue::JsonValue(const char *value)
    :type_(JSON_STRING_TYPE)
{
    value_ = new JsonString(value);
}

JsonValue::~JsonValue(void) 
{
    if (value_ != nullptr) {
        delete value_;
        value_ = nullptr;
    }
}

JsonValue::operator JsonBool()
{
    if (type_ == JSON_BOOL_TYPE) {
        return JsonBool((dynamic_cast<JsonBool*>(value_))->to_bool());
    } else {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"value cast failed: current type is not a bool. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
}

JsonValue::operator JsonNumber()
{
    if (type_ == JSON_NUMBER_TYPE) {
        return JsonNumber(dynamic_cast<JsonNumber*>(value_)->to_double());
    } else {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"value cast failed: current type is not number. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
}
JsonValue::operator JsonString()
{
    if (type_ == JSON_STRING_TYPE) {
        return JsonString(dynamic_cast<JsonString*>(value_)->to_string());
    } else {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"value cast failed: current type is not std::string. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
}

JsonValue::operator JsonObject()
{
    if (type_ == JSON_OBJECT_TYPE) {
        JsonObject *js_obj = dynamic_cast<JsonObject*>(value_);
        return JsonObject(*js_obj);
    } else {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"value cast failed: current type is not object. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
}
JsonValue::operator JsonArray()
{
    if (type_ == JSON_ARRAY_TYPE) {
        JsonArray *js_arr = dynamic_cast<JsonArray*>(value_);
        return *js_arr;
    } else {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"value cast failed: current type is not array. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
}

JsonValue::operator JsonNull()
{
    if (type_ == JSON_NULL_TYPE) {
        return JsonNull();
    } else {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"value cast failed: current type is not null. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
}

JsonValue& 
JsonValue::operator=(const JsonBool &val)
{
    type_ = JSON_BOOL_TYPE;
    delete value_;
    value_ = new JsonBool(val.to_bool());

    return *this;
}

JsonValue& 
JsonValue::operator=(const JsonNumber &val)
{
    type_ = JSON_NUMBER_TYPE;
    delete value_;
    value_ = new JsonNumber(val.to_double());

    return *this;
}

JsonValue& 
JsonValue::operator=(const JsonString &val)
{
    type_ = JSON_STRING_TYPE;
    delete value_;
    value_ = new JsonString(val.to_string());

    return *this;
}

JsonValue& 
JsonValue::operator=(const JsonObject &val)
{
    type_ = JSON_OBJECT_TYPE;
    delete value_;
    JsonObject *object = new JsonObject();
    object->value_ = val.value_;
    value_ = object;

    return *this;
}

JsonValue& 
JsonValue::operator=(const JsonArray &val)
{
    type_ = JSON_ARRAY_TYPE;
    delete value_;
    JsonArray *array = new JsonArray();
    array->value_ = val.value_;
    value_ = array;

    return *this;
}

JsonValue& 
JsonValue::operator=(const JsonNull &val)
{
    type_ = JSON_NULL_TYPE;
    delete value_;
    value_ = new JsonNull();

    return *this;
}

JsonValue& 
JsonValue::operator=(const JsonValue &val)
{
    this->copy(val);
    return *this;
}

bool JsonValue::operator==(const JsonValue& rhs) const
{
    switch (type_)
    {
    case JSON_NULL_TYPE:
        return *dynamic_cast<JsonNull*>(value_) == *dynamic_cast<JsonNull*>(rhs.value_);
    case JSON_NUMBER_TYPE:
        return *dynamic_cast<JsonNumber*>(value_) == *dynamic_cast<JsonNumber*>(rhs.value_);
    case JSON_STRING_TYPE:
        return *dynamic_cast<JsonString*>(value_) == *dynamic_cast<JsonString*>(rhs.value_);
    case JSON_BOOL_TYPE:
        return *dynamic_cast<JsonBool*>(value_) == *dynamic_cast<JsonBool*>(rhs.value_);
    case JSON_ARRAY_TYPE:
        return *dynamic_cast<JsonArray*>(value_) == *dynamic_cast<JsonArray*>(rhs.value_);
    case  JSON_OBJECT_TYPE:
        return *dynamic_cast<JsonObject*>(value_) == *dynamic_cast<JsonObject*>(rhs.value_);
    default:
        break;
    }

    return true;
}
bool 
JsonValue::operator!=(const JsonValue& rhs) const
{
    return !(*this == rhs);
}

JsonValue& 
JsonValue::operator[](const std::string &key)
{
    if (type_ == JSON_OBJECT_TYPE) {
        return (*dynamic_cast<JsonObject*>(value_))[key];
    }

    throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"JsonValue::operator[%s]: out of range.\n%s\n", key.c_str(), dump_stack().c_str()));
}

JsonValue& 
JsonValue::operator[](const int &key)
{
    if (type_ == JSON_ARRAY_TYPE) {
        return (*dynamic_cast<JsonArray*>(value_))[key];
    }

    throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"JsonValue::operator[%d]: out of range.\n%s\n", key, dump_stack().c_str()));
}

void JsonValue::copy(const JsonValue &val, bool is_release)
{
    if (is_release == true) {
        delete value_;
    }

    type_ = val.type_;
    switch (type_)
    {
    case JSON_NULL_TYPE:{
        value_ = new JsonNull();
    } break;
    case JSON_NUMBER_TYPE:{
        JsonNumber *value = new JsonNumber(dynamic_cast<JsonNumber*>(val.value_)->to_double());
        value_ = value;
    } break;
    case JSON_STRING_TYPE:{
        JsonString *value = new JsonString(dynamic_cast<JsonString*>(val.value_)->to_string());
        value_ = value;
    } break;
    case JSON_BOOL_TYPE:{
        JsonBool *value = new JsonBool(dynamic_cast<JsonBool*>(val.value_)->to_bool());
        value_ = value;
    } break;
    case JSON_ARRAY_TYPE:{
        JsonArray *value = new JsonArray();
        *value = *dynamic_cast<JsonArray*>(val.value_);
        value_ = value;
    } break;
    case  JSON_OBJECT_TYPE:{
        JsonObject *value = new JsonObject();
        *value = *dynamic_cast<JsonObject*>(val.value_);
        value_ = value;
    } break;
    default:
        break;
    }
}

std::string 
JsonValue::to_string(void)
{
    switch (type_)
    {
        case JSON_ARRAY_TYPE: {
            return dynamic_cast<JsonArray*>(value_)->to_string();
        } break;
        case JSON_OBJECT_TYPE: {
            return dynamic_cast<JsonObject*>(value_)->to_string();
        } break;
        case JSON_STRING_TYPE: {
            return dynamic_cast<JsonString*>(value_)->to_string();
        } break;
        case JSON_NUMBER_TYPE: {
            return dynamic_cast<JsonNumber*>(value_)->to_string();
        } break;
        case JSON_BOOL_TYPE: {
            return dynamic_cast<JsonBool*>(value_)->to_string();
        } break;
        default: {
            return JsonNull().to_string();
        }
    }
    return JsonNull().to_string();
}

///////////////////////////////// WeJson //////////////////////////////////////////////////
WeJson::WeJson(void)
{

}
    
WeJson::WeJson(const std::string &json)
{
    this->parse(json);
}
    
WeJson::WeJson(const ByteBuffer &data)
{
    this->parse(data);
}

WeJson::~WeJson(void)
{
}

void 
WeJson::create_object(void)
{
    if (value_ != nullptr) {
        delete value_;
    }
    type_ = JSON_OBJECT_TYPE;
    value_ = new JsonObject();
}

void 
WeJson::create_array(void)
{
    if (value_ != nullptr) {
        delete value_;
    }
    type_ = JSON_ARRAY_TYPE;
    value_ = new JsonArray();
}

JsonObject& 
WeJson::get_object(void)
{
    if (type_ != JSON_OBJECT_TYPE) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"get object failed: current type is not a object. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
    return *dynamic_cast<JsonObject*>(value_);
}

JsonArray& 
WeJson::get_array(void)
{
    if (type_ != JSON_ARRAY_TYPE) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"get array failed: current type is not a array. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
    return *dynamic_cast<JsonArray*>(value_);
}

int 
WeJson::parse(const std::string &data)
{
    return this->parse(ByteBuffer(data));
}

int 
WeJson::parse(const ByteBuffer &buff)
{
    ByteBuffer simple_json_text;
    ByteBuffer::iterator start_pos;
    
    // 找json文本的开始，如果没有遇到'{'或'['开始的会返回失败
    for (start_pos = buff.begin(); start_pos != buff.end(); ++start_pos) {
        ValueType ret = JsonType::check_value_type(start_pos);
        if (ret == JSON_OBJECT_TYPE || ret == JSON_ARRAY_TYPE) {
            break;
        }
    }

    if (start_pos == buff.end()) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Can't find any json text!\n%s\n", dump_stack().c_str()));
    }

    bool quotation_marks = false;
    for (auto iter = start_pos; iter != buff.end(); ++iter) {
        if (quotation_marks == false && *iter == '"') {
            quotation_marks = true;
            simple_json_text.write_int8(*iter); // 写入字符串的第一个 "\""
            continue;
        }

        if (quotation_marks == true) {
            if (*iter == '\\'){ 
                simple_json_text.write_int8(*iter);
                ++iter; // '\' 为转义字符下一个字符不做解析
                simple_json_text.write_int8(*iter);
            } else if (*iter == '"') {
                quotation_marks = false;
            }
        }

        int i = 0;
        for (i = 0; i < 4; ++i) {
            if (sperate_chars[i] == *iter) {
                break;
            }
        }
        // 如果匹配到json的结构字符并且不是在字符串中，那么就忽略
        if (i < 4 && quotation_marks == false) {
            continue;
        }

        // "//" 以这个开头的注释也会被忽略，这个注释可以单独一行或是跟在json那行的末尾，不能出现在json文本那行的前面
        if (quotation_marks == false && *iter == '/' && *(iter+1) == '/') {
            for (; iter != buff.end(); ++iter) {
                if (*iter == '\n') {
                    break;
                }
            }

            if (*iter == '\n') {
                continue;
            }
        }

        simple_json_text.write_int8(*iter);
    }
    auto begin_json = simple_json_text.begin();
    auto end_json = simple_json_text.end();

    parse(begin_json, end_json);

    return 0;
}


ByteBuffer::iterator 
WeJson::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    ValueType ret_type = JsonType::check_value_type(value_start_pos);
    type_ = ret_type;
    switch (ret_type)
    {
    case JSON_ARRAY_TYPE: {
        this->create_array();
        return dynamic_cast<JsonArray*>(value_)->parse(value_start_pos, json_end_pos);
    }
    case  JSON_OBJECT_TYPE: {
        this->create_object();
        return dynamic_cast<JsonObject*>(value_)->parse(value_start_pos, json_end_pos);
    }
    default:
        std::string err_str = GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Unknown json type (object or array). [type: %s]", JsonType::value_type_to_string(type_));
        break;
    }

    return json_end_pos;
}


std::string 
WeJson::to_string(void)
{
    switch (type_)
    {
    case JSON_ARRAY_TYPE:
        return dynamic_cast<JsonArray*>(value_)->to_string();
    case  JSON_OBJECT_TYPE:
        return dynamic_cast<JsonObject*>(value_)->to_string();
    default:
        break;
    }

    return "";
}

std::string 
WeJson::format_json(void)
{
    if (type_ == JSON_OBJECT_TYPE || type_ == JSON_ARRAY_TYPE){
        std::string raw_json = this->to_string();

        bool bracket_flag = false;      // 中括号标志， 表示在数组内
        bool quotation_flag = false;    // 引号标志， 表示在字符串内
        int tab = 0;
        std::ostringstream oformat_json;
        for (std::size_t i = 0; i < raw_json.size(); ++i) {
            if (raw_json[i] == '{' || raw_json[i] == '[') {
                ++tab;
                oformat_json << raw_json[i];
                oformat_json << '\n';
                for (int j = 0; j < tab; ++j) {
                    oformat_json << '\t';
                }

                if (raw_json[i] == '[') {
                    bracket_flag = true;
                }
            } else if (raw_json[i] == '}' || raw_json[i] == ']') {
                oformat_json << '\n';
                --tab;
                for (int j = 0; j < tab; ++j) {
                    oformat_json << '\t';
                }
                oformat_json << raw_json[i];
                
                if (raw_json[i] == ']') {
                    bracket_flag = false;
                }
            } else if (quotation_flag == false && raw_json[i] == ',' && (raw_json[i+1] == '"' || bracket_flag == true)) {
                if (raw_json[i+1] == '"') {
                    quotation_flag = true;
                }
                oformat_json << raw_json[i];
                oformat_json << '\n';
                for (int j = 0; j < tab; ++j) {
                    oformat_json << '\t';
                }
            } else {
                // 计算转义符号，防止在字符串内出现 "\"," 这样的格式，确保是不是真的是字符串的结束位置
                if (raw_json[i] == '"' && (raw_json[i+1] == ',' || raw_json[i+1] == ':' || raw_json[i+1] == '}' || raw_json[i+1] == ']') && quotation_flag == true) {
                    int pos = static_cast<int>(i);
                    int cnt = 0;
                    for ( ; pos >= 0; --pos) {
                        if (raw_json[pos] == '\\') {
                            cnt++;
                        } else {
                            break;
                        }
                    }
                    if (cnt % 2 == 0) {
                        quotation_flag = false;
                    }
                }
                oformat_json << raw_json[i];
                if (raw_json[i] == ':' && quotation_flag == false) { // : 后加个空格
                    oformat_json << ' ';
                    if (raw_json[i+1] == '"') { // 进入值字符串
                        quotation_flag = true;
                    } 
                }
            }
        }

        return oformat_json.str().c_str();
    } else {
        return this->to_string();
    }
}


}