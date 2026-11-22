#include "wejson.h"
#include "debug.h"

namespace basic {

// json中重要的分割字符
const std::vector<char> sperate_chars = {' ', '\r', '\n','\t','{', '}','[', ']',',',':','"'};

namespace {
// 将字符串按 JSON 规则转义(引号、反斜杠、控制字符),供序列化输出使用
std::string escape_json_string(const std::string &value)
{
    std::string out;
    for (char c : value) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            default: {
                unsigned char uc = static_cast<unsigned char>(c);
                if (uc < 0x20) {
                    char buf[8] = {0};
                    snprintf(buf, sizeof(buf), "\\u%04x", uc);
                    out += buf;
                } else {
                    out += c;
                }
            } break;
        }
    }
    return out;
}
}

ValueType
JsonType::check_value_type(ByteBuffer::iterator &iter) 
{
    if (isdigit(*iter) || *iter == '-' || *iter == '+') {
        return JSON_NUMBER_TYPE;
    }

    switch (*iter)
    {
        case '{': return JSON_OBJECT_TYPE;
        case '[': return JSON_ARRAY_TYPE;
        case '"': return JSON_STRING_TYPE;
        case 't':
        case 'f': return JSON_BOOL_TYPE;
        case 'n': return JSON_NULL_TYPE;
        default:  break;
    }

    return JSON_UNKNOWN_TYPE;
}

std::string 
JsonType::value_type_to_string(ValueType type)
{
    switch(type)
    {
        case JSON_ARRAY_TYPE:  return "JSON_ARRAY_TYPE";
        case JSON_BOOL_TYPE:   return "JSON_BOOL_TYPE";
        case JSON_NULL_TYPE:   return "JSON_NULL_TYPE";
        case JSON_NUMBER_TYPE: return "JSON_NUMBER_TYPE";
        case JSON_OBJECT_TYPE: return "JSON_OBJECT_TYPE";
        case JSON_STRING_TYPE: return "JSON_STRING_TYPE";
        default:               return "JSON_UNKNOWN_TYPE";
    }
}
///////////////////////////////////////////////////////////

JsonNumber::JsonNumber(void) : value_(0) {}
JsonNumber::JsonNumber(const double &val) : value_(val) {}
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
        ByteBuffer::iterator next_iter = iter;
        ++next_iter;
        if (next_iter != json_end_pos && isdigit(static_cast<unsigned char>(*next_iter))) {
            throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR, "Zero can't be first number of integer!\n%s\n", dump_stack().c_str()));
        }
    }

    bool has_digit = false;
    bool has_dot = false;
    for (; iter != json_end_pos && *iter != ','; ++iter) {
        if (isdigit(static_cast<unsigned char>(*iter))) {
            str += *iter;
            has_digit = true;
        } else if (*iter == '.') {
            if (has_dot) {
                throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR, "More than one '.' in number: %s\n%s\n", str.c_str(), dump_stack().c_str()));
            }
            has_dot = true;
            str += *iter;
        } else {
            break;
        }
    }

    if (!has_digit) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR, "Invalid number: no digit found!\n%s\n", dump_stack().c_str()));
    }

    value_ = stod(str) * (is_negative ? -1 : 1);
    return iter;
}

std::string 
JsonNumber::to_string(void) const
{
    char buf[1024] = {0};
    snprintf(buf, 1024, "%lf", value_);

    int j = 1023;
    for (; j >=0; --j) {
        if (buf[j] == '.') break;
    }

    for (int i = 1023; j >= 0 && i >= j; --i) {
        if (buf[i] <= '9' && buf[i] > '0') break;
        buf[i] = '\0';
    }

    return buf;
}

std::ostream& operator<<(std::ostream &os, JsonNumber &rhs) { os << rhs.to_string(); return os; }

bool JsonNumber::operator==(const JsonNumber& rhs) const { return value_ == rhs.value_; }
bool JsonNumber::operator!=(const JsonNumber& rhs) const { return !(*this == rhs); }
bool JsonNumber::operator==(const double& rhs) const { return value_ == rhs; }
bool JsonNumber::operator!=(const double& rhs) const { return !(*this == rhs); }
JsonNumber& JsonNumber::operator=(JsonNumber rhs) { value_ = rhs.value_; return *this; }

/////////////////////////////////////////////////////////////////

JsonBool::JsonBool(void) {}
JsonBool::JsonBool(bool val) { value_ = val; }
JsonBool::~JsonBool(void) {}

ByteBuffer::iterator 
JsonBool::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    ByteBuffer::iterator iter = value_start_pos;
    int read_size = (*iter == 't') ? 4 : 5;

    std::string str;
    for (int i = 0; iter != json_end_pos && i < read_size; ++iter, ++i) {
        str += *iter;
    }
    
    if (str == "true")       value_ = true;
    else if (str == "false") value_ = false;
    else throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Expected true or false, but result is %s\n%s\n", str.c_str(), dump_stack().c_str()));

    return iter;
}

