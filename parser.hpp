#include <string>
#include <vector>

namespace hex {
    /* JSON types as described in the RFC.
     * Enums would be used, except for the fact that enums are 4 bytes,
     * and we only need 1.
     */
    namespace {
        typedef uint8_t val_type;
        const uint8_t
            UNDEFINED = 0x0,
            NUMBER = 0x1,
            STRING = 0x2,
            BOOLEAN = 0x3,
            ARRAY = 0x4,
            OBJECT = 0x5,
            INVALID_ITEM = 0xFF;
    };
    class json;
    struct array {
        json *arr;
        size_t len, plen;
        const json& operator[](size_t idx);
        void push_back(const json& obj);
    };
    /* A _value_ is the actual storage of a value. */
    union value {
        int64_t num;
        double flt;
        array* arr;
        json* obj;
    };
    /* A json object represents what something is. */
    class json {
        value val; /* max size 8 */
        val_type type; /* max size 1 */
    public:
        std::string dump_json();
        json(): type(INVALID_ITEM) {}
        json(const val_type& t){
            type = t;
            if(t == ARRAY){
                val.arr = new array();
            } else if(t == OBJECT){
                val.obj = new json();
            }
        }
    };
    json parse_json(const std::string& s, int start_pos = 0){
        /* The RFC says that _any_ JSON type is valid as its own JSON. We'll exploit this. */ 
        json ret;
        int pos = start_pos;
        while(isspace(s[pos])) pos++;
        if(s[pos] == '{'){ // object
            ret = json(
        } else if(s[pos] == '['){ // array
        } else if(s[pos] == '"'){ // string
        };
    } 
};

