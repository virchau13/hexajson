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
        bool created = true;
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
                return std::to_string(val.integer);
            }
            if(type == DECIMAL){
                // FIXME: work on different locales
                return std::to_string(val.decimal);
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
            /* Parses the JSON given to it. Returns a std::pair<json, char*>.
             * If the JSON was invalid, it returns a json of type INVALID_ITEM.
             * The char* shows how far it parsed (or where the JSON ended.)
             */
            static std::pair<json, const char*> parse_incomplete(const char *inp, const char *end){
            // JSON types: object, array, std::string, numbers, boolean, null
            const char *curr = inp;
            /* Checks whitespace before. */
            skip_ws();

            // <something> means instance of that something
            // ... means more

            /* Object 
             * { "member": <json> , "member2": <json2> , ... }
             */
            if(*curr == '{'){
                curr++;
                if(curr == end) return { json(INVALID_ITEM), curr };
                json result(OBJECT);
                // Check for empty object
                skip_ws();
                if(*curr == '}'){
                    curr++;
                    return { result, curr };
                }
                for(;;){
                    skip_ws();
                    // Parse key.
                    auto key = parse_string_incomplete(curr, end);
                    if(key.second > end){
                        // Invalid.
                        return { json(INVALID_ITEM), key.second - (end - curr) - 1 };
                    }
                    curr = key.second;
                    // Whitespace, then colon.
                    skip_ws();
                    expect(':');
                    // Parse member.
                    /* Recursive time. ;) */
                    auto value = parse_incomplete(curr, end);
                    curr = value.second;
                    if(value.first.type == INVALID_ITEM){
                        // Invalid, just stop parsing now to not waste unnecessary CPU cycles.
                        return { value.first, curr };
                    }
                    result[key.first] = value.first;
                    skip_ws();
                    if(*curr == '}'){
                        // Ends object.
                        // So just return it now.
                        curr++;
                        return { result, curr };
                    } else {
                        expect(',');
                    }
                }
            }

            /* Array
             * [ <json1> , <json2> , ... ]
             */
            else if(*curr == '['){
                curr++;
                json result(ARRAY);
                skip_ws();
                // check empty array
                if(*curr == ']'){
                    return { result, curr + 1 };
                }
                for(;;){
                    skip_ws();
                    // Parse item.
                    /* Recursive time, again. */
                    auto item = parse_incomplete(curr, end);
                    curr = item.second;
                    if(item.first.type == INVALID_ITEM){
                        // Invalid.
                        return { json(INVALID_ITEM), curr };
                    }
                    result.push_back(item.first);
                    skip_ws();
                    if(*curr == ']'){
                        // Ends array.
                        curr++;
                        return { result, curr };
                    } else {
                        expect(',');
                    }
                }
            }

            /* String 
             * "text\u0027nicode\"or spaces inside"
             */
            else if(*curr == '"'){
                auto p = parse_string_incomplete(curr, end);
                if(p.second > end){
                    // Invalid.
                    return { json(INVALID_ITEM), p.second - (end - curr) - 1 };
                }
                json result(p.first);
                return { result, p.second };
            }

            /* Number
             * 1238501.123910e100
             * To avoid octal confusion JSON numbers can't start with 0,
             * unless they _are_ 0.
             * But we'll ignore that, since any valid JSON generator
             * will spit out the correct values anyway. :)
             */
            else if(('0' <= *curr && *curr <= '9') || *curr == '-'){
                const char *num_start = curr;
                if(*num_start == '-') curr++;
                while(curr != end && '0' <= *curr && *curr <= '9') curr++;
                if(*curr == '.' || tolower(*curr) == 'e'){
                    curr++;
                    /* Floating point number. (either decimal point or exponent) */
                    json result(DECIMAL);
                    while(('0' <= *curr && *curr <= '9') || tolower(*curr) == 'e' || *curr == '-' || *curr == '+') curr++;
                    // FIXME: work on different locales
                    dbg << "FLOATING POINT\n";
                    // curr should be the last thing.
                    // so:
                    char *x;
                    double ret = strtod(num_start, &x);
                    if(x != curr){
                        // Invalid.
                        return { json(INVALID_ITEM), x };
                    }
                    // Valid.
                    result.as_double() = ret;
                    return { result, curr };
                } else {
                    /* Integer */
                    const char *num_end = curr;
                    json result(INTEGER);
                    result.val.integer = 0;
                    int mul = 1;
                    if(*num_start == '-'){
                        mul = -1;
                        num_start++;
                    }
                    while(curr != num_start){
                        curr--;
                        result.val.integer += (int64_t)(*curr - '0') * mul;
                        mul *= 10;
                    }
                    return { result, num_end };
                }
            }
            /* Booleans 
             * true
             * false
             */
            else if(*curr == 'f'){
                curr++;
                // Check equivalence, the stupid way. :)
                expect('a');
                expect('l');
                expect('s');
                expect('e');
                json result(BOOLEAN);
                result.val.boolean = false;
                return { result, curr };
            } else if(*curr == 't'){
                curr++;
                expect('r');
                expect('u');
                expect('e');
                json result(BOOLEAN);
                result.val.boolean = true;
                return { result, curr };
            }

            /* Null
             * null
             */
            else if(*curr == 'n'){
                curr++;
                expect('u');
                expect('l');
                expect('l');
                return { json(UNDEFINED), curr };
            }

            /* Otherwise, it's invalid. */
            return { json(INVALID_ITEM), curr };
        }
#undef expect
#undef skip_ws
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
