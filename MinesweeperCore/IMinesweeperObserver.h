#pragma once
#include "Enums.h"

class IMinesweeperObserver {
public:
    virtual ~IMinesweeperObserver() = default;
    virtual void onCellRevealed(int row, int col, int adjacentMines) = 0;
    virtual void onCellFlagChanged(int row, int col, bool isFlagged) = 0;
    virtual void onGameStateChanged(GameState state) = 0;
};