#pragma once
#include "Enums.h"
#include "IMinesweeperObserver.h"
#include <memory>

class IMinesweeperGame {
public:
    virtual ~IMinesweeperGame() = default;
    virtual void startNewGame(Difficulty difficulty) = 0;
    virtual void revealCell(int row, int col) = 0;
    virtual void toggleFlag(int row, int col) = 0;
    virtual void addObserver(std::shared_ptr<IMinesweeperObserver> observer) = 0;
};