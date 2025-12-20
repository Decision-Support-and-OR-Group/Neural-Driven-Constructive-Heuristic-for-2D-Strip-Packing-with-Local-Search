//
// Created by tsliwins on 24.11.2025.
//
#include "io.h"
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <limits.h>
#include "chof.h"
#include "options.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

#include "BinDrawer.h"
#include "utils.h"
#include "FFN.h"

#include "GeneratorStd.h"

#include "Testing.h"
#include "BinpackConstructionHeuristic.h"
#include "FFN.h"
#include "GeneratorOdp.h"


namespace binpack {
    namespace fs = std::filesystem;

    void saveAsODPS( vector<binpack::BinpackData>& Data, string filename ) {
        ofstream ofs;
        ofs = nnutils::openFileWithDirs<ofstream>(filename);
        ofs << "***2D Rectangular Problem***" << endl << flush;
        ofs << "***Problem tests for the Open Dimension Problem (ODP/S)***" << endl << flush;
        ofs << "Input parameter file: " << endl << flush;
        ofs << "***********************************************************************" << endl << flush;
        ofs << "Total number of instances " << endl << flush;
        ofs << "Number of different large objects (j)" << endl << flush;
        ofs << "LargeObject[j].Width	LargeObject[j].Value" << endl << flush;
        ofs << "Number of different item types (i)" << endl << flush;
        ofs << "Item[i].Length	Item[i].Width	Item[i].Demand" << endl << flush;
        ofs << "***********************************************************************" << endl << flush;

        ofs << Data.size() << endl;

        for ( auto &D: Data ) {
            ofs << 1 << endl;
            ofs << D.PSizeX << " " << 1 << endl;
            ofs << D.BoxTypes.size() << endl;
            for ( int i = 0; i < D.BoxTypes.size(); i++ ) {
                ofs << D.BoxTypes[i].SizeX << " " << D.BoxTypes[i].SizeY << " " << D.BoxToLoad[i] << endl;
            }
        }
        ofs.close();
    }



    void generateLogistic( AppConfig Cfg ) {
        typedef binpack::GeneratorOdp GeneratorType;
        typedef GeneratorType::GenConfigType GenConfigType;
        typedef nnutils::FFN ASType;
        typedef binpack::BinpackConstructionHeuristic<ASType, GenConfigType> TCHType;

        int n;
        n = Cfg.T;
        pair<int, int> lims = make_pair(Cfg.dt_min, Cfg.dt_max);
        auto sc = Cfg.dimClass;

        binpack::GeneratorStd::GenConfigType GConfTmp;
        GConfTmp.nameBase = "logistic_data_" + to_string(n) + "_" + to_string(lims.first) + "-" + to_string(lims.second) + "_" + binpack::GeneratorStd::ClassNames[sc];
        GConfTmp.seed = Cfg.seed;
        GConfTmp.PSizeY = INT_MAX/2;
        GConfTmp.PSizeX = Cfg.strip_width;
        GConfTmp.numLargeBoxes = 0;
        GConfTmp.numOfBoxTypes = n;
        GConfTmp.DemandRange = lims;
        GConfTmp.sc = sc;
        GConfTmp.makeDistinct = true;
        string fn = Cfg.output_file;

        vector<TCHType::DataType> TotalData, ValidationData;

        binpack::GeneratorStd(GConfTmp ).load( Cfg.seed, Cfg.set_size, 0, TotalData, ValidationData );

        saveAsODPS( TotalData, fn );
    }


