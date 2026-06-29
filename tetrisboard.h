#ifndef TETRISBOARD_H
#define TETRISBOARD_H

#include <QTimer>
#include <QWidget>

class TetrisBoard : public QWidget
{
    Q_OBJECT

public:
    enum GameState { Idle, Playing, Paused, GameOver };
    static constexpr int Width  = 10;
    static constexpr int Height = 20;

    explicit TetrisBoard(QWidget *parent = nullptr);

    void startGame();
    void pauseGame();
    void resumeGame();
    void togglePause();

    GameState state() const { return m_state; }
    int score() const { return m_score; }
    int level() const { return m_level; }
    int lines() const { return m_lines; }
    int nextPieceType() const { return m_nextType; }

signals:
    void scoreChanged(int score);
    void levelChanged(int level);
    void linesChanged(int lines);
    void nextPieceChanged(int type);
    void gameStarted();
    void gamePaused(bool paused);
    void gameOver();
    void stateChanged(TetrisBoard::GameState state);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameTick();

private:
    void spawnPiece();
    bool movePiece(int dx, int dy);
    bool rotatePiece();
    void dropPiece();
    bool isCollision(int type, int rotation, int x, int y) const;
    void lockPiece();
    int  clearLines();
    void updateScore(int cleared);
    int  dropSpeed() const;
    void resetBoard();

    // Game state
    GameState m_state;
    int m_board[Height][Width];

    // Current piece
    int m_curType;
    int m_curRotation;
    int m_curX;
    int m_curY;

    // Next piece
    int m_nextType;

    // Score tracking
    int m_score;
    int m_level;
    int m_lines;
    int m_totalLinesCleared;

    // Timer
    QTimer m_timer;
};

#endif // TETRISBOARD_H
