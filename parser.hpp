#include <string>
#include <vector>
#include <map>

namespace hex {
    /* JSON types as described in the RFC.
     * Enums would be used, except for the fact that enums are 4 bytes,
     * and we only need 1.
     */
    typedef uint8_t val_type;
    const uint8_t
        UNDEFINED = 0x0,
        NUMBER = 0x1,
        STRING = 0x2,
        BOOLEAN = 0x3,
        ARRAY = 0x4,
        OBJECT = 0x5,
        INVALID_ITEM = 0xFF;
    class json;
    union value {
        int64_t number;
        double decimal;
        std::map<std::string, json> *object;
        std::vector<json> *array;
        std::string *str;
        bool boolean;
    };
    class json {
    public:
        value val;
        val_type type;
        json(): type(INVALID_ITEM) {}
        json(const val_type& t){
            type = t;
            if(t == OBJECT) val.object = new std::map<std::string, json>();
            if(t == ARRAY) val.array = new std::vector<json>();
            if(t == STRING) val.str = new std::string();
        }
        const json& operator=(const json& rhs){
            if(type == OBJECT) delete val.object;
            if(type == ARRAY) delete val.array;
            if(type == STRING) delete val.str;
            type = rhs.type;
            val = rhs.val;
            return *this;
        }
        bool operator==(const json& rhs){ 
            return type == rhs.type && val.number == rhs.val.number;
        }
        bool operator!=(const json& rhs){
            return !operator==(rhs);
        }
        const json& operator[](std::string key){
            return val.object->operator[](key);
        }
        const json& operator[](size_t idx){
            return val.array->operator[](idx);
        }
        const std::string& to_string();
    };
};

