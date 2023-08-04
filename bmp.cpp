#include "bmp.h"
#include <fstream>
#include <iostream>
#include <cstring>

void BMP::Write(char *file_name) {
    std::ofstream file(file_name, std::ios::binary);
    file.write(reinterpret_cast<char *>(&header_), sizeof(header_));
    file.write(reinterpret_cast<char *>(&info_), sizeof(info_));
    for (int32_t y = info_.height - 1; y >= 0; y--) {
        for (int32_t x = 0; x < info_.width; x++) {
            uint8_t b = static_cast<uint8_t>(matrix_(y, x).blue);
            uint8_t g = static_cast<uint8_t>(matrix_(y, x).green);
            uint8_t r = static_cast<uint8_t>(matrix_(y, x).red);
            file.write(reinterpret_cast<char *>(&b), sizeof(b));
            file.write(reinterpret_cast<char *>(&g), sizeof(g));
            file.write(reinterpret_cast<char *>(&r), sizeof(r));
        }

        // заглушка для выравнивания строк по границе 4-х байт
        uint8_t padding = 0;
        for (uint32_t i = 0; i < (4 - ((info_.width * sizeof(char) * 3) % 4)) % 4; i++) {
            file.write(reinterpret_cast<char *>(&padding), sizeof(padding));
        }
    }
    file.close();
}
void BMP::Read(char *file_name) {

    std::ifstream file(file_name, std::ios::binary);
    if (!file.is_open()) {
        throw FileReadErrorException("could not open file");
    }
    ReadHeader(file, file_name);
    int32_t width = info_.width;
    int32_t height = info_.height;
    matrix_ = TMatrix<RGB>(height, width);
    height_ = height;
    width_ = width;
    ReadPixels(file, file_name);

    file.close();
}
BMP::BMP() {
}
std::pair<size_t, size_t> BMP::GetSize() {
    return {width_, height_};
}
void BMP::Resize(size_t new_height, size_t new_width) {
    matrix_.Resize(new_width, new_height);
    const int32_t header_size = 54;
    header_.file_size = header_size + new_height * new_width * 3;
    width_ = new_width;
    height_ = new_height;
    info_.width = static_cast<int32_t>(new_width);
    info_.height = static_cast<int32_t>(new_height);
}
RGB &BMP::operator()(size_t height, size_t width) {
    return matrix_(height, width);
}
TMatrix<RGB> &BMP::GetMatrix() {
    return matrix_;
}
void BMP::ReadHeader(std::ifstream &file, char *file_name) {

    file.read(reinterpret_cast<char *>(&header_), sizeof(header_));
    file.read(reinterpret_cast<char *>(&info_), sizeof(info_));

    if (header_.signature[0] != 'B' || header_.signature[1] != 'M') {
        throw FileReadErrorException("input file is not a string");
    }
    if (info_.info_size != sizeof(BmpInfo)) {
        throw FileReadErrorException("invalid BMP format");
    }
    const char needed_bit_count = 24;
    if (info_.bit_count != needed_bit_count) {
        throw FileReadErrorException("only 24-bit BMPs are supported");
    }
    if (info_.compression != 0) {
        throw FileReadErrorException("compressed bmp are not supported");
    }
    if (info_.colors_used != 0 || info_.colors_important != 0) {
        throw FileReadErrorException("BMPs with color tables are not supported");
    }
}
void BMP::ReadPixels(std::ifstream &file, char *file_name) {
    int32_t width = info_.width;
    int32_t height = info_.height;
    for (int32_t y = height - 1; y >= 0; y--) {
        for (int32_t x = 0; x < width; x++) {
            uint8_t b = 0;
            uint8_t g = 0;
            uint8_t r = 0;
            file.read(reinterpret_cast<char *>(&b), sizeof(b));
            file.read(reinterpret_cast<char *>(&g), sizeof(g));
            file.read(reinterpret_cast<char *>(&r), sizeof(r));
            matrix_(y, x) = RGB{static_cast<double>(r), static_cast<double>(g), static_cast<double>(b)};
        }

        uint8_t padding = 0;
        for (uint32_t i = 0; i < (4 - ((width * sizeof(char) * 3) % 4)) % 4; i++) {
            file.read(reinterpret_cast<char *>(&padding), sizeof(padding));
        }
    }
}

const char *FileReadErrorException::what() const noexcept {
    return message_.c_str();
}
FileReadErrorException::FileReadErrorException(const std::string &str) {
    message_ = "FileReadException:" + str;
}
