#include "filters.h"
#include <math.h>
Filter::~Filter() {
}
bool Crop::ApplyFilter(BMP& bmp) {
    bmp.Resize(new_height_, new_width_);
    return true;
}
Crop::Crop(uint32_t new_width, uint32_t new_height) : new_width_{new_width}, new_height_{new_height} {
}
Filter* filter_creators::CreateCrop(const FilterDescriptor& params) {
    if (params.arguments.size() != 1) {
        throw FilterException("invalid number of arguments");
    }
    return new Crop(atoi(params.arguments[0]), atoi(params.arguments[1]));
}
Filter* filter_creators::CreateNeg(const FilterDescriptor& params) {
    if (!params.arguments.empty()) {
        throw FilterException("invalid number of arguments");
    }
    return new Negative();
}
Filter* filter_creators::CreateGray(const FilterDescriptor& params) {
    if (!params.arguments.empty()) {
        throw FilterException("invalid number of arguments");
    }
    return new Grayscale();
}
Filter* filter_creators::CreateSharp(const FilterDescriptor& params) {
    if (!params.arguments.empty()) {
        throw FilterException("invalid number of arguments");
    }
    return new Sharpening();
}
Filter* filter_creators::CreateEdge(const FilterDescriptor& params) {
    if (params.arguments.size() != 1) {
        throw FilterException("invalid number of arguments");
    }
    return new EdgeDetection(atof(params.arguments[0]));
}
Filter* filter_creators::CreateGauss(const FilterDescriptor& params) {
    return nullptr;
}
Filter* filter_creators::CreateRainbow(const FilterDescriptor& params) {
    if (!params.arguments.empty()) {
        throw FilterException("invalid number of arguments");
    }
    return new Rainbow();
}
bool Negative::ApplyFilter(BMP& bmp) {
    for (size_t i = 0; i < bmp.GetSize().second; ++i) {
        for (size_t j = 0; j < bmp.GetSize().first; ++j) {
            bmp.ChangePixel({BMP::DRGB_LIMIT - bmp(i, j).red, BMP::DRGB_LIMIT - bmp(i, j).green,
                         BMP::DRGB_LIMIT - bmp(i, j).blue},i,j);
        }
    }
    bmp.ApplyChanges();
    return true;
}
bool Grayscale::ApplyFilter(BMP& bmp) {
    const double gs_red = 0.299;
    const double gs_blue = 0.114;
    const double gs_green = 0.587;
    for (size_t i = 0; i < bmp.GetSize().second; ++i) {
        for (size_t j = 0; j < bmp.GetSize().first; ++j) {

            double color = gs_red * bmp(i, j).red + gs_green * bmp(i, j).green + gs_blue * bmp(i, j).blue;

            RGB cl = {color,color,color};
            cl.Normalize();
            bmp.ChangePixel(cl ,i,j);
        }
    }
    bmp.ApplyChanges();
    return true;
}
void MatrixFilter::ApplyMatrix(BMP& bmp) {

    for (int i = 0; i < static_cast<int64_t>(bmp.GetSize().second); ++i) {
        for (int j = 0; j < static_cast<int64_t>(bmp.GetSize().first); ++j) {
            RGB final;
            for (int i_sum = 0; i_sum < static_cast<int>(GetSum().GetCollsNum()); ++i_sum) {
                for (int j_sum = 0; j_sum < static_cast<int64_t>(GetSum().GetRowsNum()); ++j_sum) {
                    int64_t new_i = i + i_sum - 1;
                    int64_t new_j = j + j_sum - 1;

                    RGB add = bmp(new_i, new_j) * GetSum()(i_sum, j_sum);
                    final += add;
                }
            }
            final.Normalize();
            bmp.ChangePixel(final,i, j);
        }
    }
    bmp.ApplyChanges();
}
bool Sharpening::ApplyFilter(BMP& bmp) {
    ApplyMatrix(bmp);
    return true;
}
const TMatrix<double>& Sharpening::GetSum() {
    return WEIGHTED_SUM;
}
bool EdgeDetection::ApplyFilter(BMP& bmp) {
    Grayscale gs;
    gs.ApplyFilter(bmp);
    ApplyMatrix(bmp);
    for (int64_t i = 0; i < static_cast<int64_t>(bmp.GetSize().second); ++i) {
        for (int64_t j = 0; j < static_cast<int64_t>(bmp.GetSize().first); ++j) {
            if (bmp(i, j).red / static_cast<double>(BMP::RGB_LIMIT) <= threshold_) {
                bmp.ChangePixel({},i,j);
            } else {
                bmp.ChangePixel( {BMP::RGB_LIMIT, BMP::RGB_LIMIT, BMP::RGB_LIMIT},i,j);
            }
        }
    }
    bmp.ApplyChanges();
    return true;
}
const TMatrix<double>& EdgeDetection::GetSum() {
    return WEIGHTED_SUM;
}
EdgeDetection::EdgeDetection(double threshold) {
    threshold_ = threshold;
}
FilterException::FilterException(const std::string& str) {
    message_ = "FilterException: " + str;
}
const char* FilterException::what() const noexcept {
    return message_.c_str();
}
double Rainbow::Gaussian(double x, double sigma, double b) {
    return 1 / (std::sqrt(2 * M_PI * sigma * sigma)) * std::exp(-(x - b) * (x - b) / (2 * sigma * sigma));
}
bool Rainbow::ApplyFilter(BMP& bmp) {
    const double b1 = 0.0;
    const double b2 = 0.4;
    const double b3 = 0.7;
    const double b4 = 1;
    const double sigma_red = 0.2;
    const double sigma_blue = 0.3;
    const double sigma = 0.4;
    const double damp = 2.0;
    const double damp_green = 1.2;
    const double intencivity = 0.6;
    for (int64_t i = 0; i < static_cast<int64_t>(bmp.GetSize().second); ++i) {
        for (int64_t j = 0; j < static_cast<int64_t>(bmp.GetSize().first); ++j) {
            double x = static_cast<double>(i) / static_cast<double>(bmp.GetSize().second);
            RGB c = bmp(i, j)+ RGB{(BMP::DRGB_LIMIT * Gaussian(x, sigma_red, b1) / damp +
                                       BMP::DRGB_LIMIT * Gaussian(x, sigma, b4) / damp) *
                                          intencivity,
                                      (BMP::DRGB_LIMIT * Gaussian(x, sigma, b2) / damp_green) * intencivity,
                                      (BMP::DRGB_LIMIT * Gaussian(x, sigma_blue, b3) / damp) * intencivity};
            c.Normalize();
            bmp.ChangePixel(c,i,j);
        }
        bmp.ApplyChanges();
    }
    return true;
}
