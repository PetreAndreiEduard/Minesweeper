#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "MinesweeperCore.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::StrictMock;

class MockObserver : public IMinesweeperObserver {
public:
    MOCK_METHOD3(onCellRevealed, void(int row, int col, int adjacentMines));
    MOCK_METHOD3(onCellFlagChanged, void(int row, int col, bool isFlagged));
    MOCK_METHOD1(onGameStateChanged, void(GameState state));
};

using Mines = std::vector<std::pair<int, int>>;


class MinesweeperCoreTest : public ::testing::Test {
protected:
    MinesweeperCore game{ 42 };
    std::shared_ptr<StrictMock<MockObserver>> obs =
        std::make_shared<StrictMock<MockObserver>>();

    void setupBoard(int rows, int cols, const Mines& mines) {
        game.startCustomGame(rows, cols, static_cast<int>(mines.size()));
        game.setMines(mines);
        game.addObserver(obs);
    }
};


TEST(MinesweeperCoreInit, DifficultyLevelsSetDimensionsAndMines) {
    MinesweeperCore game;

    game.startNewGame(Difficulty::Beginner);
    EXPECT_EQ(game.getRows(), 9);
    EXPECT_EQ(game.getCols(), 9);
    EXPECT_EQ(game.getMineCount(), 10);

    game.startNewGame(Difficulty::Intermediate);
    EXPECT_EQ(game.getRows(), 16);
    EXPECT_EQ(game.getCols(), 16);
    EXPECT_EQ(game.getMineCount(), 40);

    game.startNewGame(Difficulty::Expert);
    EXPECT_EQ(game.getRows(), 16);
    EXPECT_EQ(game.getCols(), 30);
    EXPECT_EQ(game.getMineCount(), 99);
}

TEST(MinesweeperCoreInit, NewGameStartsOngoingWithAllCellsHidden) {
    MinesweeperCore game;
    game.startNewGame(Difficulty::Beginner);

    EXPECT_EQ(game.getGameState(), GameState::Ongoing);
    EXPECT_EQ(game.getFlagCount(), 0);
    for (int r = 0; r < game.getRows(); ++r)
        for (int c = 0; c < game.getCols(); ++c)
            EXPECT_EQ(game.getCellState(r, c), CellState::Hidden);
}

TEST(MinesweeperCoreInit, NewGameNotifiesOngoing) {
    MinesweeperCore game;
    auto obs = std::make_shared<StrictMock<MockObserver>>();
    game.addObserver(obs);

    EXPECT_CALL(*obs, onGameStateChanged(GameState::Ongoing)).Times(1);
    game.startNewGame(Difficulty::Beginner);
}

TEST(MinesweeperCoreInit, InvalidCustomGameThrows) {
    MinesweeperCore game;
    EXPECT_THROW(game.startCustomGame(0, 5, 1), std::invalid_argument);
    EXPECT_THROW(game.startCustomGame(3, 3, 9), std::invalid_argument);
    EXPECT_THROW(game.startCustomGame(3, 3, -1), std::invalid_argument);
}

TEST(MinesweeperCoreInit, RestartAfterGameOverResetsEverything) {
    MinesweeperCore game{ 1 };
    game.startCustomGame(3, 3, 1);
    game.setMines({ { 1, 1 } });
    game.toggleFlag(0, 0);
    game.revealCell(1, 1);                       
    ASSERT_EQ(game.getGameState(), GameState::Lost);

    game.startNewGame(Difficulty::Beginner);

    EXPECT_EQ(game.getGameState(), GameState::Ongoing);
    EXPECT_EQ(game.getFlagCount(), 0);
    EXPECT_EQ(game.getCellState(0, 0), CellState::Hidden);
}


TEST_F(MinesweeperCoreTest, AdjacentMineCountsAreCorrect) {
    setupBoard(3, 3, { { 0, 0 },{ 0, 2 } });

    EXPECT_EQ(game.getAdjacentMines(0, 1), 2);
    EXPECT_EQ(game.getAdjacentMines(1, 0), 1);
    EXPECT_EQ(game.getAdjacentMines(1, 1), 2);
    EXPECT_EQ(game.getAdjacentMines(1, 2), 1);
    EXPECT_EQ(game.getAdjacentMines(2, 0), 0);
    EXPECT_EQ(game.getAdjacentMines(2, 1), 0);
    EXPECT_EQ(game.getAdjacentMines(2, 2), 0);
}


