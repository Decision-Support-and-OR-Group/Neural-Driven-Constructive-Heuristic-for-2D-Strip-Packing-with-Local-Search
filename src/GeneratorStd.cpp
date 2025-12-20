//
// Created by tsliwins on 21.12.24.
//

#include <set>
#include <string>
#include "utils.h"
#include "GeneratorStd.h"
#include "Err.h"


namespace binpack {
    using namespace std;


    void GeneratorStd::load(int _seed, int numTotal, int numValid, vector<BinpackData>& TotalData, vector<BinpackData>& ValidationData ) {

        if (_seed >= 0) {
            GConf.seed = _seed;
        }

        mt19937 gen(GConf.seed);

        int validationDataSize = numValid;
        int totalDataSize = numTotal;

        ValidationData.resize(validationDataSize);

        for( int ii=0; auto &Data: ValidationData ) {
            if (GConf.numLargeBoxes == 0) {
                generateRange(gen, Data,ii);
            } else {
                generateBins(gen, Data, ii);
            }
            ii++;
        }

        TotalData.clear();

        int u = 0, pop = -1;
        BinpackData IOD;
        if (GConf.makeDistinct) {
            for( int ii = 0; ii < totalDataSize; ii++ ) {
                bool found = true;
                for ( int g = 0; g < 10 && found; g++ ) {
                    found = false;
                    if (GConf.numLargeBoxes == 0) {
                        generateRange(gen, IOD,ii);
                    } else {
                        generateBins(gen, IOD, ii);
                    }
                    for( auto &VD: ValidationData ) {
                        if (VD == IOD) {
                            found = true;
                            break;
                        }
                    }
                }

                if (u++ % 1000 == 0) {
                    cout <<   "." <<  flush;
                }
                TotalData.push_back( IOD );
            }
        } else {
            for( int ii = 0; ii < totalDataSize; ii++ ) {
                if (GConf.numLargeBoxes == 0) {
                    generateRange(gen, IOD,ii);
                } else {
                    generateBins(gen, IOD, ii);
                }
                TotalData.push_back( IOD );
            }
        }

        // if (TotalData.size() < ValidationData.size()) {
        //     INTERNAL("DataLoaderStd::load: TotalData.size() < ValidationData.size()");
        // }
    }

    void GeneratorStd::generateRange(mt19937 &gen, BinpackData &_IOD, int id ) {
        BinpackData IOD;

        vector<pair<int, int>> MixedDims = SmallDims;
        MixedDims.insert(MixedDims.end(), MedDims.begin(), MedDims.end());
        MixedDims.insert(MixedDims.end(), LargeDims.begin(), LargeDims.end());


        const vector<pair<int, int>> *Dims;

        switch (GConf.sc) {
            case scSmall: Dims = &SmallDims; break;
            case scMedium: Dims = &MedDims; break;
            case scLarge: Dims = &LargeDims; break;
            case scMixed: Dims = &MixedDims; break;
            default:
                INTERNAL("DataLoaderStd::generate: Unsupported sc ");
        }

        if (GConf.numOfBoxTypes > Dims->size()) {
            INTERNAL("DataLoaderStd::generate: too many item types");
        }

        set<pair<int, int>> ItemTypesSet;

        while ( ItemTypesSet.size() < GConf.numOfBoxTypes) {
            ItemTypesSet.insert( (*Dims)[uniform_int_distribution<>(0, Dims->size()-1)(gen)] );
        }


        vector<BinpackData::BoxType> BoxTypes;
        int idx = 0;
        for ( auto IT: ItemTypesSet) {
            BinpackData::BoxType BT;
            BT.idx = idx;
            BT.SizeX = IT.first;
            BT.SizeY = IT.second;
            idx++;
            BoxTypes.push_back(BT);
        }

        IOD.PSizeX = GConf.PSizeX;
        IOD.PSizeY = GConf.PSizeY;
        // IOD.fileName = "std_strip_" + to_string( n, 2 ) + "_" + to_string( DemandRange.first, 2 ) + "-" + to_string( DemandRange.second, 2 )
        // + "_" + ClassNames[sc] + "_" + to_string(id, 3);
        IOD.name = GConf.nameBase + "_" + nnutils::to_string(id, 6);

        IOD.BoxTypes = BoxTypes;

        std::uniform_int_distribution<> D(GConf.DemandRange.first, GConf.DemandRange.second);

        for( int i = 0; i < BoxTypes.size(); i++ ) {
            IOD.BoxToLoad.push_back( D(gen) );
        }

        _IOD = IOD;
    }

