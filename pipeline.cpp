//
// Created by supernyan on 22.03.23.
//

#include "pipeline.h"
#include <thread>
void Pipeline::Append(Filter* filter) {
    filters_.push_back(filter);
}
void Pipeline::Apply(BMP& bmp) {
    for (Filter* filter : filters_) {
        if (filter != nullptr) {
            if (filter->threads == 1) {
                filter->ApplyFilter(bmp);
            }
            else {
                bmp.Divide(filter->threads);
                std::vector<std::thread*> threads;
                for (auto& window : bmp.windows) {
                    std::thread* th = new std::thread(&Filter::ApplyFilter, filter, std::ref(window));
                    threads.push_back(th);
                }
                for (auto& thread : threads) {
                    thread->join();
                    delete thread;
                }
            }
        }
    }

}
Pipeline::~Pipeline() {
    for (Filter* filter : filters_) {
        delete filter;
    }
}
