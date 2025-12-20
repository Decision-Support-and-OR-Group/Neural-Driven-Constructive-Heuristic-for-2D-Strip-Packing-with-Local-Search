#include <random>
#include <SFML/Graphics.hpp>
#include "BinDrawer.h"
#include "utils.h"
#include <set>
#include <string>

namespace binpack {
    void BinDrawer::populateColors() {
        Colors.clear();
        for( int r = 0; r < 3; r++ ) {
            for( int g = 0; g < 3; g++ ) {
                for( int b = 0; b < 3; b++ ) {
                    if (r == b && g == b && r == g) continue;
                    Colors.push_back( sf::Color( (r+1)*64, (g+1)*64, (b+1)*64 ) );
                }
            }
        }

        shuffle(Colors.begin(), Colors.end(), std::mt19937(1));
    }

 // void BinDrawer::drawToFile(const BinpackData &IOD, bool flip, string dir, string ext) {
 //
 //        set<int> Bins;
 //        for ( auto &BP: IOD.Solution.BPV) {
 //            Bins.insert(BP.second.binIdx);
 //        }
 //
 //        for ( auto binIdx: Bins ) {
 //            const float IMG_SIZE = 512;
 //
 //            float SCALE = IMG_SIZE / (flip?IOD.PSizeX: IOD.PSizeY);
 //            float WIDTH = IOD.PSizeX * SCALE;
 //            float HEIGHT = IOD.PSizeY * SCALE;
 //            if (flip) {
 //                swap(WIDTH, HEIGHT);
 //            }
 //
 //            sf::RenderTexture window;
 //            window.create(WIDTH+4, HEIGHT+4);
 //            sf::View view = window.getDefaultView();
 //            view.setSize(WIDTH+4, -(HEIGHT+4) );
 //            window.setView(view);
 //            window.clear(sf::Color::Black);
 //            sf::RectangleShape Background({WIDTH, HEIGHT});
 //            Background.setFillColor(sf::Color::White);
 //            Background.setPosition(2, 2);
 //
 //            sf::Text text;
 //
 //            // select the font
 //            text.setFont(font); // font is a sf::Font
 //
 //            // set the string to display
 //         //   text.setString(nnutils::to_string_with_precision(IOD.getObj()*100, 2));
 //
 //            // set the character size
 //            text.setCharacterSize( SCALE*70 ); // in pixels, not points!
 //            text.setFillColor(sf::Color::Black);
 //            text.setPosition(WIDTH - IMG_SIZE/3,  IMG_SIZE/100);
 //
 //            vector<sf::RectangleShape> Rects;
 //            vector<int> BoxTypes;
 //
 //            sf::Text txtNum;
 //
 //            // select the font
 //            txtNum.setFont(font); // font is a sf::Font
 //            txtNum.setCharacterSize( SCALE*30 ); // in pixels, not points!
 //            txtNum.setFillColor(sf::Color::Black);
 //
 //
 //            for( auto &BP: IOD.Solution.BPV ) {
 //                auto &B = IOD.BoxTypes[BP.first];
 //                auto Pos = BP.second;
 //                if (Pos.binIdx != binIdx) {
 //                    continue;
 //                }
 //
 //                float PosX = Pos.X * SCALE + 2;
 //                float PosY = Pos.Y * SCALE + 2;
 //
 //                float sx = B.SizeX * SCALE;
 //                float sy = B.SizeY * SCALE;
 //                if (Pos.Rotated) swap(sx, sy);
 //
 //                if (flip) {
 //                    swap(sx, sy);
 //                    sy = -sy;
 //                    swap(PosX, PosY);
 //                    PosY = HEIGHT - PosY;
 //                }
 //
 //                sy = -sy;
 //                PosY = HEIGHT - PosY;
 //
 //
 //                sf::RectangleShape Rect(sf::Vector2f(sx, sy));
 //
 //                if (B.idx < Colors.size()) {
 //                    Rect.setFillColor(Colors[B.idx]);
 //                    Rect.setOutlineColor(sf::Color::Black);
 //                    Rect.setOutlineThickness(-IMG_SIZE/200);
 //                }
 //                Rect.setPosition(sf::Vector2f(PosX, PosY));
 //                Rects.push_back(Rect);
 //                BoxTypes.push_back(B.idx);
 //            }
 //
 //            // while (window.isOpen()) {
 //            window.clear();
 //            window.draw(Background);
 //
 //            for( int i = 0; i < Rects.size(); i++ ) {
 //                auto &R = Rects[i];
 //                int bt = BoxTypes[i];
 //                window.draw(R);
 //
 //                txtNum.setString(std::to_string(bt));
 //
 //                auto P = R.getPosition();
 //                auto S = R.getSize();
 //                S /= 2.0f;
 //                auto NP = R.getPosition() + R.getSize()/2.0f;
 //                txtNum.setPosition(NP);
 //
 //                window.draw(txtNum);
 //            }
 //
 //            if (Bins.size() == 1) {
 //                window.draw(text);
 //            }
 //
 //            // }
 //            sf::Texture outputTexture = window.getTexture();
 //
 //            sf::Image output;
 //            output = outputTexture.copyToImage();
 //            string fn = dir + "/" + IOD.name  + "_bin" + nnutils::to_string(binIdx, 2) + ext;
 //            output.saveToFile(fn);
 //        }
 //    }

