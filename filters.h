#include "matrix.h"
#include <vector>
#include "bmp.h"
#ifndef CPP_HSE_FITERS_H
#define CPP_HSE_FITERS_H
class FilterException : public std::exception {
    std::string message_;

public:
    explicit FilterException(const std::string& str);
    const char* what() const noexcept override;
};
class Filter {
public:
    uint16_t threads = 1;
    virtual bool ApplyFilter(BMP& bmp) = 0;

    virtual ~Filter();

};
struct FilterDescriptor {
    char* name;
    std::vector<char*> arguments;
    unsigned short threads_num = 1;
};
class Crop : public Filter {
public:
    bool ApplyFilter(BMP& bmp) override;
    Crop(uint32_t new_width, uint32_t new_height);

protected:
    uint32_t new_width_;
    uint32_t new_height_;
};
class Grayscale : public Filter {
public:
    bool ApplyFilter(BMP& bmp) override;
};
class Negative : public Filter {
public:
    bool ApplyFilter(BMP& bmp) override;
};
class MatrixFilter : public Filter {
public:
    bool ApplyFilter(BMP& bmp) override = 0;

protected:
    void ApplyMatrix(BMP& bmp);
    virtual const TMatrix<double>& GetSum() = 0;
};

class Sharpening : public MatrixFilter {
public:
    bool ApplyFilter(BMP& bmp) override;

protected:
    static const inline TMatrix<double> WEIGHTED_SUM{{{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}}};
    const TMatrix<double>& GetSum() override;
};
class EdgeDetection : public MatrixFilter {
public:
    explicit EdgeDetection(double threshold);
    bool ApplyFilter(BMP& bmp) override;

protected:
    double threshold_;
    static const inline TMatrix<double> WEIGHTED_SUM{{{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}}};
    const TMatrix<double>& GetSum() override;
};
class GaussianBlur : public MatrixFilter {
public:
    explicit GaussianBlur(double sigma);
    bool ApplyFilter(BMP& bmp) override;
protected:
    double sigma_;
    double GetCoefficient(long double x,long double x0,long double y,long double y0);
    TMatrix<long double> Normalize(TMatrix<long double> m);
    TMatrix<double> Weighted_Sum_;
    const TMatrix<double>& GetSum() override;
};
class Rainbow : public Filter {
public:
    bool ApplyFilter(BMP& bmp) override;

protected:
    double Gaussian(double x, double sigma, double b);
};
namespace filter_creators {
Filter* CreateCrop(const FilterDescriptor& params);
Filter* CreateNeg(const FilterDescriptor& params);
Filter* CreateGray(const FilterDescriptor& params);
Filter* CreateNeg(const FilterDescriptor& params);
Filter* CreateSharp(const FilterDescriptor& params);
Filter* CreateEdge(const FilterDescriptor& params);
Filter* CreateGauss(const FilterDescriptor& params);
Filter* CreateRainbow(const FilterDescriptor& params);
}  // namespace filter_creators
#endif  // CPP_HSE_FITERS_H
