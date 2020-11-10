#ifndef CISTRING_H
#define CISTRING_H

#include <string>

namespace gprc
{
// http://www.gotw.ca/gotw/029.htm
struct ci_char_traits : public std::char_traits<char> {
    static char to_upper(char ch) {
        return std::toupper((unsigned char) ch);
    }
    static bool eq(char c1, char c2) {
         return to_upper(c1) == to_upper(c2);
     }
    static bool lt(char c1, char c2) {
         return to_upper(c1) <  to_upper(c2);
    }
    static int compare(const char* s1, const char* s2, std::size_t n) {
        while ( n-- != 0 ) {
            if ( to_upper(*s1) < to_upper(*s2) ) return -1;
            if ( to_upper(*s1) > to_upper(*s2) ) return 1;
            ++s1; ++s2;
        }
        return 0;
    }
    static const char* find(const char* s, std::size_t n, char a) {
        auto const ua (to_upper(a));
        while ( n-- != 0 ) 
        {
            if (to_upper(*s) == ua)
                return s;
            s++;
        }
        return nullptr;
    }

    ci_char_traits(const std::char_traits<char>& c) : std::char_traits<char>{c} {}
};

class ci_string : public std::basic_string<char, ci_char_traits> {
    public:
    ci_string(const std::string& str)
        : std::basic_string<char, ci_char_traits>{str.c_str()} {}
};
};

#endif // CISTRING_H