TEST_F(MinesweeperCoreTest, FloodFillOnEmptyBoardRevealsEverythingAndWins) {
    setupBoard(3, 3, {});

    EXPECT_CALL(*obs, onCellRevealed(_, _, 0)).Times(9);
    EXPECT_CALL(*obs, onGameStateChanged(GameState::Won)).Times(1);

    game.revealCell(1, 1);

    EXPECT_EQ(game.getGameState(), GameState::Won);
}

TEST_F(MinesweeperCoreTest, FloodFillStopsAtNumberedCells) {
    setupBoard(1, 5, { { 0, 2 } });

    EXPECT_CALL(*obs, onCellRevealed(0, 0, 0)).Times(1);
    EXPECT_CALL(*obs, onCellRevealed(0, 1, 1)).Times(1);

    game.revealCell(0, 0);

    EXPECT_EQ(game.getCellState(0, 2), CellState::Hidden);
    EXPECT_EQ(game.getCellState(0, 3), CellState::Hidden);
    EXPECT_EQ(game.getCellState(0, 4), CellState::Hidden);
    EXPECT_EQ(game.getGameState(), GameState::Ongoing);
}

TEST_F(MinesweeperCoreTest, FloodFillSkipsFlaggedCells) {
    setupBoard(1, 4, { { 0, 3 } });

    EXPECT_CALL(*obs, onCellFlagChanged(0, 1, true)).Times(1);
    EXPECT_CALL(*obs, onCellRevealed(0, 0, 0)).Times(1);

    game.toggleFlag(0, 1);
    game.revealCell(0, 0);

    EXPECT_EQ(game.getCellState(0, 1), CellState::Flagged);
    EXPECT_EQ(game.getCellState(0, 2), CellState::Hidden);
}

TEST_F(MinesweeperCoreTest, RevealingNumberedCellRevealsOnlyThatCell) {

    setupBoard(3, 3, { { 0, 0 },{ 0, 2 } });

    EXPECT_CALL(*obs, onCellRevealed(1, 1, 2)).Times(1);

    game.revealCell(1, 1);

    EXPECT_EQ(game.getCellState(1, 1), CellState::Revealed);
    EXPECT_EQ(game.getCellState(1, 0), CellState::Hidden);
}

TEST_F(MinesweeperCoreTest, RevealingAlreadyRevealedCellDoesNothing) {
    setupBoard(3, 3, { { 0, 0 },{ 0, 2 } });

    EXPECT_CALL(*obs, onCellRevealed(1, 1, 2)).Times(1);

    game.revealCell(1, 1);
    game.revealCell(1, 1);
}


TEST(MinesweeperCoreFirstClick, FirstClickIsNeverAMineAndOpensAnArea) {
    const std::pair<int, int> clicks[] = { { 0, 0 },{ 0, 8 },{ 4, 4 },{ 8, 0 },{ 8, 8 },{ 3, 7 } };

    for (unsigned seed = 0; seed < 200; ++seed) {
        for (const auto& [r, c] : clicks) {
            MinesweeperCore game{ seed };
            game.startNewGame(Difficulty::Beginner);

            game.revealCell(r, c);

            ASSERT_NE(game.getGameState(), GameState::Lost) << "seed=" << seed;
            ASSERT_FALSE(game.isMine(r, c)) << "seed=" << seed;
            ASSERT_EQ(game.getCellState(r, c), CellState::Revealed) << "seed=" << seed;
            ASSERT_EQ(game.getAdjacentMines(r, c), 0) << "seed=" << seed;
        }
    }
}

TEST(MinesweeperCoreFirstClick, PlacesExactlyTheRequestedNumberOfMines) {
    for (unsigned seed = 0; seed < 50; ++seed) {
        MinesweeperCore game{ seed };
        game.startNewGame(Difficulty::Expert);
        game.revealCell(8, 15);

        int mines = 0;
        for (int r = 0; r < game.getRows(); ++r)
            for (int c = 0; c < game.getCols(); ++c)
                if (game.isMine(r, c)) ++mines;

        ASSERT_EQ(mines, 99) << "seed=" << seed;
    }
}

