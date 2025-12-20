#pragma once
#include <vector>
#include <random>
#include "BinpackData.h"
#include <nlohmann/json.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace binpack {
    using namespace std;



    struct GeneratorStd {
        enum SetClass {scLarge = 0, scMedium, scSmall, scMixed};

        struct GenConfigType {
            friend class boost::serialization::access;
            string nameBase;
            int seed;
            int PSizeX = 1200;
            int PSizeY = 800;
            int numLargeBoxes;
            int numOfBoxTypes;

            pair<int,int> DemandRange;
            SetClass sc;
            bool makeDistinct;

            template<class Archive>
           void serialize(Archive & ar, const unsigned int version) {
                ar & nameBase;
                ar & seed;
                ar & PSizeX;
                ar & PSizeY;
                ar & numLargeBoxes;
                ar & numOfBoxTypes;
                ar & DemandRange;
                ar & sc;
                ar & makeDistinct;
            }
        };

        GenConfigType GConf;

    public:
        // static const int PSizeX = 1200;
        // static const int PSizeY = 800;

        // DataLoaderStd(std::mt19937 &_gen, SetClass _sc, int _numOfBoxTypes, pair<int,int> _DemandRange,
        //               int _totalDataSize, int _validationDataSize, bool _makeDistinct, string _dir, string _prefix)
        //     :   gen(_gen), sc(_sc), numLargeBoxes(1), n(_numOfBoxTypes), DemandRange(_DemandRange),
        //         totalDataSize(_totalDataSize), validationDataSize(_validationDataSize), makeDistinct(_makeDistinct),
        //         dir(_dir), prefix(_prefix) {
        // }

        // GeneratorStd(std::mt19937 &_gen, string _filename, int _numLargeBoxes, int _numOfBoxTypes, pair<int,int> _DemandRange, SetClass _sc,
        //               int _totalDataSize, int _validationDataSize, bool _makeDistinct)
        //     :   gen(_gen), filename(_filename), sc(_sc), numLargeBoxes(_numLargeBoxes), n(_numOfBoxTypes), DemandRange(_DemandRange),
        //         totalDataSize(_totalDataSize), validationDataSize(_validationDataSize), makeDistinct(_makeDistinct) {
        // }

        GeneratorStd(GenConfigType _GConf ) : GConf(_GConf) {

        }


        void load(int _seed, int numTotal, int numValid, vector<BinpackData>& TotalData, vector<BinpackData>& ValidationData );

        void generateRange( mt19937& gen, BinpackData &IOD, int id );
        void generateBins(mt19937& gen, BinpackData &IOD, int id );

        // static string composeFilename(string dir, string prefix, int n, pair<int, int> Range, SetClass sc, int numLargeBoxes = 0) {
        //     return dir
        //         + "/"
        //         + prefix
        //         + std::to_string(numLargeBoxes)
        //         + "_"
        //         + std::to_string( n)
        //         + "_" + std::to_string( Range.first) + "-" + std::to_string( Range.second )
        //         + "_" + ClassNames[sc];
        // }

    protected:
        // std::mt19937 &gen;
        // SetClass sc;
        // int numOfBoxTypes;
        // int numLargeBoxes = 0;
        // pair<int,int> DemandRange;
        // int totalDataSize;
        // int validationDataSize;
        // bool makeDistinct;
        // string filename;

        static const vector<pair<int, int>> LargeDims, MedDims, SmallDims;
    public:
        static const vector<string> ClassNames;
    };



    inline void to_json(nlohmann::json& j, const GeneratorStd::GenConfigType& p) {
        j = nlohmann::json{};
        j.emplace("nameBase", p.nameBase);
        j.emplace("seed", p.seed);
        j.emplace("PSizeX", p.PSizeX);
        j.emplace("PSizeY", p.PSizeY);
        j.emplace("numLargeBoxes", p.numLargeBoxes);
        j.emplace("numOfBoxTypes", p.numOfBoxTypes);
        j.emplace("DemandRange", p.DemandRange);
        j.emplace("sc", p.sc);
        j.emplace("makeDistinct", p.makeDistinct);
    }

    inline void from_json(const nlohmann::json& j, GeneratorStd::GenConfigType& p) {
        j.at("nameBase").get_to(p.nameBase);
        j.at("seed").get_to(p.seed);
        j.at("PSizeX").get_to(p.PSizeX);
        j.at("PSizeY").get_to(p.PSizeY);
        j.at("numLargeBoxes").get_to(p.numLargeBoxes);
        j.at("numOfBoxTypes").get_to(p.numOfBoxTypes);
        j.at("DemandRange").get_to(p.DemandRange);
        j.at("sc").get_to(p.sc);
        j.at("makeDistinct").get_to(p.makeDistinct);
    }

}