std::string JsonBool::to_string(void) const { return value_ ? "true" : "false"; }
std::ostream& operator<<(std::ostream &os, JsonBool &rhs) { os << rhs.to_string(); return os; }

bool JsonBool::operator==(const JsonBool& rhs) const { return value_ == rhs.value_; }
bool JsonBool::operator!=(const JsonBool& rhs) const { return !(*this == rhs); }
bool JsonBool::operator==(const bool& rhs) const { return value_ == rhs; }
bool JsonBool::operator!=(const bool& rhs) const { return !(*this == rhs); }
JsonBool& JsonBool::operator=(JsonBool rhs) { value_ = rhs.value_; return *this; }

//////////////////////////////////////////////////////////////
JsonNull::JsonNull(void) : value_("null") {}
JsonNull::~JsonNull(void) {}

ByteBuffer::iterator 
JsonNull::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    ByteBuffer::iterator iter = value_start_pos;
    int read_size = 4;

    std::string str;
    for (int i = 0; iter != json_end_pos && i < read_size; ++iter, ++i) {
        str += *iter;
    }

    if (str == "null") value_ = str;
    else throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Expected null, but result is %s\n%s\n", str.c_str(), dump_stack().c_str()));

    return iter;
}

std::string JsonNull::to_string(void) const { return "null"; }
std::ostream& operator<<(std::ostream &os, JsonNull &rhs) { os << rhs.to_string(); return os; }

bool JsonNull::operator==(const JsonNull& rhs) const { return value_ == rhs.value_; }
bool JsonNull::operator!=(const JsonNull& rhs) const { return !(*this == rhs); }
JsonNull& JsonNull::operator=(JsonNull rhs) { value_ = rhs.value_; return *this; }

///////////////////////////////////////////////////////////

JsonString::JsonString(void) {}
JsonString::JsonString(const std::string &val) : value_(val) {}
JsonString::JsonString(const char *val) : value_(val != nullptr ? val : "") {}
JsonString::~JsonString(void) {}

ByteBuffer::iterator 
JsonString::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    ByteBuffer::iterator iter = value_start_pos;
    ++iter;
    std::string str;
    for (; iter != json_end_pos; ++iter) {
        if (*iter == '\\') { 
            ++iter;
            if (iter == json_end_pos) {
                throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Invalid escape sequence at end of string!\n%s\n", dump_stack().c_str()));
            }

            switch (*iter) {
                case '"':  str += '"';  break;
                case '\\': str += '\\'; break;
                case '/':  str += '/';  break;
                case 'b':  str += '\b'; break;
                case 'f':  str += '\f'; break;
                case 'n':  str += '\n'; break;
                case 'r':  str += '\r'; break;
                case 't':  str += '\t'; break;
                case 'u': {
                    std::string hex;
                    bool valid = true;
                    for (int k = 0; k < 4; ++k) {
                        ++iter;
                        if (iter == json_end_pos || !isxdigit(static_cast<unsigned char>(*iter))) {
                            valid = false;
                            break;
                        }
                        hex += *iter;
                    }
                    if (!valid || hex.size() != 4) {
                        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Invalid \\u escape sequence in string!\n%s\n", dump_stack().c_str()));
                    }
                    str += static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
                } break;
                default:
                    str += '\\'; // 未知转义保留原文,保证往返一致
                    str += *iter;
                    break;
            }
            continue;
        } else if (*iter == '"') {
            break;
        }
        str += *iter;
    }
    
    if (iter == json_end_pos) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"String need to surround by \"\"\n%s\n", dump_stack().c_str()));
    }
    value_ = str;
    ++iter;
    return iter;
}

std::string JsonString::to_string(void) const { return escape_json_string(value_); }
std::ostream& operator<<(std::ostream &os, JsonString &rhs) { os << rhs.to_string(); return os; }
bool JsonString::operator==(const JsonString& rhs) const { return value_ == rhs.value_; }
bool JsonString::operator!=(const JsonString& rhs) const { return !(*this == rhs); }
bool JsonString::operator==(const std::string& rhs) const { return value_ == rhs; }
bool JsonString::operator!=(const std::string& rhs) const { return !(*this == rhs); }
JsonString& JsonString::operator=(JsonString rhs) { value_ = rhs.value_; return *this; }

///////////////////////////////////////////////////////////

JsonObject::JsonObject(void) {}
JsonObject::JsonObject(const JsonObject &jobj) { *this = jobj; }
JsonObject::~JsonObject(void) {}

