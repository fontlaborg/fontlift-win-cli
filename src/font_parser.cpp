// this_file: src/font_parser.cpp
// Font file parser implementation
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#include "font_parser.h"
#include <fstream>
#include <cstring>

namespace FontParser {

// Helper: Read big-endian uint16
static uint16_t ReadUInt16BE(const uint8_t* data) {
    return (data[0] << 8) | data[1];
}

// Helper: Read big-endian uint32
static uint32_t ReadUInt32BE(const uint8_t* data) {
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

// Helper: Convert UTF-16BE to ASCII (basic conversion)
static std::string UTF16BEToString(const uint8_t* data, uint16_t length) {
    std::string result;
    for (uint16_t i = 0; i < length; i += 2) {
        uint16_t ch = ReadUInt16BE(data + i);
        if (ch < 128) result += (char)ch;
    }
    return result;
}

// Extract name from name table
static std::string ExtractNameFromTable(const uint8_t* nameTable, uint32_t tableSize) {
    if (tableSize < 6) return "";

    uint16_t format = ReadUInt16BE(nameTable);
    uint16_t count = ReadUInt16BE(nameTable + 2);
    uint16_t stringOffset = ReadUInt16BE(nameTable + 4);

    // Search for family name (nameID = 1)
    for (uint16_t i = 0; i < count; i++) {
        uint32_t recordOffset = 6 + i * 12;
        if (recordOffset + 12 > tableSize) break;

        const uint8_t* record = nameTable + recordOffset;
        uint16_t platformID = ReadUInt16BE(record);
        uint16_t encodingID = ReadUInt16BE(record + 2);
        uint16_t languageID = ReadUInt16BE(record + 4);
        uint16_t nameID = ReadUInt16BE(record + 6);
        uint16_t length = ReadUInt16BE(record + 8);
        uint16_t offset = ReadUInt16BE(record + 10);

        // Looking for nameID 1 (Font Family)
        if (nameID != 1) continue;

        // Prefer Windows platform (3) with Unicode encoding (1)
        if (platformID == 3 && encodingID == 1) {
            uint32_t strOffset = stringOffset + offset;
            if (strOffset + length <= tableSize) {
                return UTF16BEToString(nameTable + strOffset, length);
            }
        }
        // Fallback to Mac platform (1)
        else if (platformID == 1) {
            uint32_t strOffset = stringOffset + offset;
            if (strOffset + length <= tableSize) {
                return std::string((char*)(nameTable + strOffset), length);
            }
        }
    }
    return "";
}

// Parse font at given offset in file
static std::string ParseFontAtOffset(std::ifstream& file, uint32_t offset) {
    file.seekg(offset);

    uint8_t header[12];
    if (!file.read((char*)header, 12)) return "";

    uint16_t numTables = ReadUInt16BE(header + 4);

    // Find 'name' table
    for (uint16_t i = 0; i < numTables; i++) {
        uint8_t tableRecord[16];
        if (!file.read((char*)tableRecord, 16)) return "";

        uint32_t tag = ReadUInt32BE(tableRecord);
        uint32_t tableOffset = ReadUInt32BE(tableRecord + 8);
        uint32_t tableLength = ReadUInt32BE(tableRecord + 12);

        // 'name' = 0x6E616D65
        if (tag == 0x6E616D65) {
            // Sanity check: name table shouldn't be > 1MB
            if (tableLength == 0 || tableLength > 1024 * 1024) return "";

            std::vector<uint8_t> nameTable(tableLength);
            file.seekg(tableOffset);
            if (!file.read((char*)nameTable.data(), tableLength)) return "";

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
    if (!file.read((char*)header, 4)) return false;

    uint32_t tag = ReadUInt32BE(header);
    return tag == 0x74746366; // 'ttcf'
}

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

    // Validate file size: must be between 100 bytes and 50MB
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize < 100 || fileSize > 50 * 1024 * 1024) {
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

    // Validate file size: must be between 100 bytes and 50MB
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize < 100 || fileSize > 50 * 1024 * 1024) {
        return names;  // File too small or too large to be valid font
    }

    uint8_t header[12];
    if (!file.read((char*)header, 12)) return names;

    uint32_t tag = ReadUInt32BE(header);
    if (tag != 0x74746366) return names; // Not 'ttcf'

    uint32_t numFonts = ReadUInt32BE(header + 8);

    for (uint32_t i = 0; i < numFonts && i < 256; i++) {
        uint8_t offsetData[4];
        if (!file.read((char*)offsetData, 4)) break;

        uint32_t fontOffset = ReadUInt32BE(offsetData);
        std::string name = ParseFontAtOffset(file, fontOffset);

        if (!name.empty()) {
            names.push_back(name);
        }

        // Return to offset table for next font
        file.seekg(12 + (i + 1) * 4);
    }

    return names;
}

} // namespace FontParser