TEST(MinesweeperCoreFirstClick, SameSeedGivesSameLayout) {
    MinesweeperCore a{ 7 }, b{ 7 };
    a.startNewGame(Difficulty::Beginner);
    b.startNewGame(Difficulty::Beginner);
    a.revealCell(4, 4);
    b.revealCell(4, 4);

    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c)
            EXPECT_EQ(a.isMine(r, c), b.isMine(r, c));
}

TEST(MinesweeperCoreFirstClick, TinyBoardFallsBackToExcludingOnlyClickedCell) {
    for (unsigned seed = 0; seed < 50; ++seed) {
        MinesweeperCore game{ seed };
        game.startCustomGame(2, 2, 3);
        game.revealCell(0, 0);

        ASSERT_FALSE(game.isMine(0, 0)) << "seed=" << seed;
        ASSERT_EQ(game.getGameState(), GameState::Won) << "seed=" << seed;
    }
}


TEST_F(MinesweeperCoreTest, RevealingAMineEndsTheGame) {
    setupBoard(3, 3, { { 1, 1 } });

    EXPECT_CALL(*obs, onGameStateChanged(GameState::Lost)).Times(1);

    game.revealCell(1, 1);

    EXPECT_EQ(game.getGameState(), GameState::Lost);
}

TEST_F(MinesweeperCoreTest, NoActionsAllowedAfterGameOver) {
    setupBoard(3, 3, { { 1, 1 } });

    EXPECT_CALL(*obs, onGameStateChanged(GameState::Lost)).Times(1);
    game.revealCell(1, 1);

    game.revealCell(0, 0);
    game.toggleFlag(0, 0);

    EXPECT_EQ(game.getCellState(0, 0), CellState::Hidden);
    EXPECT_EQ(game.getGameState(), GameState::Lost);
}


TEST_F(MinesweeperCoreTest, ToggleFlagPlacesAndRemovesFlag) {
    setupBoard(3, 3, { { 1, 1 } });

    {
        InSequence seq;
        EXPECT_CALL(*obs, onCellFlagChanged(0, 0, true)).Times(1);
        EXPECT_CALL(*obs, onCellFlagChanged(0, 0, false)).Times(1);
    }

    game.toggleFlag(0, 0);
    EXPECT_EQ(game.getCellState(0, 0), CellState::Flagged);
    EXPECT_EQ(game.getFlagCount(), 1);

    game.toggleFlag(0, 0);
    EXPECT_EQ(game.getCellState(0, 0), CellState::Hidden);
    EXPECT_EQ(game.getFlagCount(), 0);
}

TEST_F(MinesweeperCoreTest, CannotFlagARevealedCell) {
    setupBoard(3, 3, { { 0, 0 },{ 0, 2 } });

    EXPECT_CALL(*obs, onCellRevealed(1, 1, 2)).Times(1);
    game.revealCell(1, 1);

    game.toggleFlag(1, 1);

    EXPECT_EQ(game.getCellState(1, 1), CellState::Revealed);
    EXPECT_EQ(game.getFlagCount(), 0);
}

TEST_F(MinesweeperCoreTest, FlaggedCellCannotBeRevealed) {
    setupBoard(3, 3, { { 1, 1 } });

    EXPECT_CALL(*obs, onCellFlagChanged(1, 1, true)).Times(1);
    game.toggleFlag(1, 1);

    game.revealCell(1, 1);

    EXPECT_EQ(game.getGameState(), GameState::Ongoing);
    EXPECT_EQ(game.getCellState(1, 1), CellState::Flagged);
}

TEST_F(MinesweeperCoreTest, WrongFlagBlocksVictoryUntilRemoved) {
    setupBoard(2, 2, { { 0, 0 } });

    EXPECT_CALL(*obs, onCellFlagChanged(1, 1, true)).Times(1);
    EXPECT_CALL(*obs, onCellFlagChanged(1, 1, false)).Times(1);
    EXPECT_CALL(*obs, onCellRevealed(0, 1, 1)).Times(1);
    EXPECT_CALL(*obs, onCellRevealed(1, 0, 1)).Times(1);
    EXPECT_CALL(*obs, onCellRevealed(1, 1, 1)).Times(1);
    EXPECT_CALL(*obs, onGameStateChanged(GameState::Won)).Times(1);

    game.toggleFlag(1, 1);       
    game.revealCell(0, 1);
    game.revealCell(1, 0);
    game.revealCell(1, 1);
    EXPECT_EQ(game.getGameState(), GameState::Ongoing);

    game.toggleFlag(1, 1);
    game.revealCell(1, 1);
    EXPECT_EQ(game.getGameState(), GameState::Won);
}