    void train( AppConfig Cfg ) {
        std::filesystem::current_path(".");

        typedef binpack::GeneratorOdp GeneratorType;

        typedef GeneratorType::GenConfigType GenConfigType;

        typedef nnutils::FFN ASType;
        typedef binpack::BinpackConstructionHeuristic<ASType, GenConfigType> TCHType;

        vector<BinpackData> TotalData, ValidationData;

        GenConfigType GConfTmp;

        GConfTmp.fileName = Cfg.input_file;
        GConfTmp.odp = true;

        GeneratorType Generator(GConfTmp);

        cout << "Reading problems..." << endl;
        Generator.load(0, 10000000, TotalData);
        if (Cfg.val_set_size > TotalData.size() / 2) {
            INTERNAL( "Too big validation set size in relation to all training data size");
        }

        ValidationData.assign(TotalData.end() - Cfg.val_set_size, TotalData.end());
        TotalData.erase(TotalData.end() - Cfg.val_set_size, TotalData.end());

        cout << to_string( TotalData.size() ) << " training data and " << ValidationData.size() << " validation data" << endl;

        chof::LearnConfig LConf;
        LConf.trainingDataSize = Cfg.batch_size;
        LConf.seed = 1;
        LConf.population = Cfg.population;
        LConf.NumEvals = {Cfg.max_evals};
        LConf.Sigmas = {Cfg.sigma};
        LConf.mt_feval = true;
        LConf.quiet = false;
        LConf.itersToValidate = 10;
        LConf.numValidationThreads = 24;
        LConf.storeProgressInfo = true;
        LConf.progressFile = Cfg.output_dir + "/progress.csv";



        auto &GConf = Generator.GConf;

        vector<int> Topo = {Cfg.layer1, Cfg.layer2, 1};

        ASType::ConfigType ASConf = {TCHType::PROPERTIES_SIZE, Topo};

        typename TCHType::ConfigType CHConf;

        CHConf.desc = "strip-packing";
        CHConf.stripPacking = true;
        CHConf.binPackInt = false;
        CHConf.AConf = ASConf;
        CHConf.GConf = GConf;

        string nn_fn = Cfg.output_dir + "/network.dat";

        {
            auto  ifs = ifstream(nn_fn , ios::binary);
            if (ifs.is_open()) {
                ERROR("Results file already present, remove it, exiting... (" + nn_fn + ")" );
            }
        }


        LearnConfig LConfig = LConf;

#ifndef NDEBUG
        LConfig.mt_feval = false;
#else
        LConfig.mt_feval = LConf.mt_feval;
#endif

        vector<double> ParamsOut;

        TCHType CH( CHConf);
        TCHType CHOut;

        double ret = chof::learn(LConfig,  CH, TotalData, ValidationData, CHOut);

        {
            auto ofs = nnutils::openFileWithDirs<ofstream>(nn_fn, ios::binary);
            boost::archive::binary_oarchive oa(ofs);
            oa << CHOut;
        }
    }


    void test( AppConfig Cfg ) {

        bool single = (Cfg.max_evals == 1);

        std::filesystem::current_path(".");
        // typedef jobshop::GeneratorTxt GeneratorType;
        typedef binpack::GeneratorOdp GeneratorType;

        typedef GeneratorType::GenConfigType GenConfigType;

        typedef nnutils::FFN ASType;
        typedef binpack::BinpackConstructionHeuristic<ASType, GenConfigType> TCHType;

        vector<BinpackData> TotalData;

        GenConfigType GConfTmp;

        GConfTmp.fileName = Cfg.input_file;
        GConfTmp.odp = true;

        GeneratorType Generator(GConfTmp);

        cout << "Reading test data..." << endl;
        Generator.load(0, 10000000, TotalData);
        cout << " ok." << endl << flush;

        TCHType BCH;

        {
            cout << "Reading network from " + Cfg.training_output_dir + "/network.dat";
            auto ifs = nnutils::openFileWithDirs<ifstream>(Cfg.training_output_dir + "/network.dat", ios::binary);
            boost::archive::binary_iarchive ia(ifs);
            ia >> BCH;
            cout << " ok." << endl << flush;
        }

        if (TotalData.empty()) {
            ERROR("No test problems in " + Cfg.input_file);
        }

        auto ofs_sum = nnutils::openFileWithDirs<ofstream>(Cfg.output_dir + "/summary.csv", ios::app);
        auto ofs_det = nnutils::openFileWithDirs<ofstream>(Cfg.output_dir + "/details.csv", ios::app);


        double sum = 0;
        int num = 0;

        chof::OptConfig OConf;
        OConf.seed = Cfg.seed;
        OConf.population = Cfg.population;
        OConf.NumEvals = {Cfg.max_evals};
        OConf.Sigmas = {Cfg.sigma};
        OConf.mt_feval = true;
        OConf.quiet = true;
        OConf.storeProgressInfo = false;
        OConf.progressFile = "";
        OConf.timeLimit = Cfg.time_limit;

        auto time_start = std::chrono::steady_clock::now();

        for ( auto &D: TotalData ) {

            typename TCHType::DataType::SolutionType S;

            double time_ms = 0.0;
            auto time_start = std::chrono::steady_clock::now();
            if (single) {
                S = BCH.run(D);
            } else {
                TCHType BCHOut;
                S = chof::opt(OConf,  BCH, D, BCHOut);
            }
            auto time_end = std::chrono::steady_clock::now();
            auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start).count();
            time_ms = (double)elapsed_us/1000.0;
            D.setSolution(S);

            cout << "." << flush;

            ofs_det << "eval" << "; " << Cfg.input_file << "; " << D.name << "; " << S.getObj() <<  "; " << time_ms << ";" << endl << flush;

            sum += S.getObj();
            num++;
        }

        auto time_end = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();

        double time_avg_ms = (double)elapsed_ms / TotalData.size();

        ofs_sum << "eval" << "; " << Cfg.input_file << "; ; " << sum/num << "; " << time_avg_ms << endl << flush;

        if (Cfg.graphics) {
            BinDrawer JD;
            for ( auto &D: TotalData ) {
                JD.drawToFile(D, true, Cfg.output_dir, ".png");
            }
        }

        if (Cfg.solution) {
            for ( auto &D: TotalData ) {
                D.printSolution(Cfg.output_dir + "/sol_" + D.name + ".csv");
            }

        }
    }





}
