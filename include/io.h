#pragma once
#include <map>
#include <vector>
#include <string>
#include "io.h"
#include "options.h"

namespace binpack {
    using namespace std;

    void train( AppConfig Cfg );
    void test( AppConfig Cfg );
    void generateLogistic( AppConfig Cfg );

}