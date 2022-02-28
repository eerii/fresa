//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

namespace Fresa
{
    //---Unsigned int---
    using ui8 = std::uint8_t;
    using ui16 = std::uint16_t;
    using ui32 = std::uint32_t;
    using ui64 = std::uint64_t;

    //---String---
    using str = std::string;
    inline void lower(str &s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    }
    inline std::vector<str> split(str s, str del = " ", bool recursive = false) {
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
            b = (int)s.find(del, a);
        }
        ss.push_back(s.substr(a, b - a));
        return ss;
    }
}
