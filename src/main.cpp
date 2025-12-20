#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "Testing.h"
#include "BinpackConstructionHeuristic.h"
#include "FFN.h"
#include "GeneratorOdp.h"
#include "GeneratorStd.h"
#include "BinDrawer.h"
#include <filesystem>
#include "options.h"
#include "io.h"



using namespace std;







class CmdLineArgs {
public:
    explicit CmdLineArgs(const std::string& commandLine) {
        std::stringstream ss(commandLine);
        std::string segment;

        // 1. Split string by spaces into a vector of strings
        std::vector<std::string> args;
        while (ss >> segment) {
            args.push_back(segment);
        }

        // 2. Update argc
        _argc = static_cast<int>(args.size());

        // 3. Allocate memory for argv (array of pointers)
        // +1 for the terminating nullptr (standard convention)
        _argv = new char*[_argc + 1];

        // 4. Copy strings into mutable char buffers
        for (int i = 0; i < _argc; ++i) {
            _argv[i] = new char[args[i].size() + 1];
            std::strcpy(_argv[i], args[i].c_str());
        }
        _argv[_argc] = nullptr; // Null terminate the array
    }

    // Destructor to clean up memory automatically
    ~CmdLineArgs() {
        if (_argv) {
            for (int i = 0; i < _argc; ++i) {
                delete[] _argv[i]; // Delete each string
            }
            delete[] _argv; // Delete the array of pointers
        }
    }

    // Accessors
    int argc() const { return _argc; }
    char** argv() const { return _argv; }

private:
    int _argc;
    char** _argv;
};




std::string cmd1 = "nd-ch-sp --generate_logistic --output_file=test/gen_file.txt --strip_width=1200 --T=10 --dt_min=1 --dt_max=10 --dim=small --set_size=50000 --seed=1";
std::string cmd2 = "nd-ch-sp --generate_logistic --output_file=test/test_file_10.txt --strip_width=1200 --T=10 --dt_min=1 --dt_max=10 --dim=small --set_size=10 --seed=1";
std::string cmd3 = "nd-ch-sp --train --output_dir=train_out_dir --input_file=test/gen_file.txt --val_set_size=10000 --layer1=32 --layer2=16 --batch_size=100 --population=192 --max_evals=500000 --sigma=0.4 --seed=1";
std::string cmd4 = "nd-ch-sp --test --output_dir=test_out_dir --input_file=test/test_file_10.txt --training_output_dir=train_out_dir --time_limit=60 --population=192 --max_evals=50000000 --sigma=0.4 --seed=1 --solution --graphics";


// artykuł - na konferencję - strip packing i douczanie
int main(int argc, char** argv) {


    // CmdLineArgs parser(cmd4);
    // AppConfig Cfg;
    //
    // if (!parse_command_line(parser.argc(), parser.argv(), Cfg)) return 1;

    // return 0;
    AppConfig Cfg;
    if (!parse_command_line(argc, argv, Cfg)) {
        return 1;
    }



    // Using the overloaded << operator
    std::cout << Cfg << std::endl;

    if (Cfg.mode == AppConfig::mTrain) {

        binpack::train(Cfg);

    } else if (Cfg.mode == AppConfig::mTest) {

        binpack::test(Cfg);

    } else if (Cfg.mode == AppConfig::mGenerateLogistic) {

        binpack::generateLogistic(Cfg);

    } else {
        ERROR( "Mode unknown.")
    }
}