ByteBuffer::iterator 
JsonObject::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    value_.clear();
    ByteBuffer::iterator iter = value_start_pos;
    ++iter;

    // 状态机: 0 = 期望键或 '}', 1 = 已读键期望 ':', 2 = 期望值
    int state = 0;
    std::string value_name;
    for (; iter != json_end_pos && *iter != '}'; ++iter) {
        ValueType ret_value_type = this->check_value_type(iter);
        if (ret_value_type == JSON_UNKNOWN_TYPE) {
            if (*iter == ',') {
                if (state != 2) {
                    throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Unexpected ',' in object!\n%s\n", dump_stack().c_str()));
                }
                state = 0;
                continue;
            } else if (*iter == ':') {
                if (state != 1) {
                    throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Unexpected ':' in object!\n%s\n", dump_stack().c_str()));
                }
                state = 2;
                continue;
            } else if (*iter == ']') {
                continue;
            }
            throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR, "Unknown character in object: %c\n%s\n", *iter, dump_stack().c_str()));
        }

        if (state == 0) {
            if (ret_value_type != JSON_STRING_TYPE) {
                throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Expect a string key in object, but get other type!\n%s\n", dump_stack().c_str()));
            }
            JsonString json_str;
            iter = json_str.parse(iter, json_end_pos);
            value_name = json_str.raw_value();
            // 键后应紧跟 ':'(for 循环的 ++iter 会跳过 ':')
            if (iter == json_end_pos || *iter != ':') {
                throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Expect ':' after key \"%s\" in object!\n%s\n", value_name.c_str(), dump_stack().c_str()));
            }
            state = 2;
            continue;
        } else if (state == 1) {
            throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Expect ':' after key \"%s\" in object!\n%s\n", value_name.c_str(), dump_stack().c_str()));
        }

        // state == 2,解析值
        JsonValue vtc;
        switch (ret_value_type)
        {
            case JSON_OBJECT_TYPE: { JsonObject val; iter = val.parse(iter, json_end_pos); vtc = val; } break;
            case JSON_ARRAY_TYPE:  { JsonArray val;  iter = val.parse(iter, json_end_pos); vtc = val; } break;
            case JSON_STRING_TYPE: { JsonString val; iter = val.parse(iter, json_end_pos); vtc = val; } break;
            case JSON_BOOL_TYPE:   { JsonBool val;   iter = val.parse(iter, json_end_pos); vtc = val; } break;
            case JSON_NULL_TYPE:   { JsonNull val;   iter = val.parse(iter, json_end_pos); vtc = val; } break;
            case JSON_NUMBER_TYPE: { JsonNumber val; iter = val.parse(iter, json_end_pos); vtc = val; } break;
            default: break;
        }

        if (value_.find(value_name) == value_.end()) {
            value_[value_name] = vtc;
        } else {
            throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"The \"%s\" is already exists.\n%s\n", value_name.c_str(), dump_stack().c_str()));
        }
        state = 0;
        if (iter != json_end_pos && *iter == '}') break;
    }

    if (state == 1) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Expect ':' after key \"%s\" in object!\n%s\n", value_name.c_str(), dump_stack().c_str()));
    }
    if (state == 2) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Expect value after ':' in object!\n%s\n", dump_stack().c_str()));
    }
    if (iter == json_end_pos) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Object need to surround by {}\n%s\n", dump_stack().c_str()));
    }

    if (*iter == '}') ++iter;
    return iter;
}

std::string 
JsonObject::to_string(void) const
{
    std::ostringstream output_obj;
    output_obj << "{";
    for (auto iter = value_.begin(); iter != value_.end(); ++iter) {
        if (iter != value_.begin()) output_obj << ",";

        output_obj << "\"" << escape_json_string(iter->first) << "\":";
        switch (iter->second.internal_type())
        {
            case JSON_ARRAY_TYPE:  output_obj << dynamic_cast<JsonArray*>(iter->second.internal_ptr())->to_string(); break;
            case JSON_OBJECT_TYPE: output_obj << dynamic_cast<JsonObject*>(iter->second.internal_ptr())->to_string(); break;
            case JSON_STRING_TYPE: output_obj << "\"" << dynamic_cast<JsonString*>(iter->second.internal_ptr())->to_string() << "\""; break;
            case JSON_NUMBER_TYPE: output_obj << dynamic_cast<JsonNumber*>(iter->second.internal_ptr())->to_string(); break;
            case JSON_BOOL_TYPE:   output_obj << dynamic_cast<JsonBool*>(iter->second.internal_ptr())->to_string(); break;
            default:               output_obj << JsonNull().to_string();
        }
    }
    output_obj << "}";
    return output_obj.str();
}

