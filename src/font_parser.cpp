// this_file: src/font_parser.cpp
// Font file parser implementation
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#include "font_parser.h"
#include <fstream>
#include <cstring>

namespace FontParser {
// Font file parsing for TTF, OTF, TTC, and OTC formats

// File size validation constants
constexpr size_t MIN_FONT_FILE_SIZE = 100;           // Minimum valid font file size (bytes)
constexpr size_t MAX_FONT_FILE_SIZE = 50 * 1024 * 1024;  // Maximum font file size (50 MB)

// Font parsing constants
constexpr size_t MAX_NAME_TABLE_SIZE = 1024 * 1024;  // Maximum name table size (1 MB)
constexpr uint32_t MAX_FONTS_IN_COLLECTION = 256;    // Maximum fonts to process in TTC/OTC
constexpr uint16_t MAX_FONT_TABLES = 1000;           // Maximum number of tables in font file

// Font structure constants
constexpr uint32_t NAME_RECORD_SIZE = 12;            // Size of name table record (bytes)
constexpr uint32_t NAME_TABLE_HEADER_SIZE = 6;       // Size of name table header (format + count + stringOffset)
constexpr uint32_t FONT_HEADER_SIZE = 12;            // Size of font header (bytes)
constexpr uint32_t TABLE_RECORD_SIZE = 16;           // Size of table directory record (bytes)
constexpr uint32_t OFFSET_SIZE = 4;                  // Size of TTC offset table entry (bytes)

// Font header field offsets (per OpenType spec)
constexpr uint32_t FONT_NUM_TABLES_OFFSET = 4;      // numTables field offset

// TTC header field offsets (per OpenType spec)
constexpr uint32_t TTC_NUM_FONTS_OFFSET = 8;        // numFonts field offset

// Name table header field offsets (per OpenType spec)
constexpr uint32_t NAME_COUNT_OFFSET = 2;           // count field offset
constexpr uint32_t NAME_STRING_OFFSET = 4;          // stringOffset field offset

// Table directory record field offsets (per OpenType spec)
constexpr uint32_t TABLE_TAG_OFFSET = 0;            // tag field offset
constexpr uint32_t TABLE_CHECKSUM_OFFSET = 4;       // checksum field offset
constexpr uint32_t TABLE_OFFSET_OFFSET = 8;         // offset field offset
constexpr uint32_t TABLE_LENGTH_OFFSET = 12;        // length field offset

// Name record field offsets (per OpenType spec)
constexpr uint32_t NAME_PLATFORM_ID_OFFSET = 0;      // platformID field offset
constexpr uint32_t NAME_ENCODING_ID_OFFSET = 2;      // encodingID field offset
constexpr uint32_t NAME_LANGUAGE_ID_OFFSET = 4;      // languageID field offset
constexpr uint32_t NAME_NAME_ID_OFFSET = 6;          // nameID field offset
constexpr uint32_t NAME_LENGTH_OFFSET = 8;           // length field offset
constexpr uint32_t NAME_OFFSET_OFFSET = 10;          // offset field offset

// Font format signatures (per OpenType spec)
constexpr uint32_t TRUETYPE_SIGNATURE = 0x00010000;  // TrueType font signature
constexpr uint32_t OPENTYPE_SIGNATURE = 0x4F54544F;  // OpenType font signature ('OTTO')

// Font table tags (per OpenType spec)
constexpr uint32_t NAME_TABLE_TAG = 0x6E616D65;      // 'name' table tag
constexpr uint32_t TTC_HEADER_TAG = 0x74746366;      // 'ttcf' TrueType Collection tag

// Name table nameID values (per OpenType spec)
constexpr uint16_t NAME_ID_FONT_FAMILY = 1;          // Font Family name

// Helper: Read big-endian uint16
static uint16_t ReadUInt16BE(const uint8_t* const data) noexcept {
    return (data[0] << 8) | data[1];
}

// Helper: Read big-endian uint32
static uint32_t ReadUInt32BE(const uint8_t* const data) noexcept {
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

// Helper: Convert UTF-16BE to ASCII (basic conversion)
static std::string UTF16BEToString(const uint8_t* const data, const uint16_t length) noexcept {
    std::string result;
    for (uint16_t i = 0; i < length; i += 2) {
        uint16_t ch = ReadUInt16BE(data + i);
        if (ch < 128) result += static_cast<char>(ch);
    }
    return result;
}

// Helper: Parse name table to extract font family name (nameID=1)
static std::string ExtractNameFromTable(const uint8_t* const nameTable, const uint32_t tableSize) {
    if (tableSize < NAME_TABLE_HEADER_SIZE) return "";

    uint16_t format = ReadUInt16BE(nameTable);
    uint16_t count = ReadUInt16BE(nameTable + NAME_COUNT_OFFSET);
    uint16_t stringOffset = ReadUInt16BE(nameTable + NAME_STRING_OFFSET);

    // Validate count is reasonable (prevent excessive iteration with corrupted files)
    constexpr uint16_t MAX_NAME_RECORDS = 1000;
    if (count > MAX_NAME_RECORDS) return "";

    // Validate stringOffset is within table bounds
    if (stringOffset >= tableSize) return "";

    // Search for family name (nameID = 1)
    for (uint16_t i = 0; i < count; i++) {
        uint32_t recordOffset = NAME_TABLE_HEADER_SIZE + i * NAME_RECORD_SIZE;
        if (recordOffset + NAME_RECORD_SIZE > tableSize) break;

        const uint8_t* record = nameTable + recordOffset;
        uint16_t platformID = ReadUInt16BE(record + NAME_PLATFORM_ID_OFFSET);
        uint16_t encodingID = ReadUInt16BE(record + NAME_ENCODING_ID_OFFSET);
        uint16_t languageID = ReadUInt16BE(record + NAME_LANGUAGE_ID_OFFSET);
        uint16_t nameID = ReadUInt16BE(record + NAME_NAME_ID_OFFSET);
        uint16_t length = ReadUInt16BE(record + NAME_LENGTH_OFFSET);
        uint16_t offset = ReadUInt16BE(record + NAME_OFFSET_OFFSET);

        // Looking for Font Family name
        if (nameID != NAME_ID_FONT_FAMILY) continue;

        // Prefer Windows platform (3) with Unicode encoding (1)
        if (platformID == 3 && encodingID == 1) {
            // Check for overflow in offset arithmetic
            if (offset > tableSize || stringOffset > tableSize - offset) continue;
            uint32_t strOffset = stringOffset + offset;
            // Check for overflow in length arithmetic
            if (length > tableSize - strOffset) continue;
            if (strOffset + length <= tableSize) {
                return UTF16BEToString(nameTable + strOffset, length);
            }
        }
        // Fallback to Mac platform (1)
        else if (platformID == 1) {
            // Check for overflow in offset arithmetic
            if (offset > tableSize || stringOffset > tableSize - offset) continue;
            uint32_t strOffset = stringOffset + offset;
            // Check for overflow in length arithmetic
            if (length > tableSize - strOffset) continue;
            if (strOffset + length <= tableSize) {
                return std::string(reinterpret_cast<const char*>(nameTable + strOffset), length);
            }
        }
    }
    return "";
}

// Helper: Read font tables starting at file offset and extract name
static std::string ParseFontAtOffset(std::ifstream& file, uint32_t offset) {
    // Get file size to validate offset
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    if (offset >= static_cast<uint32_t>(fileSize)) return "";  // Offset beyond file size

    file.seekg(offset);

    uint8_t header[FONT_HEADER_SIZE];
    if (!file.read(reinterpret_cast<char*>(header), FONT_HEADER_SIZE)) return "";

    // Validate font signature (TrueType or OpenType)
    uint32_t signature = ReadUInt32BE(header);
    if (signature != TRUETYPE_SIGNATURE && signature != OPENTYPE_SIGNATURE) return "";

    uint16_t numTables = ReadUInt16BE(header + FONT_NUM_TABLES_OFFSET);

    // Validate numTables is reasonable (prevent excessive iteration with corrupted files)
    if (numTables > MAX_FONT_TABLES) return "";

    // Find 'name' table
    for (uint16_t i = 0; i < numTables; i++) {
        uint8_t tableRecord[TABLE_RECORD_SIZE];
        if (!file.read(reinterpret_cast<char*>(tableRecord), TABLE_RECORD_SIZE)) return "";

        uint32_t tag = ReadUInt32BE(tableRecord + TABLE_TAG_OFFSET);
        uint32_t tableOffset = ReadUInt32BE(tableRecord + TABLE_OFFSET_OFFSET);
        uint32_t tableLength = ReadUInt32BE(tableRecord + TABLE_LENGTH_OFFSET);

        // Check if this is the 'name' table
        if (tag == NAME_TABLE_TAG) {
            // Sanity check: name table shouldn't exceed maximum size
            if (tableLength == 0 || tableLength > MAX_NAME_TABLE_SIZE) return "";

            std::vector<uint8_t> nameTable(tableLength);
            file.seekg(tableOffset);
            if (!file.read(reinterpret_cast<char*>(nameTable.data()), tableLength)) return "";

            std::string name = ExtractNameFromTable(nameTable.data(), tableLength);
            if (!name.empty()) return name;
        }
    }
    return "";
}

bool IsCollection(const char* fontPath) {
    std::ifstream file(fontPath, std::ios::binary);
    if (!file) return false;

    uint8_t header[4];
    if (!file.read(reinterpret_cast<char*>(header), 4)) return false;

    uint32_t tag = ReadUInt32BE(header);
    return tag == TTC_HEADER_TAG; // Check for 'ttcf' TrueType Collection
}

// Helper: Extract filename without path or extension for fallback font naming
static std::string ExtractFilenameWithoutExtension(const char* fontPath) {
    std::string path(fontPath);
    size_t lastSlash = path.find_last_of("\\/");
    size_t lastDot = path.find_last_of('.');

    if (lastSlash == std::string::npos) lastSlash = 0;
    else lastSlash++;

    if (lastDot > lastSlash) {
        return path.substr(lastSlash, lastDot - lastSlash);
    } else {
        return path.substr(lastSlash);
    }
}

std::string GetFontName(const char* fontPath) {
    std::ifstream file(fontPath, std::ios::binary);
    if (!file) return "";

    // Validate file size: must be within valid range
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize < MIN_FONT_FILE_SIZE || fileSize > MAX_FONT_FILE_SIZE) {
        return "";  // File too small or too large to be valid font
    }

    std::string name = ParseFontAtOffset(file, 0);
    if (name.empty()) {
        name = ExtractFilenameWithoutExtension(fontPath);
    }

    return name;
}

std::vector<std::string> GetFontsInCollection(const char* fontPath) {
    std::vector<std::string> names;
    std::ifstream file(fontPath, std::ios::binary);
    if (!file) return names;

    // Validate file size: must be within valid range
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize < MIN_FONT_FILE_SIZE || fileSize > MAX_FONT_FILE_SIZE) {
        return names;  // File too small or too large to be valid font
    }

