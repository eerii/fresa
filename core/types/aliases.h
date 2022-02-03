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
    inline std::vector<str> split(str s, str del = " ") {
        std::vector<str> ss{};
        int a = 0;
        int b = (int)s.find(del);
        while (b != -1) {
            ss.push_back(s.substr(a, b - a));
            a = (int)(b + del.size());
            b = (int)s.find(del, a);
        }
        ss.push_back(s.substr(a, b - a));
        return ss;
    }
}
