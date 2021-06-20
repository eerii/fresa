//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_font.h"

#include <filesystem>
#include <fstream>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype_fresa.h"

#include "r_renderer.h"
#include "log.h"

using namespace Verse;

void Graphics::Font::load(FontInfo* font, str path) {
    std::filesystem::path font_path{path};
    
    if (not std::filesystem::exists(font_path)) {
        log::error("The font file that you attempted to read (" + path + ") does not exist");
        return;
    }
    
    font->path = path;
    
    font->font_size = std::filesystem::file_size(font_path);
    font->data = std::vector<ui8>(font->font_size);
    
    std::ifstream f(path, std::ios::binary);
    f.read(reinterpret_cast<char*>(font->data.data()), font->font_size);
    
    stbtt_InitFont(&font->info, font->data.data(), stbtt_GetFontOffsetForIndex(font->data.data(), 0));
}

void Graphics::Font::render(Component::Text* text, Vec2 size, int line_height) {
    float scale = stbtt_ScaleForPixelHeight(&text->font->info, line_height);
    
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&text->font->info, &ascent, &descent, &line_gap);
        
    ascent = roundf(ascent * scale);
    descent = roundf(descent * scale);
    
    text->bitmap = std::vector<ui8>(size.x * size.y, 0);
    
    int x = 0;
    
    for (int i = 0; i < text->text.size(); i++) {
        int advance_width; int left_side_bearing;
        stbtt_GetCodepointHMetrics(&text->font->info, text->text[i], &advance_width, &left_side_bearing);
        
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&text->font->info, text->text[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
        
        int y = ascent + c_y1;
        
        int byte_offset = x + roundf(left_side_bearing * scale) + (y * size.x);
        stbtt_MakeCodepointBitmap(&text->font->info, text->bitmap.data() + byte_offset,
                                  c_x2 - c_x1, c_y2 - c_y1, size.x, scale, scale, text->text[i]);
        
        x += roundf(advance_width * scale);
        
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&text->font->info, text->text[i], text->text[i + 1]);
        x += roundf(kern * scale);
    }
    
    text->tex_id = Graphics::Renderer::createTexture(text->bitmap.data(), size.x, size.y, false);
    text->bitmap_size = size;
    text->line_height = line_height;
}