    uint8_t header[FONT_HEADER_SIZE];
    if (!file.read(reinterpret_cast<char*>(header), FONT_HEADER_SIZE)) return names;

    uint32_t tag = ReadUInt32BE(header);
    if (tag != TTC_HEADER_TAG) return names; // Not a TrueType Collection

    uint32_t numFonts = ReadUInt32BE(header + TTC_NUM_FONTS_OFFSET);

    // Validate numFonts is reasonable (0 or suspiciously large values indicate corruption)
    if (numFonts == 0 || numFonts > MAX_FONTS_IN_COLLECTION) return names;

    // Reserve capacity to avoid reallocations during iteration
    names.reserve(numFonts);

    for (uint32_t i = 0; i < numFonts; i++) {
        uint8_t offsetData[OFFSET_SIZE];
        if (!file.read(reinterpret_cast<char*>(offsetData), OFFSET_SIZE)) break;

        uint32_t fontOffset = ReadUInt32BE(offsetData);
        // Validate offset is within file bounds
        if (fontOffset >= static_cast<uint32_t>(fileSize)) continue;

        std::string name = ParseFontAtOffset(file, fontOffset);

        if (!name.empty()) {
            names.push_back(name);
        }

        // Return to offset table for next font
        // Check for overflow: i + 1 must not exceed UINT32_MAX / OFFSET_SIZE
        if (i + 1 > UINT32_MAX / OFFSET_SIZE) break;
        file.seekg(FONT_HEADER_SIZE + (i + 1) * OFFSET_SIZE);
        if (!file.good()) break;  // Seek failed, stop processing
    }

    return names;
}

} // namespace FontParser
