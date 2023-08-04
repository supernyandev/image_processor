
#include "application.h"
#include "bmp.h"
#include <iostream>
std::vector<FilterDescriptor> Application::PickDescriptions(int argc, char** argv) {
    std::vector<FilterDescriptor> ans;
    if (argc < 3) {
        throw CommandLineException("not enough command line parameters");
    }
    file_name_ = argv[1];
    output_file_name_ = argv[2];
    FilterDescriptor curr;
    bool is_filter = false;
    for (int i = 3; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (is_filter) {
                ans.push_back(curr);
                curr = FilterDescriptor();
            }
            is_filter = true;
            curr.name = argv[i] + 1;
        } else {
            if (is_filter) {
                curr.arguments.push_back(argv[i]);
            }
        }
    }
    if (is_filter) {
        ans.push_back(curr);
    }

    return ans;
}

Application::~Application() {
}
void Application::Run() {
    if (is_reference) {
        return;
    }
    pipeline_.Apply(bmp_);
    bmp_.Write(output_file_name_);
}
Application::Application(int argc, char** argv) {
    if (argc == 1) {
        is_reference = true;
        std::cerr << "Available filters:-crop {height} {width}; -gs (grayscale}; -sharp (sharpening); -neg (negative); "
                     "-edge {threshold} (edge detection); -rainbow";
        return;
    }
    std::vector<FilterDescriptor> descriptors = PickDescriptions(argc, argv);

    FillCreators();
    bmp_.Read(file_name_);

    for (const auto& descriptor : descriptors) {
        pipeline_.Append(CreateFilter(descriptor));
    }
}

void Application::FillCreators() {
    filter_creators_.insert({static_cast<std::string_view>("crop"), &filter_creators::CreateCrop});
    filter_creators_.insert({static_cast<std::string_view>("gs"), &filter_creators::CreateGray});
    filter_creators_.insert({static_cast<std::string_view>("neg"), &filter_creators::CreateNeg});
    filter_creators_.insert({static_cast<std::string_view>("sharp"), &filter_creators::CreateSharp});
    filter_creators_.insert({static_cast<std::string_view>("edge"), &filter_creators::CreateEdge});
    filter_creators_.insert({static_cast<std::string_view>("blur"), &filter_creators::CreateGauss});
    filter_creators_.insert({static_cast<std::string_view>("rainbow"), &filter_creators::CreateRainbow});
}
Filter* Application::CreateFilter(const FilterDescriptor& params) {
    return filter_creators_[params.name](params);
}
CommandLineException::CommandLineException(const std::string& str) {
    message_ = "CommandLineException: " + str;
}
const char* CommandLineException::what() const noexcept {
    return message_.c_str();
}
