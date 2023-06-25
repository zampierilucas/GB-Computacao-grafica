#ifndef DiamondView_h
#define DiamondView_h

#include <iostream>
#include "ExercicioM6_TilemapView.h"

using namespace std;

class DiamondView : public TilemapView {
public:
    void computeDrawPosition(const int col, const int row, const float tw, const float th, float& targetx, float& targety) const {
        targetx = (col * (tw / 2.0f)) + (row * (tw / 2.0f));
        targety = (col * (th / 2.0f)) - (row * (th / 2.0f));
    }

    void computeMouseMap(int& col, int& row, const int windowHeight, const float tw, const float th, const float mx, const float my) const {
        int x = mx;
        int y = my - ((windowHeight / 2) - (th / 2));

        col = (int)(((double)x / (double)tw) + ((double)y / (double)th) - 0.5);
        row = (int)(((double)x / (double)tw) - ((double)y / (double)th) + 0.5);
    }

    // void computeTileWalking(int& col, int& row, const int direction) const {
    //     throw new exception("Error, Method Not Implemented Exception!");
    // }
};
#endif