TEST_F(MinesweeperCoreTest, WinningRequiresAllSafeCellsRevealed) {
    setupBoard(2, 2, { { 0, 0 } });

    {
        InSequence seq;
        EXPECT_CALL(*obs, onCellRevealed(0, 1, 1)).Times(1);
        EXPECT_CALL(*obs, onCellRevealed(1, 0, 1)).Times(1);
        EXPECT_CALL(*obs, onCellRevealed(1, 1, 1)).Times(1);
        EXPECT_CALL(*obs, onGameStateChanged(GameState::Won)).Times(1);
    }

    game.revealCell(0, 1);
    game.revealCell(1, 0);
    EXPECT_EQ(game.getGameState(), GameState::Ongoing);

    game.revealCell(1, 1);
    EXPECT_EQ(game.getGameState(), GameState::Won);
}

TEST_F(MinesweeperCoreTest, NoActionsAllowedAfterWinning) {
    setupBoard(2, 2, { { 0, 0 } });

    EXPECT_CALL(*obs, onCellRevealed(_, _, 1)).Times(3);
    EXPECT_CALL(*obs, onGameStateChanged(GameState::Won)).Times(1);
    game.revealCell(0, 1);
    game.revealCell(1, 0);
    game.revealCell(1, 1);

    game.revealCell(0, 0);
    game.toggleFlag(0, 0);

    EXPECT_EQ(game.getGameState(), GameState::Won);
}

TEST_F(MinesweeperCoreTest, OutOfBoundsActionsAreIgnored) {
    setupBoard(3, 3, { { 1, 1 } });

    game.revealCell(-1, 0);
    game.revealCell(0, -1);
    game.revealCell(3, 0);
    game.revealCell(0, 3);
    game.toggleFlag(-1, -1);
    game.toggleFlag(3, 3);

    EXPECT_EQ(game.getGameState(), GameState::Ongoing);
    EXPECT_EQ(game.getFlagCount(), 0);
}

TEST_F(MinesweeperCoreTest, QueriesOutOfBoundsThrow) {
    setupBoard(3, 3, { { 1, 1 } });

    EXPECT_THROW(game.getCellState(3, 0), std::out_of_range);
    EXPECT_THROW(game.getCellState(-1, 0), std::out_of_range);
    EXPECT_THROW(game.isMine(0, 3), std::out_of_range);
}

TEST(MinesweeperCoreObserver, SameObserverRegisteredTwiceIsNotifiedOnce) {
    MinesweeperCore game{ 1 };
    game.startCustomGame(3, 3, 1);
    game.setMines({ { 1, 1 } });

    auto obs = std::make_shared<StrictMock<MockObserver>>();
    game.addObserver(obs);
    game.addObserver(obs);

    EXPECT_CALL(*obs, onCellFlagChanged(0, 0, true)).Times(1);
    game.toggleFlag(0, 0);
}

TEST(MinesweeperCoreObserver, MultipleObserversAllGetNotified) {
    MinesweeperCore game{ 1 };
    game.startCustomGame(3, 3, 1);
    game.setMines({ { 1, 1 } });

    auto a = std::make_shared<StrictMock<MockObserver>>();
    auto b = std::make_shared<StrictMock<MockObserver>>();
    game.addObserver(a);
    game.addObserver(b);

    EXPECT_CALL(*a, onGameStateChanged(GameState::Lost)).Times(1);
    EXPECT_CALL(*b, onGameStateChanged(GameState::Lost)).Times(1);
    game.revealCell(1, 1);
}

TEST(MinesweeperCoreObserver, NullObserverIsIgnored) {
    MinesweeperCore game{ 1 };
    EXPECT_NO_THROW(game.addObserver(nullptr));
    EXPECT_NO_THROW(game.startNewGame(Difficulty::Beginner));
}
