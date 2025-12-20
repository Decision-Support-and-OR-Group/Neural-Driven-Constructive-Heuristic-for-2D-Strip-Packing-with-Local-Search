#pragma once
#include <string>
#include <cfloat>
#include <random>
#include <cassert>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <Eigen/Dense>
#include "ConstructionHeuristicConcept.h"
#include "BinpackData.h"
//#include "ReadConfig.h"
#include "CornerPoints.h"
#include "FFN.h"
#include "Err.h"
#include "utils.h"


namespace binpack {
    using namespace std;
    using namespace Eigen;


    template<typename AFType, typename GenConfType>
    class BinpackConstructionHeuristic {
        friend class boost::serialization::access;
    public:

        struct ConfigType {
            friend class boost::serialization::access;

            using AFT = AFType;
            using GenConfT = GenConfType;

            string desc;

            bool stripPacking;
            bool binPackInt = false;

            typename AFType::ConfigType AConf;
            GenConfType GConf;

            ConfigType() {

            }

            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                ar & desc;
                ar & stripPacking;
                ar & binPackInt;
                ar & AConf;
            }

        };

    public:
        // ConstructionHeuristicConcept interface
        typedef BinpackData DataType;

        bool maximize() const {
            return (Conf.stripPacking ? true : false);
        }

        int getParamsSize() const {
            int s = 0;
            s += AF.getParamsSize();
            return s;
        }

        BinpackConstructionHeuristic& setParams( const double *params, int n ) {
            AF.setParams(params, AF.getParamsSize());
            return *this;
        }

        void getParams( vector<double> &Params ) const {
            AF.getParams(Params);
        }

        DataType::SolutionType run(const DataType &Data) {
            return pack(Data);
        }
        // end of interface
    public:
        static const int PROPERTIES_SIZE = 25;
    protected:
        // fields
        ConfigType Conf;

        static const int GRID_NUM = 8;


        int binVolInserted = 0;   // item area inserted into the last bin
        int totalVolInserted = 0; // total Volume of items that were inserted so far
        int totalVolLeft = 0;      // volume of items that are left so far
        int numItems = 0; // total number of items

        //BinpackData::SolutionType Solution;


        int x_max = 0; //< how far the current bin is filled (to the right)
        EvaListType EvaList;

        AFType AF;

        CornerPoints CP;

        vector<int> View1D; //< 1D view of the current bin
        int gridSize = -1; //< distance between probing points in View1D

        vector<int> Q; //< Number of remaining boxes for each BoxType to be packed
        vector<double> BA; //< relative area of the BoxTypes in relation to the PSizeY*PSizeY

        vector<float> Properties; //< auxiliary vector with properties to be assessed

