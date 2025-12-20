#pragma once
#include  <string>
#include <utils.h>
#include <vector>
#include "DataConcept.h"





namespace binpack {
    using namespace std;
    using namespace chof;


   // typedef pair<int, Pos> BoxPosType;


    struct BinpackData {

        struct BoxType    {

            int idx;

            int SizeX;
            int SizeY;

            bool operator==(const BoxType &o) const {
                return SizeX == o.SizeX && SizeY == o.SizeY;
            }
            bool operator!=(const BoxType &o) const {
                return !(*this == o) ;
            }

            BoxType() {
            }

            BoxType(int _SizeX, int _SizeY) :  SizeX(_SizeX), SizeY(_SizeY) {
            }
            BoxType(int _idx, int _SizeX, int _SizeY) : idx(_idx), SizeX(_SizeX), SizeY(_SizeY) {
            }
        };

        struct Pos {
            int X;
            int Y;
            bool Rotated; // true - rotated, false - original
            int binIdx;

            Pos() : X(0), Y(0), Rotated(false), binIdx(-1) {
            }

            Pos(int _X, int _Y,bool _Rotated, int _binIdx) :
            X(_X), Y(_Y), Rotated(_Rotated), binIdx(_binIdx) {
            }
        };


        // DataConcept interface

        struct SolutionType {
            double obj;
            vector<pair<int, Pos>> BPV;
            int cluster = -1;

            // interface of the SolutionConcept
            double getObj() const {
                return obj;
            }
            void setObj (double _obj) {
                obj = _obj;
            }
            void setCluster(int c) {
                cluster = c;
            }
        };

        double getObj() const {
            return Solution.getObj();
        }

        const SolutionType& getSolution() {
            return Solution;
        }

        void setSolution(const SolutionType &_solution) {
            Solution = _solution;
        }

        // interface end


        string name;

        vector<BoxType> BoxTypes;
        vector<int> BoxToLoad;

        SolutionType Solution;


        int PSizeX = -1; // [mm] Palette dimensions, common to all palettes
        int PSizeY = -1;


        bool operator==(const BinpackData &o) const {
            if (BoxTypes.size() != o.BoxTypes.size() || BoxToLoad.size() != o.BoxToLoad.size()) return false;
            for (int i = 0; i < BoxTypes.size(); i++) {
                if (BoxTypes[i] != o.BoxTypes[i]) return false;
                if (BoxToLoad[i] != o.BoxToLoad[i]) return false;
            }
            return true;
        }

        BinpackData(int _PSizeX, int _PSizeY) {
            PSizeX = _PSizeX;
            PSizeY = _PSizeY;
        }

        BinpackData() {
        }

        void printSolution( string fn ) {
            auto ofs = nnutils::openFileWithDirs<ofstream>(fn);

            ofs << "box_idx; box_width; box_height; rotated; x; y; bin_idx" << endl;

            for ( auto &Sol: Solution.BPV) {
                int idx = Sol.first;
                auto Pos = Sol.second;
                ofs << idx << "; " << BoxTypes[idx].SizeX << "; " << BoxTypes[idx].SizeY << "; " << (Pos.Rotated?1:0) << "; " << Pos.X << "; " << Pos.Y << "; " << Pos.binIdx << endl;
            }
        }

        //
        // vector<int> getVecToLoad() const {
        //     return BoxToLoad;
        // }
    };



    inline void exportToJson( const vector<BinpackData> &DIOs, string fn ) {
        auto ofs = nnutils::openFileWithDirs<ofstream>(fn);

        ofs << "[" << endl;

        int idx = 0;
        for ( auto &DIO: DIOs) {
            ofs << "{" << endl;

            ofs << "\"bin_width\": " << DIO.PSizeY << "," << endl;
            ofs << "\"items\": [" << endl;

            for ( int i = 0; i < DIO.BoxTypes.size(); i++) {
                for ( int j = 0; j < DIO.BoxToLoad[i]; j++ ) {
                    ofs << "[" << DIO.BoxTypes[i].SizeX << ", " << DIO.BoxTypes[i].SizeY << "]";

                    if (i < DIO.BoxTypes.size() - 1 || j < DIO.BoxToLoad[i] - 1) {
                        ofs << ",";
                    }
                    ofs << endl;
                }
            }

            ofs << "]" << endl;

            ofs << "}";

            idx += 1;
            if (idx < DIOs.size()) {
                ofs << ",";
            }
            ofs << endl;
        }

        ofs << "]" << endl << flush;

    }




    static_assert(SolutionConcept<BinpackData::SolutionType>);
    static_assert(DataConcept<BinpackData>);

}

