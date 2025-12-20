//
// Created by tsliwins on 02.12.24.
//

#pragma once
#include <vector>
#include <string>
#include "BinpackData.h"
#include "utils.h"
#include <nlohmann/json.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace binpack {
    using namespace std;
    //using namespace __gnu_debug;

    struct GeneratorOdp {
        struct GenConfigType {
            friend class boost::serialization::access;
            string fileName;
            bool odp; // odp or csssp


            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & fileName;
                ar & odp;
            }
        };

        GenConfigType GConf;

        // string filename;
        // int start, size;
        // bool odp;

        // GeneratorOdp( string _filename, bool _odp, int _start = 0, int _size = -1 )
        // : filename( _filename ), start( _start ), size( _size ), odp( _odp ) {
        //
        // }

        GeneratorOdp(const GenConfigType &_GConf) : GConf(_GConf) {

        }

        //    vector<PalOpt::InputOutputData> IODs;
        void load(int _start, int _size, vector<BinpackData> &IODs);

        // static string composeFileName( string dir, string prefix, int n, pair<int, int> Range, int id ) {
        //     return dir
        //         + "/"
        //         + prefix
        //         + std::to_string(n)
        //         +  "_" + std::to_string(Range.first) + "-" + std::to_string(Range.second)
        //         + "_" + std::to_string(id);
        // }
    };


    inline void to_json(nlohmann::json& j, const GeneratorOdp::GenConfigType& p) {
        j = nlohmann::json{};
        j.emplace("fileName", p.fileName);
        j.emplace("odp", p.odp);
    }

    inline void from_json(const nlohmann::json& j, GeneratorOdp::GenConfigType& p) {
        j.at("fileName").get_to(p.fileName);
        j.at("odp").get_to(p.odp);
    }
}

