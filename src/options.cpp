#include "options.h"
#include <iostream>
#include <iomanip>
#include <string>

bool parse_command_line(int argc, char* argv[], AppConfig& opts) {
    // 1. Define ALL option groups upfront so they are available for the help message
    po::options_description global("Global Options");
    global.add_options()
        ("generate_logistic", "Mode: Generate logistic data")
        ("train", "Mode: Train neural network")
        ("test", "Mode: Test/Solve problems")
        ("help,h", "Show help");

    po::options_description gen_desc("Generation Options (--generate_logistic)");
    gen_desc.add_options()
        ("output_file", po::value<std::string>(&opts.output_file)->required(), "File path to store generated training/validation or test problems")
        ("strip_width", po::value<int>(&opts.strip_width)->required(), "Strip width")
        ("T", po::value<int>(&opts.T)->required(), "Number of different item types")
        ("dt_min", po::value<int>(&opts.dt_min)->required(), "Minimum number of items of each type")
        ("dt_max", po::value<int>(&opts.dt_max)->required(), "Maximum number of items of each type")
        ("dim", po::value<std::string>(&opts.dim)->required(), "Chosen size category: small | medium | large | mixed")
        ("set_size", po::value<long>(&opts.set_size)->required(), "Number of generated problems.")
        ("seed", po::value<int>(&opts.seed)->default_value(1), "Generator's seed.");

    po::options_description train_desc("Training Options (--train)");
    train_desc.add_options()
        ("output_dir", po::value<std::string>(&opts.output_dir)->required(), "Directory to store output data including trained NN.")
        ("input_file", po::value<std::string>(&opts.input_file)->required(), "File with training/validation problems.")
        ("val_set_size", po::value<long>(&opts.val_set_size)->default_value(10000), "Number of validation problems to take from the 'file'")
        ("layer1", po::value<int>(&opts.layer1)->default_value(32), "Neural networks first hiden layer size")
        ("layer2", po::value<int>(&opts.layer2)->default_value(16), "Neural networks second hiden layer size")
        ("batch_size", po::value<int>(&opts.batch_size)->default_value(100), "Training batch size")
        ("population", po::value<int>(&opts.population)->default_value(192), "CMAES population size")
        ("max_evals", po::value<int>(&opts.max_evals)->default_value(500000), "Maximum number of black box function evaluations (number of evaluated batches)")
        ("sigma", po::value<float>(&opts.sigma)->default_value(0.4f), "CMAES sigma parameter")
        ("seed", po::value<int>(&opts.seed)->default_value(1), "CMAES initial seed");

    po::options_description test_desc("Testing Options (--test)");
    test_desc.add_options()
        ("output_dir", po::value<std::string>(&opts.output_dir)->required(), "Directory to store results")
        ("input_file", po::value<std::string>(&opts.input_file)->required(), "File with the problems to solve")
        ("training_output_dir", po::value<std::string>(&opts.training_output_dir)->required(), "Directory with saved NN weights")
        ("time_limit", po::value<int>(&opts.time_limit)->default_value(60), "Maximum testing time per problem")
        ("population", po::value<int>(&opts.population)->default_value(192), "CMAES population size")
        ("max_evals", po::value<int>(&opts.max_evals)->default_value(500000), "Maximum number of black box function evaluations")
        ("sigma", po::value<float>(&opts.sigma)->default_value(0.4f), "CMAES sigma parameter")
        ("seed", po::value<int>(&opts.seed)->default_value(1), "CMAES initial seed")
        ("solution", po::bool_switch(&opts.solution)->default_value(false), "If specified, the solution will be written to the output_dir")
        ("graphics", po::bool_switch(&opts.graphics)->default_value(false), "If specified, the solution image will be generated for each problem");

    // 2. Parse Global Options
    po::variables_map vm;
    try {
        auto parsed = po::command_line_parser(argc, argv)
            .options(global)
            .allow_unregistered() // Allow sub-options to pass through first stage
            .run();
        po::store(parsed, vm);

        // 3. Handle Help: Print ALL option groups
        if (vm.count("help") || argc <= 1) {
            std::cout << "Usage: nd-ch-sp [mode] [options]\n\n";
            std::cout << global << "\n";
            std::cout << gen_desc << "\n";
            std::cout << train_desc << "\n";
            std::cout << test_desc << std::endl;
            return false;
        }

        // 4. Determine Mode and Validation
        if (vm.count("generate_logistic") + vm.count("train") + vm.count("test") > 1) {
            std::cerr << "Error: Exactly one program mode is possible.\n";
            return false;
        }

        std::vector<std::string> opts_to_parse = po::collect_unrecognized(parsed.options, po::include_positional);
        const po::options_description* target_desc = nullptr;

        if (vm.count("generate_logistic")) {
            opts.strMode = "generate_logistic";
            opts.mode = AppConfig::mGenerateLogistic;
            target_desc = &gen_desc;
        } else if (vm.count("train")) {
            opts.strMode = "train";
            opts.mode = AppConfig::mTrain;
            target_desc = &train_desc;
        } else if (vm.count("test")) {
            opts.strMode = "test";
            opts.mode = AppConfig::mTest;
            target_desc = &test_desc;
        } else {
            std::cerr << "Error: No valid mode specified.\n";
            return false;
        }

        // 5. Parse Sub-options using the selected description
        po::store(po::command_line_parser(opts_to_parse).options(*target_desc).run(), vm);
        po::notify(vm);

        // 6. Post-parsing Validation (Dimension Class)
        if (opts.mode == AppConfig::mGenerateLogistic) {
            if (opts.dim == "small") opts.dimClass = binpack::GeneratorStd::scSmall;
            else if (opts.dim == "medium") opts.dimClass = binpack::GeneratorStd::scMedium;
            else if (opts.dim == "large") opts.dimClass = binpack::GeneratorStd::scLarge;
            else if (opts.dim == "mixed") opts.dimClass = binpack::GeneratorStd::scMixed;
            else {
                std::cerr << "Error: No valid dim class name specified. Allowed are: small, medium, large, mixed.\n";
                return false;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }

    return true;
}



std::ostream& operator<<(std::ostream& os, const AppConfig& c) {
    const int w = 25; // Label width for nice alignment
    const std::string sep = "--------------------------------------------------\n";

    os << "\n" << sep;
    os << " CONFIGURATION SUMMARY: " << c.strMode << "\n";
    os << sep;

    // Common to all modes (if seed is used everywhere)
    os << std::left << std::setw(w) << "Mode:" << c.strMode << "\n";
    os << std::left << std::setw(w) << "Seed:" << c.seed << "\n";

    switch (c.mode) {
        case AppConfig::mGenerateLogistic:
            os << std::left << std::setw(w) << "Output File:" << c.output_file << "\n";
            os << std::left << std::setw(w) << "Strip Width:" << c.strip_width << "\n";
            os << std::left << std::setw(w) << "Item Types (T):" << c.T << "\n";
            os << std::left << std::setw(w) << "dt range:" << "[" << c.dt_min << ", " << c.dt_max << "]\n";
            os << std::left << std::setw(w) << "Dimension Class:" << c.dim << "\n";
            os << std::left << std::setw(w) << "Set Size:" << c.set_size << "\n";
            break;

        case AppConfig::mTrain:
            os << std::left << std::setw(w) << "Output Directory:" << c.output_dir << "\n";
            os << std::left << std::setw(w) << "File with Training/Validation Problems:" << c.input_file << "\n";
            os << std::left << std::setw(w) << "Validation Set Size:" << c.val_set_size << "\n";
            os << std::left << std::setw(w) << "NN Architecture:" << c.layer1 << " x " << c.layer2 << "\n";
            os << std::left << std::setw(w) << "Batch Size:" << c.batch_size << "\n";
            os << std::left << std::setw(w) << "CMA-ES Population:" << c.population << "\n";
            os << std::left << std::setw(w) << "Max Evaluations:" << c.max_evals << "\n";
            os << std::left << std::setw(w) << "Sigma:" << c.sigma << "\n";
            break;

        case AppConfig::mTest:
            os << std::left << std::setw(w) << "Output Directory:" << c.output_dir << "\n";
            os << std::left << std::setw(w) << "File with Test Problems:" << c.input_file << "\n";
            os << std::left << std::setw(w) << "NN Weights Dir:" << c.training_output_dir << "\n";
            os << std::left << std::setw(w) << "Time Limit (s):" << c.time_limit << "\n";
            os << std::left << std::setw(w) << "CMA-ES Population:" << c.population << "\n";
            os << std::left << std::setw(w) << "Max Evaluations:" << c.max_evals << "\n";
            os << std::left << std::setw(w) << "Sigma:" << c.sigma << "\n";
            os << std::left << std::setw(w) << "Write Solution:" << (c.solution ? "Yes" : "No") << "\n";
            os << std::left << std::setw(w) << "Generate Graphics:" << (c.graphics ? "Yes" : "No") << "\n";
            break;
    }

    os << sep;
    return os;
}


