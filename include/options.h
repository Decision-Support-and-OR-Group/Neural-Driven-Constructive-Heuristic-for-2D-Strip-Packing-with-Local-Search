#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include "GeneratorStd.h"

namespace po = boost::program_options;

struct AppConfig {
    // Shared and Mode Flags
    enum AppConfigMode { mGenerateLogistic = 0, mTrain = 1, mTest = 2 };
    AppConfigMode mode;
    std::string strMode;
    std::string output_dir;
    std::string output_file;
    std::string input_file;
    int seed = 1;

    // Generation specific
    int T;
    int dt_min, dt_max;
    std::string dim;
    int strip_width;
    binpack::GeneratorStd::SetClass dimClass;
    long set_size;

    // Training/Testing specific
    long val_set_size = 10000;
    std::string training_output_dir;
    int layer1 = 32, layer2 = 16;
    int batch_size = 100;
    int population = 192;
    int max_evals = 500000;
    float sigma = 0.4f;
    int time_limit = 60;
    bool solution = false;
    bool graphics = false;
};


std::ostream& operator<<(std::ostream& os, const AppConfig& c);
bool parse_command_line(int argc, char* argv[], AppConfig& opts) ;
