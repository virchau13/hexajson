#include <string>
#include <vector>
#include <map>

namespace hex {
    /* JSON types as described in the RFC. */
    enum val_type : uint8_t {
        UNDEFINED,
        NUMBER,
        STRING,
        BOOLEAN,
        ARRAY,
        OBJECT,
        INVALID_ITEM
    };
    class json;
    union value {
        double number;
        std::map<std::string, json> *object;
        std::vector<json> *array;
        std::string *str;
        bool boolean;
    };
    class json {
    public:
        value val;
        val_type type = INVALID_ITEM;
        json(const val_type& t = OBJECT){
            type = t;
            if(t == OBJECT) val.object = new std::map<std::string, json>();
            if(t == ARRAY) val.array = new std::vector<json>();
            if(t == STRING) val.str = new std::string();
        }
        json(double rhs){
            operator=(rhs);
        }
        json(const std::string& rhs){
            operator=(rhs);
        }
        json(const char* rhs){
            std::string s(rhs);
            operator=(s);
        }
        json(const std::initializer_list<json>& rhs){
            operator=(rhs);
        }
        void clean_type(){
            if(type == OBJECT) delete val.object;
            if(type == ARRAY) delete val.array;
            if(type == STRING) delete val.str;
            type = INVALID_ITEM;
        }
        const json& operator=(const json& rhs){
            clean_type();
            type = rhs.type;
            val = rhs.val;
            return *this;
        }
        const json& operator=(const std::string& rhs){
            clean_type();
            type = STRING;
            val.str = new std::string(rhs);
            return *this;
        }
        const json& operator=(double rhs){
            clean_type();
            type = NUMBER;
            val.number = rhs;
            return *this;
        }
        const json& operator=(const char* rhs){
            std::string s(rhs);
            operator=(s);
            return *this;
        }
        const json& operator=(const std::initializer_list<json>& rhs){
            clean_type();
            type = ARRAY;
            val.array = new std::vector<json>(rhs);
            return *this;
        }
        bool operator==(const json& rhs){ 
            return type == rhs.type && val.number == rhs.val.number;
        }
        bool operator!=(const json& rhs){
            return !operator==(rhs);
        }
        json& operator[](const std::string& key){
            return val.object->operator[](key);
        }
        json& operator[](size_t idx){
            return val.array->operator[](idx);
        }
        std::string dump(){
            if(type == NUMBER) return std::to_string(val.number);
            if(type == BOOLEAN) return val.boolean ? "true" : "false";
            if(type == UNDEFINED) return "null";
            if(type == STRING) return '"' + (*val.str) + '"';
            if(type == ARRAY){
                std::string ret = "[";
                for(auto& j : *val.array){
                    ret += j.dump();
                    ret += ",";
                }
                ret.pop_back();
                ret += "]";
                return ret;
            }
            if(type == OBJECT){
                std::string ret = "{";
                for(auto& j : *val.object){
                    ret += '"' + j.first + "\":";
                    ret += j.second.dump();
                    ret += ",";
                }
                ret.pop_back();
                ret += "}";
                return ret;
            }
            __builtin_trap();
        }
    };
    json make_obj(const std::initializer_list<std::pair<std::string,json>> rhs){
        json ret;
        for(auto& i : rhs){
            ret[i.first] = i.second;
        }
        return ret;
    }
    /* Time to beat RapidJSON. */
    json parse_json(const std::string& input, int start = 0){
        json ret(INVALID_ITEM);
        for(int pos = start; pos < input.length(); pos++){
        }
    }
};

