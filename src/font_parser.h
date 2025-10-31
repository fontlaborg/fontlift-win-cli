// this_file: src/font_parser.h
// Font file parser for fontlift-win-cli
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#ifndef FONT_PARSER_H
#define FONT_PARSER_H

#include <string>
#include <vector>

namespace FontParser {
    // Extract font family name from TTF/OTF file
    // Returns empty string if parsing fails
    std::string GetFontName(const char* fontPath);

    // Extract all font names from TTC/OTC collection
    // Returns empty vector if parsing fails
    std::vector<std::string> GetFontsInCollection(const char* fontPath);

    // Check if file is a font collection (TTC/OTC)
    bool IsCollection(const char* fontPath);
}

#endif // FONT_PARSER_H
