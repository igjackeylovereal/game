#ifndef TETRISWINDOW_H
#define TETRISWINDOW_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "tetrisboard.h"

class NextPiecePreview : public QWidget
{
    Q_OBJECT

public:
    explicit NextPiecePreview(QWidget *parent = nullptr);
    void setPieceType(int type);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_pieceType;
};

class TetrisWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TetrisWindow(QWidget *parent = nullptr);
    ~TetrisWindow() = default;

private slots:
    void onStartClicked();
    void onPauseClicked();
    void updateStatusFromState(TetrisBoard::GameState state);

private:
    void setupUi();

    TetrisBoard      *m_board;
    NextPiecePreview *m_nextPreview;
    QLabel           *m_scoreLabel;
    QLabel           *m_levelLabel;
    QLabel           *m_linesLabel;
    QLabel           *m_statusLabel;
    QPushButton      *m_startBtn;
    QPushButton      *m_pauseBtn;
};

#endif // TETRISWINDOW_H