JsonObject::iterator JsonObject::find(const std::string &key) { return value_.find(key); }
JsonObject::const_iterator JsonObject::find(const std::string &key) const { return value_.find(key); }
bool JsonObject::contains(const std::string &key) const { return value_.find(key) != value_.end(); }
int JsonObject::erase(const std::string &key) { return static_cast<int>(value_.erase(key)); }
JsonObject::iterator JsonObject::erase(JsonObject::iterator &remove_iter) { return value_.erase(remove_iter); }
int JsonObject::add(const std::string &key, const JsonValue &value) { value_[key] = value; return 0; }

std::ostream& operator<<(std::ostream &os, JsonObject &rhs) { os << rhs.to_string(); return os; }

bool JsonObject::operator==(const JsonObject& rhs) const
{
    if (value_.size() != rhs.value_.size()) return false;

    auto iter_this = value_.begin();
    auto iter_rhs = rhs.value_.begin();
    while (iter_rhs != rhs.value_.end() && iter_this != value_.end()) {
        if (iter_this->first != iter_rhs->first) return false;
        if (iter_this->second != iter_rhs->second) return false;
        ++iter_this; ++iter_rhs;
    }

    return (iter_rhs == rhs.value_.end() && iter_this == value_.end());
}
bool JsonObject::operator!=(const JsonObject& rhs) const { return !(*this == rhs); }

JsonValue& JsonObject::operator[](const std::string &key)
{
    if (!contains(key)) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"JsonObject: out of range.[key: %s]\n%s\n", key.c_str(), dump_stack().c_str()));
    }
    return value_[key];
}
const JsonValue& JsonObject::operator[](const std::string &key) const
{
    auto it = value_.find(key);
    if (it == value_.end()) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"JsonObject: out of range.[key: %s]\n%s\n", key.c_str(), dump_stack().c_str()));
    }
    return it->second;
}

JsonObject& JsonObject::operator=(const JsonObject &rhs) { value_ = rhs.value_; return *this; }

JsonObject::iterator       JsonObject::begin()       { return value_.begin(); }
JsonObject::const_iterator JsonObject::begin() const { return value_.begin(); }
JsonObject::iterator       JsonObject::end()         { return value_.end(); }
JsonObject::const_iterator JsonObject::end()   const { return value_.end(); }

////////////////////////////////////////////////////////////

JsonArray::JsonArray(void) {}
JsonArray::JsonArray(const JsonArray &jarr) { *this = jarr; }
JsonArray::~JsonArray(void) {}

ByteBuffer::iterator 
JsonArray::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    value_.clear();
    ByteBuffer::iterator iter = value_start_pos;
    ++iter;

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
            case JSON_OBJECT_TYPE: { JsonObject val; iter = val.parse(iter, json_end_pos); vtc = val; } break;
            case JSON_ARRAY_TYPE:  { JsonArray val;  iter = val.parse(iter, json_end_pos); vtc = val; } break;
            case JSON_STRING_TYPE: { JsonString val; iter = val.parse(iter, json_end_pos); vtc = val; } break;
            case JSON_BOOL_TYPE:   { JsonBool val;   iter = val.parse(iter, json_end_pos); vtc = val; } break;
            case JSON_NULL_TYPE:   { JsonNull val;   iter = val.parse(iter, json_end_pos); vtc = val; } break;
            case JSON_NUMBER_TYPE: { JsonNumber val; iter = val.parse(iter, json_end_pos); vtc = val; } break;
            default: break;
        }
        
        value_.push_back(vtc);
        if (iter != json_end_pos && *iter == ']') break;
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
        if (iter != value_.begin()) output_arr << ",";
        switch (iter->internal_type())
        {
            case JSON_ARRAY_TYPE:  output_arr << dynamic_cast<JsonArray*>(iter->internal_ptr())->to_string(); break;
            case JSON_OBJECT_TYPE: output_arr << dynamic_cast<JsonObject*>(iter->internal_ptr())->to_string(); break;
            case JSON_STRING_TYPE: output_arr << "\"" << dynamic_cast<JsonString*>(iter->internal_ptr())->to_string() << "\""; break;
            case JSON_NUMBER_TYPE: output_arr << dynamic_cast<JsonNumber*>(iter->internal_ptr())->to_string(); break;
            case JSON_BOOL_TYPE:   output_arr << dynamic_cast<JsonBool*>(iter->internal_ptr())->to_string(); break;
            default:               output_arr << JsonNull().to_string();
        }
    }

    output_arr << "]";
    return output_arr.str();
}

int JsonArray::add(const JsonValue &value) { value_.push_back(value); return 0; }

JsonArray::iterator JsonArray::erase(const int &index)
{
    if (index < 0 || static_cast<std::size_t>(index) >= value_.size()) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"JsonArray::erase: index out of range [%d]\n%s\n", index, dump_stack().c_str()));
    }
    auto remove_iter = value_.begin();
    std::advance(remove_iter, index);
    return value_.erase(remove_iter);
}

