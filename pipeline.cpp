//
// Created by supernyan on 22.03.23.
//

#include "pipeline.h"
void Pipeline::Append(Filter* filter) {
    filters_.push_back(filter);
}
void Pipeline::Apply(BMP& bmp) {
    for (Filter* filter : filters_) {
        if (filter != nullptr) {
            filter->ApplyFilter(bmp);
        }
    }
}
Pipeline::~Pipeline() {
    for (Filter* filter : filters_) {
        delete filter;
    }
}
