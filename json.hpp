#ifndef HEX_JSON_HPP
#define HEX_JSON_HPP

#include <iostream>
#include <vector>
#include <map>
#include <charconv>
#include <sstream>

#ifdef DEBUG
#define dbg std::cerr
#else
#define dbg if(0) std::cerr
#endif

namespace hex {
    /* JSON types as described in the RFC. */
    enum val_type : uint8_t {
        UNDEFINED,
        DECIMAL,
        INTEGER,
        STRING,
        BOOLEAN,
        ARRAY,
        OBJECT,
        INVALID_ITEM
    };
    class json;
    typedef std::map<std::string, json> table;
    union value {
        double decimal;
        int64_t integer;
        table *object;
        std::vector<json> *array;
        std::string *str;
        bool boolean;
        const char *invalid_end;
    };
    class json {
        public:
        value val;
        val_type type = INVALID_ITEM;
        // constructors and destructors
        // {{{
        json(const json& rhs){
            type = rhs.type;
            if(type == OBJECT) val.object = new table(*rhs.val.object);
            else if(type == ARRAY) val.array = new std::vector<json>(*rhs.val.array);
            else if(type == STRING) val.str = new std::string(*rhs.val.str);
            else val = rhs.val;
        }
        json(json&& rhs){
            type = rhs.type;
            /* Move constructor, more like pilfer constructor. */
            val = rhs.val;
            rhs.type = INVALID_ITEM;
        }
        json(const val_type& t = OBJECT){
            type = t;
            if(t == OBJECT) val.object = new table();
            if(t == ARRAY) val.array = new std::vector<json>();
            if(t == STRING) val.str = new std::string();
        }
        json(const std::string& rhs){
            operator=(rhs);
        }
        json(const char* rhs){
            std::string s(rhs);
            operator=(s);
        }
        json(int rhs){
            operator=(rhs);
        }
        json(double rhs){
            operator=(rhs);
        }
        void clean_type(){
            if(type == OBJECT) delete val.object;
            if(type == ARRAY) delete val.array;
            if(type == STRING) delete val.str;
            type = INVALID_ITEM;
        }
        ~json(){
            clean_type();
        }
        // }}}
        // operators
        // {{{
        const json& operator=(const json rhs){
            clean_type();
            type = rhs.type;
            if(type == OBJECT) val.object = new table(*rhs.val.object);
            else if(type == ARRAY) val.array = new std::vector<json>(*rhs.val.array);
            else if(type == STRING) val.str = new std::string(*rhs.val.str);
            else val = rhs.val;
            return *this;
        }
        const json& operator=(const val_type& t){
            clean_type();
            type = t;
            if(t == OBJECT) val.object = new table();
            if(t == ARRAY) val.array = new std::vector<json>();
            if(t == STRING) val.str = new std::string();
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
            type = DECIMAL;
            val.decimal = rhs;
            return *this;
        }
        const json& operator=(const char* rhs){
            std::string s(rhs);
            operator=(s);
            return *this;
        }
        const json& operator=(const std::vector<json>& rhs){
            clean_type();
            type = ARRAY;
            val.array = new std::vector<json>(rhs);
            return *this;
        }
        bool operator==(const json& rhs) const noexcept { 
            return type == rhs.type &&
                (type == OBJECT ? *val.object == *rhs.val.object :
                 type == ARRAY ? *val.array == *rhs.val.array :
                 type == STRING ? *val.str == *rhs.val.str :
                 type == BOOLEAN ? val.boolean == rhs.val.boolean :
                 type == UNDEFINED ? true :
                 /* all other types are 64 bit */ val.integer == rhs.val.integer);
        }
        inline bool operator!=(const json& rhs) const noexcept {
            return !operator==(rhs);
        }
        json& operator[](const std::string& key){
            std::string k(key.begin(), key.end());
            return val.object->operator[](k);
        }
        json& operator[](size_t idx){
            return val.array->operator[](idx);
        }
        // }}}
        // convenience functions
        // {{{
        inline bool& as_bool() noexcept {
            return val.boolean;
        }
        inline int64_t& as_int() noexcept {
            return val.integer;
        }
        inline std::string& as_str(){
            return *val.str;
        }
        inline double& as_double(){
            return val.decimal;
        }
        void push_back(const json& j){
            val.array->push_back(j);
        }
        inline bool invalid(){
            return type == INVALID_ITEM;
        }
        // }}}
        std::string dump(){
            if(type == INTEGER){
                std::ostringstream oss;
                oss.imbue(std::locale::classic());
                oss << val.decimal;
                return oss.str();
            }
            if(type == DECIMAL){
                std::ostringstream oss;
                oss.imbue(std::locale::classic());
                oss << val.decimal;
                return oss.str();
            }
            if(type == BOOLEAN) return val.boolean ? "true" : "false";
            if(type == UNDEFINED) return "null";
            if(type == STRING) return '"' + (*val.str) + '"';
            if(type == ARRAY){
                std::string ret = "[";
                for(int i = 0; i < val.array->size(); i++){
                    ret += val.array->operator[](i).dump();
                    ret += ",";
                }
                ret.pop_back();
                ret += "]";
                return ret;
            }
            if(type == OBJECT){
                std::string ret = "{";
                for(auto& j : *val.object){
                    ret += '"';
                    for(char i : j.first){
                        if(i == '"'){
                            ret += '\\';
                            ret += i;
                        } else if(i <= 0x1f){ // From RFC (7. Strings)
                            // get top part
                            int first = i & (0b11110000);
                            // get bottom part
                            int second = i & (0b00001111);
                            char f = (first < 10 ? '0' + first : 'a' + (first - 10));
                            char s = (second < 10 ? '0' + second : 'a' + (second - 10));
                            ret += "\\u00";
                            ret += f;
                            ret += s;
                        } else {
                            ret += i;
                        }
                    }
                    ret += "\":";
                    ret += j.second.dump();
                    ret += ',';
                }
                if(!val.object->empty()) ret.pop_back();
                ret += "}";
                return ret;
            }
            /* If it got to this point, it's of type INVALID_ITEM. */
            dbg << (type == INVALID_ITEM ? "INVALID" : "NOT INVALID BUT TRAPPING") << '\n';
            __builtin_trap();
        }