JsonArray::iterator JsonArray::erase(const JsonArray::iterator &remove_iter) { return value_.erase(remove_iter); }

std::ostream& operator<<(std::ostream &os, JsonArray &rhs) { os << rhs.to_string(); return os; }

JsonValue& JsonArray::operator[](size_t key)
{
    if (key >= value_.size()) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"JsonArray: out of range[index: %zu]\n%s", key, dump_stack().c_str()));
    }
    return value_[key];
}
const JsonValue& JsonArray::operator[](size_t key) const
{
    if (key >= value_.size()) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"JsonArray: out of range[index: %zu]\n%s", key, dump_stack().c_str()));
    }
    return value_[key];
}

bool JsonArray::operator==(const JsonArray& rhs) const
{
    if (value_.size() != rhs.value_.size()) return false;
    for (std::size_t i = 0; i < value_.size(); ++i) {
        if (value_[i].internal_type() != rhs.value_[i].internal_type()) return false;
        if (value_[i] != rhs.value_[i]) return false;
    }
    return true;
}
bool JsonArray::operator!=(const JsonArray& rhs) const { return !(*this == rhs); }
JsonArray& JsonArray::operator=(const JsonArray &rhs) { value_ = rhs.value_; return *this; }

JsonArray::iterator       JsonArray::begin()       { return value_.begin(); }
JsonArray::const_iterator JsonArray::begin() const { return value_.begin(); }
JsonArray::iterator       JsonArray::end()         { return value_.end(); }
JsonArray::const_iterator JsonArray::end()   const { return value_.end(); }

/////////////////////////////////////////////////////////////////

// JsonValue constructors
JsonValue::JsonValue(void) : type_(JSON_NULL_TYPE) { value_ = new JsonNull(); }
JsonValue::JsonValue(const JsonBool &value)   : type_(JSON_BOOL_TYPE)   { value_ = new JsonBool(value.to_bool()); }
JsonValue::JsonValue(const JsonNumber &value) : type_(JSON_NUMBER_TYPE) { value_ = new JsonNumber(value.to_double()); }
JsonValue::JsonValue(const JsonString &value) : type_(JSON_STRING_TYPE) { value_ = new JsonString(value.raw_value()); }
JsonValue::JsonValue(const JsonObject &value) : type_(JSON_OBJECT_TYPE) { JsonObject *o = new JsonObject(); *o = value; value_ = o; }
JsonValue::JsonValue(const JsonArray &value)  : type_(JSON_ARRAY_TYPE)  { JsonArray *a = new JsonArray(); *a = value; value_ = a; }
JsonValue::JsonValue(const JsonNull &value)   : type_(JSON_NULL_TYPE)   { value_ = new JsonNull(); }
JsonValue::JsonValue(const JsonValue &value)  : type_(value.type_)      { this->copy(value, false); }
JsonValue::JsonValue(const bool &value)        : type_(JSON_BOOL_TYPE)   { value_ = new JsonBool(value); }
JsonValue::JsonValue(const int &value)         : type_(JSON_NUMBER_TYPE) { value_ = new JsonNumber(value); }
JsonValue::JsonValue(const double &value)      : type_(JSON_NUMBER_TYPE) { value_ = new JsonNumber(value); }
JsonValue::JsonValue(const std::string &value) : type_(JSON_STRING_TYPE) { value_ = new JsonString(value); }
JsonValue::JsonValue(const char *value)        : type_(JSON_STRING_TYPE) { value_ = new JsonString(value != nullptr ? value : ""); }

JsonValue::~JsonValue(void) { if (value_ != nullptr) { delete value_; value_ = nullptr; } }

// Type conversion operators
JsonValue::operator JsonBool() {
    if (type_ != JSON_BOOL_TYPE) throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"cast failed: not a bool. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    return JsonBool(dynamic_cast<JsonBool*>(value_)->to_bool());
}
JsonValue::operator JsonNumber() {
    if (type_ != JSON_NUMBER_TYPE) throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"cast failed: not a number. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    return JsonNumber(dynamic_cast<JsonNumber*>(value_)->to_double());
}
JsonValue::operator JsonString() {
    if (type_ != JSON_STRING_TYPE) throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"cast failed: not a string. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    return JsonString(dynamic_cast<JsonString*>(value_)->raw_value());
}
JsonValue::operator JsonObject() {
    if (type_ != JSON_OBJECT_TYPE) throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"cast failed: not an object. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    return JsonObject(*dynamic_cast<JsonObject*>(value_));
}
JsonValue::operator JsonArray() {
    if (type_ != JSON_ARRAY_TYPE) throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"cast failed: not an array. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    return *dynamic_cast<JsonArray*>(value_);
}
JsonValue::operator JsonNull() {
    if (type_ != JSON_NULL_TYPE) throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"cast failed: not null. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    return JsonNull();
}

