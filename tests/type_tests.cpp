//* types_tests

#include "unit_test.h"
#include "fresa_types.h"

namespace test
{
    using namespace fresa;

    //* strings
    inline TestSuite strings_test("strings", []{
        "lowercase conversion"_test = []{
            return expect(lower("FReSa") == "fresa");
        };
        "lowercase literal"_test = []{
            return expect("JoSEkOALaS"_lower == "josekoalas");
        };
        "lowercase with string variable"_test = []{
            str s = "AguACAtE!";
            return expect(lower(s) == "aguacate!");
        };
    });
}