        /* Static functions */
        // {{{
        static json make_obj(const std::initializer_list< std::pair<std::string, json> >& t){
            json ret(OBJECT);
            std::vector< std::pair<std::string, json> > v(t);
            for(int i = 0; i < v.size(); i++){
                ret[v[i].first] = v[i].second;
            }
            return ret;
        }
        static json make_array(const std::initializer_list<json>& t){
            json ret(ARRAY);
            ret.val.array = new std::vector<json>(t);
            return ret;
        }
        static bool is_space(char c){
            // Whitespace in JSON is any of 0x20 (space), 0x09 (horizontal tab), 0x0a (\n), 0x0d (\r).
            return (c == 0x20 || c == 0x09 || c == 0x0a || c == 0x0d);
        }

        /* Parses a std::string that would appear in a JSON.
         * Returns the std::string, and a pointer showing the one directly _after_ it finished parsing.
         * If the std::string was invalid, the pointer will be (the actual pointer) + (length passed to this function) + 1,
         * (so > checks are easy to do.)
         */
        static std::pair<std::string, const char*> parse_string_incomplete(const char *str, const char *end){
            /* This is not actually that simple. 
             * Excerpt from RFC:
             * char = unescaped /
                  escape (
                      %x22 /          ; "    quotation mark  U+0022
                      %x5C /          ; \    reverse solidus U+005C
                      %x2F /          ; /    solidus         U+002F
                      %x62 /          ; b    backspace       U+0008
                      %x66 /          ; f    form feed       U+000C
                      %x6E /          ; n    line feed       U+000A
                      %x72 /          ; r    carriage return U+000D
                      %x74 /          ; t    tab             U+0009
                      %x75 4HEXDIG )  ; uXXXX
              */
            size_t length = end - str;
            std::string result;
            // Check if str starts with '"'.
            if(*str != '"') return { result, str + length + 1 };
            str++;
            while(*str != '"' && str != end){
                /* Annoying part. */
                if(*str == '\\'){
                    str++;
                    /* Literally mean themselves. */
                    if(*str == '"' || *str == '\\' || *str == '/'){
                        result += *str;
                    /* Check the RFC excerpt above. Don't mean themselves. */
                    } else if(*str == 'b'){
                        result += (char)0x08;
                    } else if(*str == 'f'){
                        result += (char)0x0c;
                    } else if(*str == 'n'){
                        result += (char)0x0a;
                    } else if(*str == 'r'){
                        result += (char)0x0d;
                    } else if(*str == 't'){
                        result += (char)0x09;
                    } else if(*str == 'u') {
                        /* Oh god. Unicode handling. */
                        // In format \uXXXX 
                        int codepoint = 0;
                        for(int mul = 4096; mul; mul >>= 4){
                            str++;
                            char c = tolower(*str);
                            if('a' <= c && c <= 'f'){
                                codepoint += (int)(c - 'a' + 10) * mul;
                            } else if('0' <= c && c <= '9'){
                                codepoint += (int)(c - '0' + 10) * mul;
                            } else {
                                // Invalid.
                                return { result, str + length + 1 };
                            }
                        }
                        if(codepoint <= 0x007f){ // ASCII range (1 byte)
                            result += (char)codepoint;
                        } else if(codepoint <= 0x07ff){ // 2 byte
                            // Bytes in binary form: 110xxxxx 10xxxxxx, where the `x`s are the binary digits
                            //                         5 bits  6 bits
                            result += (char)(0b11111000000 & codepoint);
                            result += (char)(0b00000111111 & codepoint);
                        } else if(codepoint <= 0xffff){ // 3 byte
                            // Bytes in binary form: 1110xxxx 10xxxxxx 10xxxxxx
                            //                        4 bits   6 bits   6 bits
                            result += (char)(0b1111000000000000 & codepoint);
                            result += (char)(0b0000111111000000 & codepoint);
                            result += (char)(0b0000000000111111 & codepoint);
                        }
                        /* Thankfully, we do not have to handle U+10000 to U+10FFFF. */
                    } else {
                        // Invalid.
                        return { result, str + length + 1 };
                    }
                    str++;
                } else {
                    result += *str;
                    str++;
                }
            }
            if(str == end){
                return { result, str + length + 1 };
            }
            // Get rid of last quote.
            str++;
            return { result, str };
        }