// Assignment operators
JsonValue& JsonValue::operator=(const JsonBool &val)   { type_ = JSON_BOOL_TYPE;   delete value_; value_ = new JsonBool(val.to_bool());     return *this; }
JsonValue& JsonValue::operator=(const JsonNumber &val) { type_ = JSON_NUMBER_TYPE; delete value_; value_ = new JsonNumber(val.to_double());   return *this; }
JsonValue& JsonValue::operator=(const JsonString &val) { type_ = JSON_STRING_TYPE; delete value_; value_ = new JsonString(val.raw_value());   return *this; }
JsonValue& JsonValue::operator=(const JsonObject &val) { type_ = JSON_OBJECT_TYPE; delete value_; value_ = new JsonObject(val); return *this; }
JsonValue& JsonValue::operator=(const JsonArray &val)  { type_ = JSON_ARRAY_TYPE;  delete value_; value_ = new JsonArray(val); return *this; }
JsonValue& JsonValue::operator=(const JsonNull &val)   { type_ = JSON_NULL_TYPE;   delete value_; value_ = new JsonNull();               return *this; }
JsonValue& JsonValue::operator=(const JsonValue &val)  { if (this == &val) { return *this; } this->copy(val); return *this; }

bool JsonValue::operator==(const JsonValue& rhs) const
{
    if (type_ != rhs.type_) return false;
    switch (type_)
    {
    case JSON_NULL_TYPE:   return *dynamic_cast<JsonNull*>(value_)   == *dynamic_cast<JsonNull*>(rhs.value_);
    case JSON_NUMBER_TYPE: return *dynamic_cast<JsonNumber*>(value_) == *dynamic_cast<JsonNumber*>(rhs.value_);
    case JSON_STRING_TYPE: return *dynamic_cast<JsonString*>(value_) == *dynamic_cast<JsonString*>(rhs.value_);
    case JSON_BOOL_TYPE:   return *dynamic_cast<JsonBool*>(value_)   == *dynamic_cast<JsonBool*>(rhs.value_);
    case JSON_ARRAY_TYPE:  return *dynamic_cast<JsonArray*>(value_)  == *dynamic_cast<JsonArray*>(rhs.value_);
    case JSON_OBJECT_TYPE: return *dynamic_cast<JsonObject*>(value_) == *dynamic_cast<JsonObject*>(rhs.value_);
    default:               return true;
    }
}
bool JsonValue::operator!=(const JsonValue& rhs) const { return !(*this == rhs); }

bool JsonValue::operator==(const char *rhs) const
{
    if (type_ != JSON_STRING_TYPE) return false;
    return dynamic_cast<JsonString*>(value_)->to_string() == escape_json_string(rhs != nullptr ? rhs : "");
}
bool JsonValue::operator!=(const char *rhs) const { return !(*this == rhs); }
bool JsonValue::operator==(const std::string &rhs) const
{
    if (type_ != JSON_STRING_TYPE) return false;
    return dynamic_cast<JsonString*>(value_)->to_string() == escape_json_string(rhs);
}
bool JsonValue::operator!=(const std::string &rhs) const { return !(*this == rhs); }

JsonValue& JsonValue::operator[](const std::string &key)
{
    if (type_ != JSON_OBJECT_TYPE) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"operator[]: not an object. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
    return (*dynamic_cast<JsonObject*>(value_))[key];
}
const JsonValue& JsonValue::operator[](const std::string &key) const
{
    if (type_ != JSON_OBJECT_TYPE) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"operator[]: not an object. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
    return (*dynamic_cast<const JsonObject*>(value_))[key];
}
JsonValue& JsonValue::operator[](const int &key)
{
    if (type_ != JSON_ARRAY_TYPE) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"operator[]: not an array. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
    return (*dynamic_cast<JsonArray*>(value_))[key];
}
const JsonValue& JsonValue::operator[](const int &key) const
{
    if (type_ != JSON_ARRAY_TYPE) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"operator[]: not an array. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
    return (*dynamic_cast<const JsonArray*>(value_))[key];
}

void JsonValue::copy(const JsonValue &val, bool is_release)
{
    if (is_release) { delete value_; value_ = nullptr; }
    type_ = val.type_;
    switch (type_)
    {
    case JSON_NULL_TYPE:   value_ = new JsonNull(); break;
    case JSON_NUMBER_TYPE: value_ = new JsonNumber(dynamic_cast<JsonNumber*>(val.value_)->to_double()); break;
    case JSON_STRING_TYPE: value_ = new JsonString(dynamic_cast<JsonString*>(val.value_)->raw_value()); break;
    case JSON_BOOL_TYPE:   value_ = new JsonBool(dynamic_cast<JsonBool*>(val.value_)->to_bool()); break;
    case JSON_ARRAY_TYPE:  { JsonArray *a = new JsonArray(); *a = *dynamic_cast<JsonArray*>(val.value_); value_ = a; } break;
    case JSON_OBJECT_TYPE: { JsonObject *o = new JsonObject(); *o = *dynamic_cast<JsonObject*>(val.value_); value_ = o; } break;
    default: value_ = nullptr; break;
    }
}

