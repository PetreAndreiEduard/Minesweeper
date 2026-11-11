#include "MinesweeperCore.h"
#include <algorithm>
#include <cstdlib>
#include <queue>
#include <stdexcept>

MinesweeperCore::MinesweeperCore(unsigned int seed) : m_rng(seed) {}

void MinesweeperCore::startNewGame(Difficulty difficulty) {
    switch (difficulty) {
    case Difficulty::Beginner:     startCustomGame(9, 9, 10);    break;
    case Difficulty::Intermediate: startCustomGame(16, 16, 40);  break;
    case Difficulty::Expert:       startCustomGame(16, 30, 99);  break;
    }
}

void MinesweeperCore::startCustomGame(int rows, int cols, int mineCount) {
    if (rows <= 0 || cols <= 0 || mineCount < 0 || mineCount >= rows * cols) {
        throw std::invalid_argument("Parametri invalizi pentru tabla de joc");
    }

    m_rows = rows;
    m_cols = cols;
    m_mineCount = mineCount;
    m_board.assign(rows, std::vector<Cell>(cols));
    m_flagCount = 0;
    m_revealedCount = 0;
    m_minesPlaced = false;
    m_state = GameState::Ongoing;

    notifyGameState(m_state);
}

void MinesweeperCore::setMines(const std::vector<std::pair<int, int>>& mines) {
    for (auto& row : m_board)
        for (auto& cell : row)
            cell.isMine = false;

    int count = 0;
    for (const auto& [r, c] : mines) {
        if (inBounds(r, c) && !m_board[r][c].isMine) {
            m_board[r][c].isMine = true;
            ++count;
        }
    }
    m_mineCount = count;
    computeAdjacentCounts();
    m_minesPlaced = true;
}

void MinesweeperCore::revealCell(int row, int col) {
    if (m_state != GameState::Ongoing || !inBounds(row, col)) return;

    Cell& cell = m_board[row][col];
    if (cell.state != CellState::Hidden) return;

    if (!m_minesPlaced) {
        placeMines(row, col);
    }

    if (cell.isMine) {
        m_state = GameState::Lost;
        notifyGameState(m_state);
        return;
    }

    floodReveal(row, col);

    if (m_revealedCount == m_rows * m_cols - m_mineCount) {
        m_state = GameState::Won;
        notifyGameState(m_state);
    }
}

void MinesweeperCore::toggleFlag(int row, int col) {
    if (m_state != GameState::Ongoing || !inBounds(row, col)) return;

    Cell& cell = m_board[row][col];
    if (cell.state == CellState::Revealed) return;

    if (cell.state == CellState::Hidden) {
        cell.state = CellState::Flagged;
        ++m_flagCount;
        notifyFlagChanged(row, col, true);
    }
    else {
        cell.state = CellState::Hidden;
        --m_flagCount;
        notifyFlagChanged(row, col, false);
    }
}

void MinesweeperCore::addObserver(std::shared_ptr<IMinesweeperObserver> observer) {
    if (!observer) return;
    if (std::find(m_observers.begin(), m_observers.end(), observer) != m_observers.end()) return;
    m_observers.push_back(std::move(observer));
}

CellState MinesweeperCore::getCellState(int row, int col) const {
    return m_board.at(row).at(col).state;
}

bool MinesweeperCore::isMine(int row, int col) const {
    return m_board.at(row).at(col).isMine;
}

int MinesweeperCore::getAdjacentMines(int row, int col) const {
    return m_board.at(row).at(col).adjacentMines;
}

bool MinesweeperCore::inBounds(int row, int col) const {
    return row >= 0 && row < m_rows && col >= 0 && col < m_cols;
}

void MinesweeperCore::placeMines(int safeRow, int safeCol) {
    std::vector<int> candidates;

    auto buildCandidates = [&](bool excludeNeighbors) {
        candidates.clear();
        for (int r = 0; r < m_rows; ++r) {
            for (int c = 0; c < m_cols; ++c) {
                int dr = std::abs(r - safeRow);
                int dc = std::abs(c - safeCol);
                bool inSafeZone = excludeNeighbors ? (dr <= 1 && dc <= 1)
                    : (dr == 0 && dc == 0);
                if (!inSafeZone) candidates.push_back(r * m_cols + c);
            }
        }
        };

    buildCandidates(true);
    if (static_cast<int>(candidates.size()) < m_mineCount) {
        buildCandidates(false);
    }

    std::shuffle(candidates.begin(), candidates.end(), m_rng);
    for (int i = 0; i < m_mineCount; ++i) {
        int idx = candidates[i];
        m_board[idx / m_cols][idx % m_cols].isMine = true;
    }

    computeAdjacentCounts();
    m_minesPlaced = true;
}

void MinesweeperCore::computeAdjacentCounts() {
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            int count = 0;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    if (dr == 0 && dc == 0) continue;
                    int nr = r + dr, nc = c + dc;
                    if (inBounds(nr, nc) && m_board[nr][nc].isMine) ++count;
                }
            }
            m_board[r][c].adjacentMines = count;
        }
    }
}

void MinesweeperCore::floodReveal(int row, int col) {
    std::queue<std::pair<int, int>> pending;

    auto reveal = [&](int r, int c) {
        Cell& cell = m_board[r][c];
        cell.state = CellState::Revealed;
        ++m_revealedCount;
        notifyCellRevealed(r, c, cell.adjacentMines);
        if (cell.adjacentMines == 0) pending.push({ r, c });
        };

    reveal(row, col);

    while (!pending.empty()) {
        auto [r, c] = pending.front();
        pending.pop();

        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                int nr = r + dr, nc = c + dc;
                if (inBounds(nr, nc) && m_board[nr][nc].state == CellState::Hidden) {
                    reveal(nr, nc);
                }
            }
        }
    }
}

void MinesweeperCore::notifyCellRevealed(int row, int col, int adjacentMines) {
    for (auto& obs : m_observers) obs->onCellRevealed(row, col, adjacentMines);
}

void MinesweeperCore::notifyFlagChanged(int row, int col, bool isFlagged) {
    for (auto& obs : m_observers) obs->onCellFlagChanged(row, col, isFlagged);
}

void MinesweeperCore::notifyGameState(GameState state) {
    for (auto& obs : m_observers) obs->onGameStateChanged(state);
}