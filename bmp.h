
#ifndef CPP_HSE_BMP_H
#define CPP_HSE_BMP_H
#include "matrix.h"
#include <string>
struct RGB;
class FileReadErrorException : public std::exception {
    std::string message_;

public:
    explicit FileReadErrorException(const std::string& str);
    const char* what() const noexcept override;
};

class BMP {
public:
    const static inline unsigned char RGB_LIMIT = 255;
    static const inline double DRGB_LIMIT = 255.0;
    BMP();
    BMP(const BMP& bmp) = delete;
    void Read(char* file_name);
    void Write(char* file_name);
    std::pair<size_t, size_t> GetSize();
    void Resize(size_t new_height, size_t new_width);
    RGB& operator()(size_t height, size_t width);
    TMatrix<RGB>& GetMatrix();

protected:
    void ReadHeader(std::ifstream& file, char* file_name);
    void ReadPixels(std::ifstream& file, char* file_name);
    struct __attribute__((packed)) BmpHeader {
        char signature[2];
        uint32_t file_size;
        uint32_t reserved;
        uint32_t data_offset;
    };
    struct __attribute__((packed)) BmpInfo {
        uint32_t info_size;
        int32_t width;
        int32_t height;
        uint16_t planes;
        uint16_t bit_count;
        uint32_t compression;
        uint32_t image_size;
        int32_t x_pixels_per_meter;
        int32_t y_pixels_per_meter;
        uint32_t colors_used;
        uint32_t colors_important;
    };
    size_t height_;
    size_t width_;

    BmpHeader header_;
    BmpInfo info_;
    TMatrix<RGB> matrix_;
};

struct RGB {
    // даблы чтобы в фильтрах не накапливалась ошибка, буду преобразовывать при записи
    double red = 0;
    double green = 0;
    double blue = 0;
    RGB& operator+=(const RGB& other) {
        this->red += other.red;
        this->green += other.green;
        this->blue += other.blue;
        return *this;
    }
    RGB operator+(const RGB& other) const {
        RGB ans;
        ans += other;
        ans += *this;
        return ans;
    }
    RGB operator*(double num) const {
        RGB ans = *this;
        ans.red *= num;
        ans.blue *= num;
        ans.green *= num;
        return ans;
    }
    RGB& operator*=(double num) {
        this->red *= num;
        this->blue *= num;
        this->green *= num;
        return *this;
    }
    void Normalize() {
        if (red < 0) {
            red = 0;
        }
        if (red > BMP::DRGB_LIMIT) {
            red = BMP::DRGB_LIMIT;
        }
        if (green < 0) {
            green = 0;
        }
        if (green > BMP::DRGB_LIMIT) {
            green = BMP::DRGB_LIMIT;
        }
        if (blue < 0) {
            blue = 0;
        }
        if (blue > BMP::DRGB_LIMIT) {

            blue = BMP::DRGB_LIMIT;
        }
    }
};
#endif  // CPP_HSE_BMP_H