std::string JsonValue::to_string(void) const
{
    switch (type_)
    {
        case JSON_ARRAY_TYPE:  return dynamic_cast<JsonArray*>(value_)->to_string();
        case JSON_OBJECT_TYPE: return dynamic_cast<JsonObject*>(value_)->to_string();
        case JSON_STRING_TYPE: return dynamic_cast<JsonString*>(value_)->to_string();
        case JSON_NUMBER_TYPE: return dynamic_cast<JsonNumber*>(value_)->to_string();
        case JSON_BOOL_TYPE:   return dynamic_cast<JsonBool*>(value_)->to_string();
        default:               return JsonNull().to_string();
    }
}

int64_t JsonValue::to_int(void) const
{
    if (type_ != JSON_NUMBER_TYPE) throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"to_int: not a number. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    return dynamic_cast<JsonNumber*>(value_)->to_int();
}
double JsonValue::to_double(void) const
{
    if (type_ != JSON_NUMBER_TYPE) throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"to_double: not a number. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    return dynamic_cast<JsonNumber*>(value_)->to_double();
}

JsonArray& JsonValue::to_array(void)
{
    if (type_ != JSON_ARRAY_TYPE) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"to_array: not an array. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
    return *dynamic_cast<JsonArray*>(value_);
}
const JsonArray& JsonValue::to_array(void) const
{
    if (type_ != JSON_ARRAY_TYPE) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"to_array: not an array. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
    return *dynamic_cast<const JsonArray*>(value_);
}
JsonObject& JsonValue::to_object(void)
{
    if (type_ != JSON_OBJECT_TYPE) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"to_object: not an object. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
    return *dynamic_cast<JsonObject*>(value_);
}
const JsonObject& JsonValue::to_object(void) const
{
    if (type_ != JSON_OBJECT_TYPE) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"to_object: not an object. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    }
    return *dynamic_cast<const JsonObject*>(value_);
}

int JsonValue::size(void) const
{
    switch (type_) {
        case JSON_ARRAY_TYPE:  return dynamic_cast<JsonArray*>(value_)->size();
        case JSON_OBJECT_TYPE: return dynamic_cast<JsonObject*>(value_)->size();
        default: return 0;
    }
}
bool JsonValue::empty(void) const
{
    switch (type_) {
        case JSON_ARRAY_TYPE:  return dynamic_cast<JsonArray*>(value_)->empty();
        case JSON_OBJECT_TYPE: return dynamic_cast<JsonObject*>(value_)->empty();
        default: return true;
    }
}

///////////////////////////////// WeJson //////////////////////////////////////////////////
WeJson::WeJson(void) {}
WeJson::WeJson(const std::string &json) { this->parse(json); }
WeJson::WeJson(const ByteBuffer &data)  { this->parse(data); }
WeJson::~WeJson(void) {}

void WeJson::create_object(void) { if (value_ != nullptr) delete value_; type_ = JSON_OBJECT_TYPE; value_ = new JsonObject(); }
void WeJson::create_array(void)  { if (value_ != nullptr) delete value_; type_ = JSON_ARRAY_TYPE;  value_ = new JsonArray(); }

JsonObject& WeJson::get_object(void)
{
    if (type_ != JSON_OBJECT_TYPE) throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"get_object: not an object. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    return *dynamic_cast<JsonObject*>(value_);
}
JsonArray& WeJson::get_array(void)
{
    if (type_ != JSON_ARRAY_TYPE) throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"get_array: not an array. [type: %s]\n%s\n", JsonType::value_type_to_string(type_), dump_stack().c_str()));
    return *dynamic_cast<JsonArray*>(value_);
}

int WeJson::parse(const std::string &data) { return this->parse(ByteBuffer(data)); }

