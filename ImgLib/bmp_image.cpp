#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    uint16_t signature = 0x4D42;
    uint32_t fileSize = 0;
    uint32_t reserved = 0;
    uint32_t dataOffset = 54;
} PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t headerSize = 40;
    int32_t width = 0;
    int32_t height = 0;
    uint16_t planes = 1;
    uint16_t bitCount = 24;
    uint32_t compression = 0;
    uint32_t imageSize = 0;
    int32_t xPixelsPerMeter = 11811;
    int32_t yPixelsPerMeter = 11811;
    uint32_t colorUsed = 0;
    uint32_t colorsImportant = 0x1000000;
} PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
    if (!out) {
        return false;
    }
    
    const int width = image.GetWidth();
    const int height = image.GetHeight();
    const int stride = GetBMPStride(width);
    
    BitmapFileHeader file_header;
    file_header.fileSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + stride * height; 
    file_header.dataOffset = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);
    
    BitmapInfoHeader info_header;
    info_header.width = width;
    info_header.height = height;
    info_header.imageSize = stride * height;
    
    out.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    out.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));
    
    vector<char> row(stride);

    for (int y = height - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < width; ++x) {
            row[x * 3 + 0] = static_cast<char>(line[x].b);
            row[x * 3 + 1] = static_cast<char>(line[x].g); 
            row[x * 3 + 2] = static_cast<char>(line[x].r); 
        }
        out.write(row.data(), stride);
    }

    return true;
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    ifstream in(file, ios::binary);
    if (!in) {
        return {};
    }
    
    BitmapFileHeader file_header;
    in.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));

    if (file_header.signature != 0x4D42) {
        return {};
    }

    BitmapInfoHeader info_header;
    in.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));

    if (info_header.headerSize != 40 || info_header.bitCount != 24 || info_header.compression != 0) {
        return {};
    }

    const int width = info_header.width;
    const int height = info_header.height;
    const int stride = GetBMPStride(width);

    Image result(width, height, Color::Black());
    vector<char> row(stride);
    
    in.seekg(file_header.dataOffset, ios::beg);
    
    for (int y = height - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        in.read(row.data(), stride);
        for (int x = 0; x < width; ++x) {
            line[x].b = static_cast<byte>(row[x * 3 + 0]);
            line[x].g = static_cast<byte>(row[x * 3 + 1]);
            line[x].r = static_cast<byte>(row[x * 3 + 2]);
        }
    }    
    
    return result;
}

} 