    void BinDrawer::drawToFile(const BinpackData &IOD, bool flip, string dir, string ext) {

        int max_height = 0;
        set<int> Bins;
        for ( auto &BP: IOD.Solution.BPV) {
            Bins.insert(BP.second.binIdx);
            max_height = max(max_height, BP.second.X + max(IOD.BoxTypes[BP.first].SizeX, IOD.BoxTypes[BP.first].SizeY));
        }

        for ( auto binIdx: Bins ) {
            const float IMG_SIZE = 512;


            float SCALE = IMG_SIZE / (IOD.PSizeY);

            float WIDTH = max_height * SCALE;
            float HEIGHT = IOD.PSizeY * SCALE;
            if (flip) {
                swap(WIDTH, HEIGHT);
            }

            sf::RenderTexture window;
            window.create(WIDTH+4, HEIGHT+4);
            sf::View view = window.getDefaultView();
            view.setSize(WIDTH+4, -(HEIGHT+4) );
            window.setView(view);
            window.clear(sf::Color::Black);
            sf::RectangleShape Background({WIDTH, HEIGHT});
            Background.setFillColor(sf::Color::White);
            Background.setPosition(2, 2);

            sf::Text text;

            // select the font
            text.setFont(font); // font is a sf::Font

            // set the string to display
        //    text.setString(nnutils::to_string_with_precision(IOD.getObj()*100, 2));

            // set the character size
            text.setCharacterSize( SCALE*70 ); // in pixels, not points!
            text.setFillColor(sf::Color::Black);
            text.setPosition(WIDTH - IMG_SIZE/3,  IMG_SIZE/100);

            vector<sf::RectangleShape> Rects;
            vector<int> BoxTypes;

            sf::Text txtNum;

            // select the font
            txtNum.setFont(font); // font is a sf::Font
            txtNum.setCharacterSize( SCALE*70 ); // in pixels, not points!
            txtNum.setFillColor(sf::Color::Black);


            for( auto &BP: IOD.Solution.BPV ) {
                auto &B = IOD.BoxTypes[BP.first];
                auto Pos = BP.second;
                if (Pos.binIdx != binIdx) {
                    continue;
                }

                float PosX = Pos.X * SCALE + 2;
                float PosY = Pos.Y * SCALE + 2;

                float sx = B.SizeX * SCALE;
                float sy = B.SizeY * SCALE;
                if (Pos.Rotated) swap(sx, sy);

                if (flip) {
                    swap(sx, sy);
                    sy = -sy;
                    swap(PosX, PosY);
                    PosY = HEIGHT - PosY;
                }


                sf::RectangleShape Rect(sf::Vector2f(sx, sy));

                if (B.idx < Colors.size()) {
                    Rect.setFillColor(Colors[B.idx]);
                    Rect.setOutlineColor(sf::Color::Black);
                    Rect.setOutlineThickness(-IMG_SIZE/200);
                }
                Rect.setPosition(sf::Vector2f(PosX, PosY));
                Rects.push_back(Rect);
                BoxTypes.push_back(B.idx);
            }

            // while (window.isOpen()) {
            window.clear();
            window.draw(Background);

            for( int i = 0; i < Rects.size(); i++ ) {
                auto &R = Rects[i];
                int bt = BoxTypes[i];
                window.draw(R);

                txtNum.setString(std::to_string(bt));

                auto P = R.getPosition();
                auto S = R.getSize();
                S /= 2.0f;
                auto NP = R.getPosition() + R.getSize()/2.0f;
                NP.y -= 15;
                NP.x -= 5;
                txtNum.setPosition(NP);

                window.draw(txtNum);
            }

            if (Bins.size() == 1) {
                window.draw(text);
            }

            // }
            sf::Texture outputTexture = window.getTexture();

            sf::Image output;
            output = outputTexture.copyToImage();
            string fn = dir + "/" + IOD.name  + "_bin" + nnutils::to_string(binIdx, 2) + ext;
            output.saveToFile(fn);
        }
    }



}