        // int currentNetwork = 0;
        int currentBin = 0;
        int PSizeX = -1;
    public:
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & Conf;
            ar & AF;
        }

    public:

        explicit BinpackConstructionHeuristic() {

        }

        explicit BinpackConstructionHeuristic( const ConfigType &_Config)
            : Conf(_Config), AF( _Config.AConf) {
            int dim = getParamsSize();
            vector<double> Params(dim, 0.0);
            setParams( Params.data(), dim );
        }

    protected:


        DataType::SolutionType pack( const DataType &IOD ) {

            DataType::SolutionType Solution;

            if (Conf.stripPacking) {
                PSizeX = INT_MAX/2;
            } else {
                PSizeX = IOD.PSizeX;
            }

            init(IOD);

            Solution.BPV.clear();
            Solution.BPV.reserve(numItems);

            while(totalVolLeft > 0) {
                // do for each bin

                initBin(IOD);

                int selectedBoxTypeIdx;

                x_max = 0;

                do {
                    // do as long as something can be inserted into bin
                    selectedBoxTypeIdx = -1;
                    double best_v = -DBL_MAX;
                    EvaListElementType best_EE, ETmp;

                    for (int i = 0; i < Q.size(); i++) {
                        if (Q[i] == 0) continue;

                        double v = evalBox(IOD, &(IOD.BoxTypes[i]), ETmp);

                        if (v > best_v) {
                            best_v = v;
                            selectedBoxTypeIdx = i;
                            best_EE = ETmp;
                        }
                    }

                    if (selectedBoxTypeIdx >= 0) {
                        auto BB = &IOD.BoxTypes[selectedBoxTypeIdx];
                        CP.Insert(BB, best_EE);

                        // modify x_max
                        auto &ip = best_EE.first;
                        //                    int y_min = ip.P1.Y;
                        int sx = BB->SizeX, sy = BB->SizeY;
                        if (ip.Rotated) swap(sx, sy);
                        int maxItemPosX = ip.P1.X + sx;
                        x_max = max(x_max, maxItemPosX); // jak daleko ulozenie siega w prawo

                        // modify View1D
                        int y_max = ip.P1.Y + sy - 1;
                        y_max = (y_max + gridSize - gridSize/2) / gridSize - 1;
                        y_max = min((int) y_max, (int) View1D.size() - 1);

                        for ( int k = 0; k <= y_max; k++) {
                            View1D[k] = max(View1D[k], maxItemPosX);
                        }

                        Q[selectedBoxTypeIdx] -= 1;

                        Solution.BPV.emplace_back(selectedBoxTypeIdx, BinpackData::Pos(ip.P1.X, ip.P1.Y, ip.Rotated, currentBin));
                        int ar = BB->SizeX * BB->SizeY;
                        totalVolLeft -= ar;
                        binVolInserted += ar;
                        totalVolInserted += ar;
                    }
                } while (selectedBoxTypeIdx >= 0);

                currentBin++;
            }

            if (Conf.stripPacking) {
                Solution.setObj((double)totalVolInserted / (IOD.PSizeY * x_max));
            } else {
                if (Conf.binPackInt) {
                    Solution.setObj(currentBin);
                } else {
                    Solution.setObj((double)(currentBin-1) + (double)binVolInserted / (IOD.PSizeY * PSizeX));
                }
            }

            return Solution;
        }


        void init(const DataType &IOD) {
            gridSize = IOD.PSizeY / GRID_NUM;

            currentBin = 0;

            // wypełnienie zapotrzebowania
            Q = IOD.BoxToLoad;

            totalVolLeft = 0;
            numItems = 0;

            BA.assign(IOD.BoxTypes.size(), 0.0);
            int PV = IOD.PSizeY * IOD.PSizeY; // kwadrat!!!!!!
            for (int i = 0; i < Q.size(); i++) {
                auto &B = IOD.BoxTypes[i];
                BA[i] = (double) B.SizeX * B.SizeY / PV; // w relacji do kwadratu!! a nie do prostokąta
                totalVolLeft += Q[i]*(B.SizeX * B.SizeY);
                numItems += Q[i];
            }

            totalVolInserted = 0.0;
        }


        void initBin(const DataType &IOD)  {

            binVolInserted = 0;

            View1D.assign((IOD.PSizeY-1) / gridSize + 1, 0);

            CP = CornerPoints(PSizeX, IOD.PSizeY );

        }

        void computeProperties(const DataType &IOD, const BinpackData::BoxType *Box, const EvaListElementType &EE) {
            Properties.clear();

            int PSizeX = CP.PSizeX;
            int PSizeY = CP.PSizeY;

            int x_max = (Conf.stripPacking ? this->x_max : PSizeX);

            double PO = (double) PSizeY * PSizeY / 8; // 1/8 kwadratu!!!!!!

            for (auto v : View1D) {
                Properties.push_back(nnutils::scaleTanh(0.5/(PSizeY/2) * (x_max - v)));
            }

            int sx = Box->SizeX, sy = Box->SizeY;

            if (EE.first.Rotated) {
                swap(sx, sy);
            }

            // odległość od maksymalnej linii wypełnienia pudełka
            int DX =  x_max - (EE.first.P1.X + sx);

            //
            double ar = 0.0;
            int num = 0;
            for (int i = 0; i < Q.size(); i++) {
                // dane każdego typu pudełka
                auto &B = IOD.BoxTypes[i];

                num += Q[i];
                ar += Q[i]*(B.SizeX * B.SizeY);

            }

            // 1. pozostała powierzchnia wszystkich pudełek do zapakowania, w relacji do 8/8 powierzchi kwadratu palety
            Properties.push_back(nnutils::scaleTanh(0.5/(PO*8)  * ar));

            // 1. pozostała powierzchnia wszystkich pudełek do zapakowania, w relacji do 3/8 powierzchi kwadratu palety
            Properties.push_back(nnutils::scaleTanh(0.5/(PO*3)  * ar));

            //Pozostała powierzchnia danego pudełka w relacji do 2/8 powierzchni kwadratu palety
            Properties.push_back(nnutils::scaleTanh(0.5/(PO*2)  * (Box->SizeX * Box->SizeY) * Q[Box->idx] ));

            // 1. pozostała liczba wszystkich do zapakowania, w relacji do 10 sztuk
            Properties.push_back(nnutils::scaleTanh(0.5/10  * num));

            // 1. pozostała liczba danego typu do zapakowania, w relacji do 4 sztuk
            Properties.push_back(nnutils::scaleTanh(0.5/4  * Q[Box->idx]));
            // x7

            // mismach x
            Properties.push_back(nnutils::scaleTanh(0.5/((double)PSizeY/32) * EE.second.MismatchX));

            // mismach y
            Properties.push_back(nnutils::scaleTanh(0.5/((double)PSizeY/32) * EE.second.MismatchY));

            // rozmiary pudełka w porównaniu do Y palety
            Properties.push_back((double)sx*2/PSizeY);
            Properties.push_back((double)sy*2/PSizeY);

            // powierzchnia w relacji do 1/4 kwadratu
            Properties.push_back(nnutils::scaleTanh(0.5/0.25 * BA[Box->idx]));

            // dopasowanie wielokrotności pudełka do x i do y
            Properties.push_back(DX < 0 ? -1.0f : (float) (DX % sx) / (float)sx);
            // Features.push_back((float) (DX % sx) / sx);

            Properties.push_back((float) (EE.second.DY % sy) / (float)sy);

            // zmarnowane miejsce
            int dv = Box->SizeX * Box->SizeY;
            Properties.push_back( nnutils::scaleTanh( 0.5/0.5 *  ((double)(EE.second.VolumeIncrease - dv)) / dv));

            // pozycja wstawienia x względem x_max, skalowana względem rozmiaru Y pojemnika
            Properties.push_back(nnutils::scaleTanh(0.5/(PSizeY/2) * (x_max - EE.first.P1.X)));

            // pozycja wstawienia y wzlędem końca pojemnika
            // Features.push_back(scaleTanh(0.5/(CP.PSizeY/2) * (CP.PSizeY - EE.first.P1.Y)));
            Properties.push_back(((float) (PSizeY - EE.first.P1.Y)) / PSizeY);

            // odleglosc pudełka od krawedzi x i y: 0-1
            Properties.push_back(nnutils::scaleTanh(0.5/(PSizeY/2) * DX));
            // Features.push_back(scaleTanh(0.5/(CP.PSizeY/2) * EE.second.DY));
            Properties.push_back((float) EE.second.DY / PSizeY);


            if (Properties.size() != PROPERTIES_SIZE) {
                ERROR("Wrong number of properties provided. Properties.size()=" + to_string(Properties.size()));
            }
        }

        double evalBox(const DataType &IOD, const BinpackData::BoxType* Box, EvaListElementType &ERet) {
            EvaList.clear();
            CP.Evaluate(Box, false, EvaList);
            CP.Evaluate(Box, true, EvaList);

            if (EvaList.empty()) return -DBL_MAX;

            int best_i = -1;
            double best_eval = -DBL_MAX;


            auto View1DTmp = View1D;
            int x_max_tmp = x_max;

            for (int i = 0; i < EvaList.size(); i++) {

                auto &BB = Box;
                auto &EE = EvaList[i];
                auto &ip = EE.first;
                // int y_min = ip.P1.Y;

                // modyfikacja View1D
                int sx = BB->SizeX, sy = BB->SizeY;
                if (ip.Rotated) swap(sx, sy);
                int y_max = ip.P1.Y + sy - 1;
                y_max = (y_max + gridSize - gridSize/2) / gridSize - 1;
                y_max = min((int) y_max, (int) View1D.size() - 1);
                int maxItemPosX = ip.P1.X + sx;
                x_max = max(x_max, maxItemPosX); // jak daleko ulozenie siega w prawo

                for (int k = 0; k <= y_max; k++) {
                    View1D[k]  = max(View1D[k], maxItemPosX);
                }

                computeProperties(IOD, Box, EE);

                Map<Matrix<float, Dynamic, 1>> PropertiesMap(Properties.data(), Properties.size(), 1);

                double e = AF(PropertiesMap);

                if (e > best_eval) {
                    best_eval = e;
                    best_i = i;
                }

                View1D = View1DTmp;
                x_max = x_max_tmp;
            }


            if (best_i >= 0) {
                ERet = EvaList[best_i];
            }
            return best_eval;
        }
    };

    template<typename T>
        auto to_json(nlohmann::json& j, const T& c)
            -> std::enable_if_t<std::is_same_v<T, typename BinpackConstructionHeuristic<typename T::AFT, typename T::GenConfT>::ConfigType>>
    {
        j = nlohmann::json{};
        j.emplace("desc", c.desc);
        j.emplace("stripPacking", c.stripPacking);
        j.emplace("binPackInt", c.binPackInt);
        j.emplace("AConf", c.AConf);
        j.emplace("GConf", c.GConf);
    }

    template<typename T>
       auto from_json(nlohmann::json& j, T& c)
          -> std::enable_if_t<std::is_same_v<T, typename BinpackConstructionHeuristic<typename T::AFT, typename T::GenConfT>::ConfigType>> {
        j.at("desc").get_to(c.desc);
        j.at("stripPacking").get_to(c.stripPacking);
        j.at("binPackInt").get_to(c.binPackInt);
        j.at("AConf").get_to(c.AConf);
        j.at("GConf").get_to(c.GConf);
    }



//    static_assert(chof::ConstructionHeuristicConcept<BinpackConstructionHeuristic<nnutils::FFN>>);
};




