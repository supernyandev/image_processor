
#ifndef CPP_HSE_PIPELINE_H
#define CPP_HSE_PIPELINE_H
#include "filters.h"
#include <vector>
#include "bmp.h"
class Pipeline {
public:
    void Append(Filter* filter);
    void Pop();
    void Apply(BMP& bmp);
    ~Pipeline();

protected:
    std::vector<Filter*> filters_;
};
#endif  // CPP_HSE_PIPELINE_H
