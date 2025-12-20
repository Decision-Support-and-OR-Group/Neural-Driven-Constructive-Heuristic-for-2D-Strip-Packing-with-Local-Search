#include <fstream>
#include <sstream>
#include <filesystem>
#include "GeneratorOdp.h"
#include "Err.h"
#include "utils.h"

namespace binpack {

    void GeneratorOdp::load(int _start, int _size, vector<BinpackData> &IODs) {

        IODs.clear();

        int numIODs;
        int numLargeObjects;
        int W, V;
        int n;

        IODs.clear();


        const std::filesystem::path FSP{GConf.fileName }; // + ".txt"};

        filesystem::directory_entry dir_entry(FSP);

        ifstream ifs(GConf.fileName);

        std::string filename = FSP.filename().string();
        std::string stem = FSP.stem().string();

        if (!ifs.is_open()) {
            INTERNAL("cannot find " + GConf.fileName ); // + ".txt" );
        }

        std::string line;

        int idx = 0;
        int numLines = (GConf.odp ? 10 : 9);
        while (std::getline(ifs, line))
        {
            idx++;
            if (idx == numLines) break;
        }

        ifs >> numIODs;
        for ( int j = 0; j < numIODs; j++ ) { // start; j < min( start + size, numIODs); j++ ) {
            BinpackData IOD;
            IOD.name = stem + "_" + nnutils::to_string(j, 6);
            if (GConf.odp) {
                ifs >> numLargeObjects;
            }
            ifs >> W >> V;
            if (GConf.odp) {
                IOD.PSizeX = INT_MAX/2;
                IOD.PSizeY = W;
            } else {
                IOD.PSizeX = W;
                IOD.PSizeY = V;
            }
            ifs >> n;
            for ( int i = 0; i < n; i++ ) {
                int w, h;
                int box_num;
                ifs >> w >> h >> box_num;
                if (h < w) swap(h, w);
                IOD.BoxTypes.emplace_back(i, w, h);
                IOD.BoxToLoad.push_back(box_num);
            }
            if (j >= _start && j < _start + _size) {
                IODs.push_back(IOD);
            }
        }
    };
}