        /* Time to beat RapidJSON. */
        /* Just to make life easier: */
#define skip_ws() do { \
    while(curr != end && is_space(*curr)) curr++; \
    if(curr == end) return { json(INVALID_ITEM), curr }; \
} while(0);
#define expect(c) do { \
    if(curr == end || *curr != c) return { json(INVALID_ITEM), curr }; \
    curr++; \
} while(0);
#ifdef DEBUG
#define invalid_(p) std::move(std::make_pair(json(INVALID_ITEM), (const char *)p + ((int)(bool)(dbg << "INVALID AT " << __LINE__ << '\n') & 0)))
#else
#define invalid_(p) std::move(std::make_pair(json(INVALID_ITEM), (const char *)p))
#endif
        /* Parses the JSON given to it. Returns a std::pair<json, const char*>.
         * If the JSON was invalid, it returns a json of type INVALID_ITEM.
         * The char* shows how far it parsed (or where the JSON ended.)
         */
        static std::pair<json, const char*> parse_incomplete(const char *inp, const char *end){
            // JSON types: object, array, std::string, numbers, boolean, null
            // Current character.
            const char *curr = inp;
            // result.
            json result(INVALID_ITEM);

            while(true){
                skip_ws();

                /* Object
                 * { "key1": <member1> , "key2": <member2> , ... }
                 */
                if(*curr == '{'){
                    result = OBJECT;
                    curr++;
                    skip_ws();
                    // Check for empty object.
                    if(*curr == '}'){
                        return std::move(std::make_pair(result, curr+1));
                    }
                    for(;;){
                        skip_ws();
                        auto key = parse_string_incomplete(curr, end);
                        if(key.second > end){
                            return invalid_(key.second - (end-curr) - 1);
                        }
                        curr = key.second;
                        skip_ws();
                        expect(':');
                        /* Recursive time. */
                        auto value = parse_incomplete(curr, end);
                        if(value.first.invalid()){
                            return invalid_(value.second);
                        }
                        curr = value.second;
                        result[key.first] = std::move(value.first);
                        skip_ws();
                        if(*curr == '}'){
                            // End of object.
                            return std::move(std::make_pair(result, curr+1));
                        } else {
                            expect(',');
                        }
                    }
                }

                /* Array
                 * [ <member1> , <member2> , ... ]
                 */
                else if(*curr == '['){
                    result = ARRAY;
                    curr++;
                    skip_ws();
                    // Check for empty array.
                    if(*curr == ']'){
                        return std::move(std::make_pair(result, curr+1));
                    }
                    for(;;){
                        std::pair<json, const char *> value = parse_incomplete(curr, end);
                        if(value.first.invalid()){
                            return invalid_(value.second);
                        }
                        curr = value.second;
                        result.push_back(std::move(value.first));
                        skip_ws();
                        if(*curr == ']'){
                            // End array
                            return std::move(std::make_pair(result, curr+1));
                        } else {
                            expect(',');
                        }
                    }
                }

                /* String
                 * "x"
                 */
                else if(*curr == '"'){
                    result = STRING;
                    std::pair<std::string, const char *> str = parse_string_incomplete(curr, end);
                    if(str.second > end){
                        return invalid_(str.second - (end-curr) - 1);
                    }
                    curr = str.second;
                    result.as_str() = str.first;
                    return std::move(std::make_pair(result, curr));
                }

                /* Number
                 * 31.415e-1
                 */
                else if(('0' <= *curr && *curr <= '9') || *curr == '-'){
                    // Non-zero numbers can't start with 0.
                    // We will ignore that restriction.
                    const char *num_start = curr;
                    if(*curr == '-') curr++;
                    while(curr != end && ('0' <= *curr && *curr <= '9')){
                        curr++;
                    }
                    if(curr != end && (*curr == '.' || tolower(*curr) == 'e')){
                        /* Decimal */
                        result = DECIMAL;
                        std::istringstream istr(num_start);
                        istr.imbue(std::locale::classic());
                        if(!(istr >> result.val.decimal)){
                            // Error
                            return invalid_(num_start + istr.tellg());
                        }
                        return std::move(std::make_pair(result, num_start + istr.tellg()));
                    } else {
                        /* Integer */
                        result = INTEGER;
                        std::from_chars(num_start, end, result.val.integer);
                        // We already know it's valid and therefore don't need
                        // to check if the result errored.
                        return std::move(std::make_pair(result, curr));
                    }
                }

                /* Booleans
                 * true
                 * false
                 */
                else if(*curr == 't'){
                    result = BOOLEAN;
                    curr++;
                    expect('r');
                    expect('u');
                    expect('e');
                    result.val.boolean = true;
                    return std::move(std::make_pair(result, curr));
                } else if(*curr == 'f'){
                    result = BOOLEAN;
                    curr++;
                    expect('a');
                    expect('l');
                    expect('s');
                    expect('e');
                    result.val.boolean = false;
                    return std::move(std::make_pair(result, curr));
                }

                /* Null
                 * null
                 */
                else if(*curr == 'n'){
                    result = UNDEFINED;
                    curr++;
                    expect('u');
                    expect('l');
                    expect('l');
                    return std::move(std::make_pair(result, curr));
                }

                // Otherwise, it's invalid.
                return invalid_(curr);
            }
        }
#undef expect
#undef skip_ws
#undef invalid_
        static json parse(const char *input, const char *end){
            auto p = parse_incomplete(input, end);
            // Don't waste unnecessary CPU cycles.
            if(p.first.type == INVALID_ITEM){
                p.first.val.invalid_end = p.second;
                return p.first;
            }
            // if is not end of string (minus whitespace)
            while(p.second != end){
                if(!is_space(*p.second)){
                    json res(INVALID_ITEM);
                    res.val.invalid_end = p.second;
                    return res;
                }
                p.second++;
            }
            return p.first;
        }
        static json parse(const std::string& input){
            return parse(input.c_str(), input.c_str() + input.size());
        }
        // }}}
    };
}

#undef dbg

#endif /* HEX_JSON_HPP */
