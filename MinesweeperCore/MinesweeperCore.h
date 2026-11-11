#pragma once
#include "IMinesweeperGame.h"
#include <memory>
#include <random>
#include <utility>
#include <vector>

class MinesweeperCore : public IMinesweeperGame {
public:
    // Seed-ul permite teste deterministe. Implicit e aleator.
    explicit MinesweeperCore(unsigned int seed = std::random_device{}());

    // ---------- IMinesweeperGame ----------
    void startNewGame(Difficulty difficulty) override;
    void revealCell(int row, int col) override;
    void toggleFlag(int row, int col) override;
    void addObserver(std::shared_ptr<IMinesweeperObserver> observer) override;

    // ---------- Extra (utile pentru teste si UI) ----------
    // Tabla cu dimensiuni custom (ex: 3x3 in teste).
    void startCustomGame(int rows, int cols, int mineCount);

    // Plaseaza minele pe pozitii fixe (pentru teste deterministe).
    // Se apeleaza dupa startNewGame / startCustomGame, inainte de primul revealCell.
    void setMines(const std::vector<std::pair<int, int>>& mines);

    // ---------- Interogari (declarate in IMinesweeperGame) ----------
    int getRows() const override { return m_rows; }
    int getCols() const override { return m_cols; }
    int getMineCount() const override { return m_mineCount; }
    int getFlagCount() const override { return m_flagCount; }
    GameState getGameState() const override { return m_state; }

    // Arunca std::out_of_range pentru coordonate invalide.
    CellState getCellState(int row, int col) const override;
    bool isMine(int row, int col) const override;

    // Doar in clasa concreta (UI-ul primeste numarul prin callback).
    int getAdjacentMines(int row, int col) const;

private:
    struct Cell {
        bool isMine = false;
        CellState state = CellState::Hidden;
        int adjacentMines = 0;
    };

    bool inBounds(int row, int col) const;
    void placeMines(int safeRow, int safeCol);
    void computeAdjacentCounts();
    void floodReveal(int row, int col);

    void notifyCellRevealed(int row, int col, int adjacentMines);
    void notifyFlagChanged(int row, int col, bool isFlagged);
    void notifyGameState(GameState state);

    std::vector<std::vector<Cell>> m_board;
    int m_rows = 0;
    int m_cols = 0;
    int m_mineCount = 0;
    int m_flagCount = 0;
    int m_revealedCount = 0;
    bool m_minesPlaced = false;
    GameState m_state = GameState::Ongoing;

    std::vector<std::shared_ptr<IMinesweeperObserver>> m_observers;
    std::mt19937 m_rng;
};