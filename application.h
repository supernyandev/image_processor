
#ifndef CPP_HSE_APPLICATION_H
#define CPP_HSE_APPLICATION_H
#include "filters.h"
#include "bmp.h"
#include <map>
#include <string_view>
#include "pipeline.h"
class CommandLineException : public std::exception {
    std::string message_;

public:
    explicit CommandLineException(const std::string& str);
    const char* what() const noexcept override;
};
class Application {
public:
    Application(int argc, char** argv);
    void Run();
    ~Application();

protected:
    using Descriptor2Filter = Filter* (*)(const FilterDescriptor&);
    std::vector<FilterDescriptor> PickDescriptions(int argc, char** argv);
    void FillCreators();
    std::map<std::string_view, Descriptor2Filter> filter_creators_;
    bool is_reference = false;
    char* file_name_;
    char* output_file_name_;
    BMP bmp_;
    Filter* CreateFilter(const FilterDescriptor& params);
    Pipeline pipeline_;
};
#endif