    void GeneratorStd::generateBins(mt19937 &gen, BinpackData &_IOD, int id ) {

        vector<pair<int, int>> MixedDims = SmallDims;
        MixedDims.insert(MixedDims.end(), MedDims.begin(), MedDims.end());
        MixedDims.insert(MixedDims.end(), LargeDims.begin(), LargeDims.end());


        const vector<pair<int, int>> *Dims;

        switch (GConf.sc) {
            case scSmall: Dims = &SmallDims; break;
            case scMedium: Dims = &MedDims; break;
            case scLarge: Dims = &LargeDims; break;
            case scMixed: Dims = &MixedDims; break;
            default:
                INTERNAL("DataLoaderStd::generate: Unsupported sc ");
        }

        if (GConf.numOfBoxTypes > Dims->size()) {
            INTERNAL("DataLoaderStd::generate: too many item types");
        }

        set<pair<int, int>> ItemTypesSet;

        while ( ItemTypesSet.size() < GConf.numOfBoxTypes) {
            ItemTypesSet.insert( (*Dims)[uniform_int_distribution<>(0, Dims->size()-1)(gen)] );
        }




        vector<BinpackData::BoxType> BoxTypes;
        int idx = 0;
        for ( auto IT: ItemTypesSet) {
            BinpackData::BoxType BT;
            BT.idx = idx;
            BT.SizeX = IT.first;
            BT.SizeY = IT.second;
            idx++;
            BoxTypes.push_back(BT);
        }

        vector<int> BoxDiscr;

        std::uniform_int_distribution<> D(GConf.DemandRange.first, GConf.DemandRange.second);

        for( int i = 0; i < BoxTypes.size(); i++ ) {
            BoxDiscr.push_back( D(gen) );
        }


        BinpackData IOD;
        IOD.PSizeX = GConf.PSizeX;
        IOD.PSizeY = GConf.PSizeY;
        IOD.name = GConf.nameBase + "_" + nnutils::to_string(id, 6);
        IOD.BoxTypes = BoxTypes;
        IOD.BoxToLoad.assign(BoxTypes.size(), 0);

        std::discrete_distribution<> Discr(BoxDiscr.begin(), BoxDiscr.end());
        int vol = 0;
        do {
            auto &BT = IOD.BoxTypes[Discr(gen)];
            vol += BT.SizeX*BT.SizeY;
            if (vol > GConf.numLargeBoxes * GConf.PSizeX * GConf.PSizeY) {
            // if (vol > (numLargeBoxes-1) * PSizeX * PSizeY + 0.9 * PSizeX * PSizeY) {
                break;
            }
            IOD.BoxToLoad[BT.idx] += 1;
        } while (true);

        _IOD = IOD;
    }

    const vector<string> GeneratorStd::ClassNames = {"large", "medium", "small", "mixed"};

    const vector<pair<int, int>> GeneratorStd::LargeDims = {
        {266,	109},
        {266,	120},
        {266,	133},
        {266,	150},
        {266,	171},
        {266,	200},
        {266,	240},
        {300,	133},
        {300,	160},
        {300,	200},
        {300,	240},
        {300,	266},
        {400,	160},
        {400,	171},
        {400,	200},
        {400,	240},
        {400,	266},
        {400,	300},
        {400,	400},
        {600,	266},
        {600,	400}
    };

    const vector<pair<int, int>> GeneratorStd::MedDims = {
        {171,   100},
        {171,	114},
        {171,	133},
        {171,	160},
        {200,	100},
        {200,	109},
        {200,	114},
        {200,	120},
        {200,	133},
        {200,	150},
        {200,	160},
        {200,	171},
        {200,	200},
        {240,	100},
        {240,	114},
        {240,	133},
        {240,	160},
        {240,	200},
        {250,	200}
    };

    const vector<pair<int, int>> GeneratorStd::SmallDims = {
        {100,	100},
        {109,	100},
        {114,	100},
        {114,	109},
        {120,	100},
        {120,	114},
        {133,	100},
        {133,	109},
        {133,	114},
        {133,	120},
        {133,	133},
        {150,	100},
        {150,	114},
        {150,	133},
        {160,	100},
        {160,	109},
        {160,	120},
        {160,	133},
        {160,	150}
    };
}
















