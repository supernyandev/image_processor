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
    buffer_matrix_ = TMatrix<RGB>(height, width);
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
const RGB &BMP::operator()(int64_t height, int64_t width) {
    if (height < 0) {
        height = 0;
    }
    if (width < 0) {
        width = 0;
    }
    if (height >= static_cast<int64_t>(height_)) {
        height = height_ - 1;
    }
    if (width >= static_cast<int64_t>(width_)) {
        width = width_ - 1;
    }
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
void BMP::ChangePixel(RGB color, int64_t height, int64_t width) {
    if (height < 0) {
        height = 0;
    }
    if (width < 0) {
        width = 0;
    }
    if (height >= static_cast<int64_t>(height_)) {
        height = height_ - 1;
    }
    if (width >= static_cast<int64_t>(width_)) {
        width = width_ - 1;
    }
    buffer_matrix_(height, width) = color;
}
void BMP::ApplyChanges() {
    matrix_ = std::move(buffer_matrix_);
    buffer_matrix_ = matrix_;
}
void BMP::Divide(size_t threads_count) {
    threads_count = std::min(threads_count, width_);
    if (!WaitBarrier) {
        delete WaitBarrier;
    }
    WaitBarrier = new std::barrier(threads_count);
    WindowCount_ = threads_count;
    windows.clear();
    for (size_t x = 0; x < width_; x += width_ / threads_count) {
        if (x + width_ / threads_count * 2 - 1 >= width_) {
            windows.push_back({*this, x, 0, width_ - x, height_});
            continue ;
        }
        windows.push_back({*this, x, 0, width_ / threads_count, height_});
    }
}
BMP::~BMP() {
    if (!WaitBarrier) {
        delete WaitBarrier;
    }
}

const char *FileReadErrorException::what() const noexcept {
    return message_.c_str();
}
FileReadErrorException::FileReadErrorException(const std::string &str) {
    message_ = "FileReadException:" + str;
}
BMPWindow::BMPWindow(BMP &parent, size_t x, size_t y, size_t width, size_t height) : parent_(parent), y_(y), x_(x) {
    BMP::height_ = height;
    BMP::width_ = width;
}
void BMPWindow::Resize(size_t new_height, size_t new_width) {
    throw new std::logic_error("BMPWindow cannot be resized");
}
void BMPWindow::ApplyChanges() {
    parent_.ApplyCounter_.fetch_add(1, std::memory_order_seq_cst);
    if (parent_.ApplyCounter_ == parent_.WindowCount_) {

        parent_.ApplyMutex_.lock();

        if (parent_.ApplyCounter_ != 0) {
            parent_.ApplyChanges();
        }
        parent_.ApplyCounter_ = 0;
        parent_.ApplyMutex_.unlock();
    }

    parent_.WaitBarrier->arrive_and_wait();

}
const RGB &BMPWindow::operator()(int64_t height, int64_t width) {
    return parent_(y_ + height, x_ + width);
}
void BMPWindow::ChangePixel(RGB color, int64_t height, int64_t width) {
    parent_.ChangePixel(color, y_ + height, x_ + width);
}
BMPWindow::BMPWindow(BMPWindow &other) : parent_(other.parent_) {
    x_ = other.x_;
    y_ = other.y_;
    height_ = other.height_;
    width_ = other.width_;
    WindowCount_ = parent_.WindowCount_;
}
BMPWindow::BMPWindow(BMPWindow &&other) : parent_(other.parent_) {
    x_ = other.x_;
    y_ = other.y_;
    height_ = other.height_;
    width_ = other.width_;
}