int WeJson::parse(const ByteBuffer &buff)
{
    ByteBuffer simple_json_text;
    ByteBuffer::iterator start_pos;
    
    for (start_pos = buff.begin(); start_pos != buff.end(); ++start_pos) {
        ValueType ret = JsonType::check_value_type(start_pos);
        if (ret == JSON_OBJECT_TYPE || ret == JSON_ARRAY_TYPE) break;
    }

    if (start_pos == buff.end()) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR,"Can't find any json text!\n%s\n", dump_stack().c_str()));
    }

    bool quotation_marks = false;
    for (auto iter = start_pos; iter != buff.end(); ++iter) {
        if (quotation_marks == false && *iter == '"') { quotation_marks = true; simple_json_text.write_int8(*iter); continue; }
        if (quotation_marks == true) {
            if (*iter == '\\') {
                simple_json_text.write_int8(*iter);
                ++iter;
                if (iter != buff.end()) {
                    simple_json_text.write_int8(*iter);
                }
                continue; // 转义序列已整体处理,跳过底部写入
            } else if (*iter == '"') {
                quotation_marks = false;
            }
        }

        int i = 0;
        for (i = 0; i < 4; ++i) { if (sperate_chars[i] == *iter) break; }
        if (i < 4 && quotation_marks == false) continue;

        if (quotation_marks == false && *iter == '/') {
            ByteBuffer::iterator next_iter = iter;
            ++next_iter;
            if (next_iter != buff.end() && *next_iter == '/') {
                for (++iter; iter != buff.end(); ++iter) { if (*iter == '\n') break; }
                continue; // 行注释整体跳过(含 EOF 无换行的情况)
            }
        }

        simple_json_text.write_int8(*iter);
    }
    auto begin_json = simple_json_text.begin();
    auto end_json = simple_json_text.end();
    ByteBuffer::iterator parse_end = parse(begin_json, end_json);
    if (parse_end != end_json) {
        throw std::runtime_error(GLOBAL_GET_MSG(LOG_LEVEL_ERROR, "Unexpected content after json text!\n%s\n", dump_stack().c_str()));
    }
    return 0;
}


ByteBuffer::iterator 
WeJson::parse(ByteBuffer::iterator &value_start_pos, ByteBuffer::iterator &json_end_pos)
{
    ValueType ret_type = JsonType::check_value_type(value_start_pos);
    type_ = ret_type;
    switch (ret_type)
    {
    case JSON_ARRAY_TYPE:  { this->create_array();  return dynamic_cast<JsonArray*>(value_)->parse(value_start_pos, json_end_pos); }
    case JSON_OBJECT_TYPE: { this->create_object(); return dynamic_cast<JsonObject*>(value_)->parse(value_start_pos, json_end_pos); }
    default: break;
    }
    return json_end_pos;
}


std::string WeJson::to_string(void)
{
    switch (type_)
    {
    case JSON_ARRAY_TYPE:  return dynamic_cast<JsonArray*>(value_)->to_string();
    case JSON_OBJECT_TYPE: return dynamic_cast<JsonObject*>(value_)->to_string();
    default:               break;
    }
    return "";
}

std::string WeJson::format_json(void)
{
    if (type_ == JSON_OBJECT_TYPE || type_ == JSON_ARRAY_TYPE) {
        std::string raw_json = this->to_string();

        bool quotation_flag = false;
        int tab = 0;
        std::vector<char> brackets; // 栈式管理括号嵌套
        std::ostringstream oformat_json;
        for (std::size_t i = 0; i < raw_json.size(); ++i) {
            if (raw_json[i] == '{' || raw_json[i] == '[') {
                ++tab; oformat_json << raw_json[i] << '\n';
                for (int j = 0; j < tab; ++j) oformat_json << '\t';
                if (raw_json[i] == '[' || raw_json[i] == '{') brackets.push_back(raw_json[i]);
            } else if (raw_json[i] == '}' || raw_json[i] == ']') {
                oformat_json << '\n'; --tab;
                for (int j = 0; j < tab; ++j) oformat_json << '\t';
                oformat_json << raw_json[i];
                if (!brackets.empty()) brackets.pop_back();
            } else if (quotation_flag == false && raw_json[i] == ',' && (raw_json[i+1] == '"' || (!brackets.empty() && brackets.back() == '['))) {
                if (raw_json[i+1] == '"') quotation_flag = true;
                oformat_json << raw_json[i] << '\n';
                for (int j = 0; j < tab; ++j) oformat_json << '\t';
            } else {
                if (raw_json[i] == '"' && (raw_json[i+1] == ',' || raw_json[i+1] == ':' || raw_json[i+1] == '}' || raw_json[i+1] == ']') && quotation_flag == true) {
                    int pos = static_cast<int>(i); int cnt = 0;
                    for ( ; pos >= 0; --pos) { if (raw_json[pos] == '\\') cnt++; else break; }
                    if (cnt % 2 == 0) quotation_flag = false;
                }
                oformat_json << raw_json[i];
                if (raw_json[i] == ':' && quotation_flag == false) { oformat_json << ' '; if (raw_json[i+1] == '"') quotation_flag = true; }
            }
        }
        return oformat_json.str().c_str();
    } else {
        return this->to_string();
    }
}

}