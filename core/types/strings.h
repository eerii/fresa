//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include <algorithm>

namespace Fresa
{
    //: String
    using str = std::string;
    
    //: To lowercase
    inline str lower(str s) {
        for (auto &c : s)
            c = std::tolower(c);
        return s;
    }
    
    //: Split
    //      - Recursive (deleting all consecutive delimiters, "a b" is the same as "a   b")
    //      - With lists (conserving [ ... ] in a single entry)
    inline std::vector<str> split(str s, str del = " ", bool recursive = false, bool lists = false) {
        std::vector<str> ss{};
        int a = 0;
        int b = (int)s.find(del);
        while (b != -1) {
            ss.push_back(s.substr(a, b - a));
            a = (int)(b + del.size());
            if (recursive) {
                while (s.size() > a and s.substr(a, del.size()) == del)
                    a++;
            }
            if (lists and s.find("[", a) == a) {
                b = (int)s.find("]", a) + 1;
                int c = (int)s.find("[", a+1) + 1; c = c == 0 ? (int)s.size() : c;
                if (b == 0 or b > c) throw std::runtime_error("[ERROR] The list is not closed, check the brackets");
                if (b == s.size()) b = -1;
            } else {
                b = (int)s.find(del, a);
            }
        }
        ss.push_back(s.substr(a, b - a));
        return ss;
    }
    
    //: Get list contents (from a string like '[a b c]' returns 'a b c')
    inline str list_contents(str s) {
        return s.substr(1, s.size() - 2);
    }
}
