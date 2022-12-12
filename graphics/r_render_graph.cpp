//* render graph
//      creates a render graph abstraction from a file

#include "r_render_graph.h"

#include "file.h"
#include "fresa_config.h"
#include "fresa_assert.h"

using namespace fresa;
using namespace graphics;

enum class State {
    None,
    Renderpass,
    Attachment
};

//* load render graph
//      from a file input fills the render graph abstraction
RenderGraph rg::loadRenderGraph() {
    RenderGraph r;
    RenderpassID current_renderpass;
    AttachmentID current_attachment;

    //: load file
    auto path = file::path(config.render_graph_path);
    //- hot reload
    std::ifstream f(path);
    strong_assert<str_view>(f.is_open(), "failed to open shader file '{}'", path);

    //: parse file
    str s;
    int line = 0;
    State state = State::None;
    while (std::getline(f, s)) {
        line++;
        //: skip blank lines and comments
        if (s.size() == 0 || s.at(0) == '#') continue;

        //: split into key and value using the separator ':'
        auto tokens = split(s, ':') | ranges::to_vector;
        strong_assert<int, str_view>(tokens.size() == 2 or (tokens.size() == 1 and s.at(s.size() - 1) == ':'),
                                     "invalid syntax, all lines must contain a separator token ':'\n"
                                     "at line ({}): '{}'", std::forward<int>(line), s);

        //: parse the key token
        str_view key = tokens.at(0);
        auto key_tokens = split(key, ' ') | ranges::to_vector;
        bool is_header = key_tokens.size() == 2;
        strong_assert<int, str_view>(is_header or (not is_header and key_tokens.size() == 1),
                                     "invalid syntax, wrong number of words\n"
                                     "at line ({}): '{}'", std::forward<int>(line), s);

        //: parse the value token
        strong_assert<int, str_view>((is_header and tokens.size() == 1) or (not is_header and tokens.size() == 2),
                                     "invalid syntax, wrong number of tokens\n"
                                     "at line ({}): '{}'", std::forward<int>(line), s);
        str_view value = is_header ? "" : tokens.at(1);

        //: header, check which type it is and set the state
        if (is_header) {
            switch (key.at(0)) {
                case 'p':
                    state = State::Renderpass;
                    current_renderpass = rid(key.substr(2));
                    r.renderpasses[current_renderpass] = {};
                    break;
                case 'a':
                    state = State::Attachment;
                    current_attachment = aid(key.substr(2));
                    r.attachments[current_attachment] = {};
                    break;
                default:
                    strong_assert<const char, int, str_view>(false, "invalid syntax, unknown token '{}'\n"
                                                                    "at line ({}): '{}'", std::forward<const char>(key.at(0)), std::forward<int>(line), s);
            }
        } 
        //: definition line, parse the values
        else {
            switch (state) {
                case State::Renderpass:
                    //- todo: create custom reflection field serializator for this
                    // log::info("renderpass: {} {}", key, value);
                    break;
                case State::Attachment:
                    // log::info("attachment: {} {}", key, value);
                    break;
                case State::None:
                    strong_assert<int, str_view>(false, "invalid syntax, no header token found\n"
                                                        "at line ({}): '{}'", std::forward<int>(line), s);
                    break;
            }
        }
    }

    return r;
}