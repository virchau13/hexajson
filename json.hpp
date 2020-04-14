#ifndef HEX_JSON_HPP
#define HEX_JSON_HPP

#include <iostream>
#include <vector>
#include <map>
#include <charconv>

#ifdef DEBUG
#define dbg std::cerr
#else
#define dbg if(0) std::cerr
#endif

namespace hex {
    /* JSON types as described in the RFC. */
    typedef uint8_t val_type;
    val_type
        UNDEFINED = 1,
        DECIMAL = 2,
        INTEGER = 3,
        STRING = 4,
        BOOLEAN = 5,
        ARRAY = 6,
        OBJECT = 7,
        INVALID_ITEM = 0;
    class json;
    typedef std::map<std::string, json> table;
    typedef std::vector<json> array_t;
    union value {
        double decimal;
        int64_t integer;
        table *object;
        array_t *array;
        std::string *str;
        bool boolean;
        const char *invalid_end;
    };
    class json {
        public:
        value val;
        val_type type;
        // constructors and destructors
        // {{{
        json(const json& rhs){
            type = rhs.type;
            if(type == OBJECT) val.object = new table(*rhs.val.object);
            else if(type == ARRAY){
                val.array = new array_t(*rhs.val.array);
            }
            else if(type == STRING) val.str = new std::string(*rhs.val.str);
            else val = rhs.val;
        }
        json(json&& rhs) noexcept {
            type = rhs.type;
            /* Move constructor, more like pilfer constructor. */
            val = rhs.val;
            if(rhs.type == OBJECT) rhs.val.object = nullptr;
            if(rhs.type == ARRAY) rhs.val.array = nullptr;
            if(rhs.type == STRING) rhs.val.str = nullptr;
            rhs.type = INVALID_ITEM;
        }
        json(const val_type& t = OBJECT){
            type = t;
            if(t == OBJECT) val.object = new table();
            if(t == ARRAY) val.array = new array_t();
            if(t == STRING) val.str = new std::string();
        }
        json(const std::string& rhs){
            type = STRING;
            val.str = new std::string(rhs);
        }
        json(const char* rhs){
            std::string s(rhs);
            operator=(s);
        }
        json(int rhs){
            operator=(rhs);
        }
        json(int64_t rhs){
            operator=(rhs);
        }
        json(double rhs){
            operator=(rhs);
        }
        void clean_type() noexcept {
            if(type == OBJECT) delete val.object;
            if(type == ARRAY){
                delete val.array;
            }
            if(type == STRING) delete val.str;
            type = INVALID_ITEM;
        }
        ~json() noexcept {
            clean_type();
        }
        // }}}
        // operators
        // {{{
        const json& operator=(const json rhs){
            clean_type();
            type = rhs.type;
            if(type == OBJECT) val.object = new table(*rhs.val.object);
            else if(type == ARRAY){
                val.array = new array_t(*rhs.val.array);
            }
            else if(type == STRING) val.str = new std::string(*rhs.val.str);
            else val = rhs.val;
            return *this;
        }
        const json& operator=(const val_type& t){
            clean_type();
            type = t;
            if(t == OBJECT) val.object = new table();
            if(t == ARRAY) val.array = new array_t();
            if(t == STRING) val.str = new std::string();
            return *this;
        }
        const json& operator=(const std::string& rhs){
            clean_type();
            type = STRING;
            val.str = new std::string(rhs);
            return *this;
        }
        const json& operator=(int rhs){
            clean_type();
            type = INTEGER;
            val.integer = rhs;
            return *this;
        }
        const json& operator=(int64_t rhs){
            clean_type();
            type = INTEGER;
            val.integer = rhs;
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
        const json& operator=(const array_t& rhs){
            clean_type();
            type = ARRAY;
            val.array = new array_t(rhs);
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
            return (*val.array)[idx];
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
        void pop_back(){
            val.array->pop_back();
        }
        inline bool invalid(){
            return type == INVALID_ITEM;
        }
        inline const size_t size() const noexcept {
            return (type == OBJECT ? val.object->size() :
                    type == ARRAY ? val.array->size() :
                    /* type == STRING */ val.str->size());
        }
        inline json& back() noexcept {
            return val.array->back();
        }
        inline array_t& as_arr() noexcept {
            return *val.array;
        }
        inline table& as_obj() noexcept {
            return *val.object;
        }
        // }}}
        // stringify functions
        // {{{
        std::string dump(){
            // FIXME: locale independent dump()
            if(type == INTEGER){
                return std::to_string(val.integer);
            }
            if(type == DECIMAL){
                return std::to_string(val.decimal);
            }
            if(type == BOOLEAN) return val.boolean ? "true" : "false";
            if(type == UNDEFINED) return "null";
            if(type == STRING) return '"' + (*val.str) + '"';
            if(type == ARRAY){
                std::string ret = "[";
                for(int i = 0; i < val.array->size(); i++){
                    ret += (*val.array)[i].dump();
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
        // }}}
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
        static json make_arr(const std::initializer_list<json>& t){
            json ret(ARRAY);
            ret.as_arr() = t;
            return ret;
        }
        static inline bool is_space(char c){
            // Whitespace in JSON is any of 0x20 (space), 0x09 (horizontal tab), 0x0a (\n), 0x0d (\r).
            return (c == 0x20 || c == 0x09 || c == 0x0a || c == 0x0d);
        }

        /* Parses a std::string that would appear in a JSON.
         * Puts the result value in the std::string, and returns a pointer showing the one directly _after_ it finished parsing.
         * If the std::string was invalid, the pointer will be (the actual pointer) + (length passed to this function) + 1,
         * (so > checks are easy to do.)
         */
        static const char *parse_string_incomplete(const char *str, const char *end, std::string& result){
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
            // Check if str starts with '"'.
            if(*str != '"') return str + length + 1;
            str++;
            while(*str != '"' && str != end){
                /* Annoying part. */
                if(*str < 0x20){
                    // These characters have to be escaped.
                    return str + length + 1;
                } else if(*str == '\\'){
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
                                return str + length + 1;
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
                        return str + length + 1;
                    }
                    str++;
                } else {
                    result += *str;
                    str++;
                }
            }
            if(str == end){
                return str + length + 1;
            }
            // Get rid of last quote.
            str++;
            return str;
        }

        /* Time to beat RapidJSON. */
        /* Just to make life easier: */
#define skip_ws() do { \
    while(curr != end && is_space(*curr)) curr++; \
    if(curr == end){ result = INVALID_ITEM; return end; } \
} while(0);
#define expect(c) do { \
    if(curr == end || *curr != c){ result = INVALID_ITEM; return curr; }; \
    curr++; \
} while(0);
        /* Parses the JSON given to it. Returns a std::pair<json, const char*>.
         * If the JSON was invalid, it returns a json of type INVALID_ITEM.
         * The char* shows how far it parsed (or where the JSON ended.)
         */
        static const char *parse_incomplete(const char *inp, const char *end, json& result){
            // JSON types: object, array, std::string, numbers, boolean, null
            // Current character.
            const char *curr = inp;

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
                    return curr+1;
                }
                for(;;){
                    skip_ws();
                    std::string key;
                    const char *next = parse_string_incomplete(curr, end, key);
                    if(next > end){
                        result = INVALID_ITEM;
                        return next - (end-curr) - 1;
                    }
                    curr = next;
                    skip_ws();
                    expect(':');
                    result[key] = INVALID_ITEM;
                    auto& valref = result[key];
                    /* Recursive time. */
                    next = parse_incomplete(curr, end, valref);
                    if(valref.invalid()){
                        result = INVALID_ITEM;
                        return next;
                    }
                    curr = next;
                    skip_ws();
                    if(*curr == '}'){
                        // End of object.
                        return curr+1;
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
                    return curr+1;
                }
                for(;;){
                    result.push_back(json(INVALID_ITEM));
                    const char *next = parse_incomplete(curr, end, result.back());
                    if(result.back().type == INVALID_ITEM){
                        result = INVALID_ITEM;
                        return next;
                    }
                    curr = next;
                    skip_ws();
                    if(*curr == ']'){
                        // End array
                        return curr+1;
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
                const char *next = parse_string_incomplete(curr, end, result.as_str());
                if(next > end){
                    result = INVALID_ITEM;
                    return next - (end-curr) - 1;
                }
                return next;
            }

            /* Number
             * 31.415e-1
             */
            else if(('0' <= *curr && *curr <= '9') || *curr == '-'){
                // Non-zero numbers can't start with 0.
                if(*curr == '0' && curr+1 != end && '0' <= *(curr+1) && *(curr+1) <= '9'){
                    result = INVALID_ITEM;
                    return curr;
                }
                const char *num_start = curr;
                if(*curr == '-') curr++;
                while(curr != end && ('0' <= *curr && *curr <= '9')){
                    curr++;
                }
                if(curr != end && (*curr == '.' || tolower(*curr) == 'e')){
                    /* Decimal */
                    result = DECIMAL;
                    if(*curr == '.'){
                        curr++;
                        while('0' <= *curr && *curr <= '9') curr++;
                    }
                    if(tolower(*curr) == 'e'){
                        curr++;
                        if(*curr == '+' || *curr == '-') curr++;
                        while('0' <= *curr && *curr <= '9') curr++;
                    }
                    char *endptr;
                    result.val.decimal = strtod(num_start, &endptr);
                    if(endptr != curr){
                        result = INVALID_ITEM;
                    }
                    return endptr;
                } else {
                    /* Integer */
                    result = INTEGER;
                    std::from_chars(num_start, curr, result.val.integer);
                    // We already know it's valid and therefore don't need
                    // to check if the result errored.
                    return curr;
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
                return curr;
            } else if(*curr == 'f'){
                result = BOOLEAN;
                curr++;
                expect('a');
                expect('l');
                expect('s');
                expect('e');
                result.val.boolean = false;
                return curr;
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
                return curr;
            }

            // Otherwise, it's invalid.
            result = INVALID_ITEM;
            return curr;
        }
#undef expect
#undef skip_ws
        static json parse(const char *input, const char *end){
            json result(INVALID_ITEM);
            const char *p = parse_incomplete(input, end, result);
            // Don't waste unnecessary CPU cycles.
            if(result.invalid()){
                result.val.invalid_end = p;
                return result;
            }
            // if is not end of string (minus whitespace)
            while(p != end){
                if(!is_space(*p)){
                    result = INVALID_ITEM;
                    result.val.invalid_end = p;
                    return result;
                }
                p++;
            }
            return result;
        }
        static json parse(const std::string& input){
            return parse(input.c_str(), input.c_str() + input.size());
        }
        // }}}
    };
}

#undef dbg

#endif /* HEX_JSON_HPP */
