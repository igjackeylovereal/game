#include "tetrispiece.h"

bool TetrisPiece::s_initialized = false;
QVector<QVector<QVector<QPoint>>> TetrisPiece::s_pieces;
QVector<QColor> TetrisPiece::s_colors;

void TetrisPiece::initPieces()
{
    if (s_initialized) return;
    s_initialized = true;

    s_pieces.resize(TypeCount);

    // I piece - Cyan
    s_pieces[I] = {
        // Rotation 0: horizontal
        { QPoint(0,0), QPoint(1,0), QPoint(2,0), QPoint(3,0) },
        // Rotation 1: vertical
        { QPoint(1,0), QPoint(1,1), QPoint(1,2), QPoint(1,3) },
        // Rotation 2: horizontal (same as 0)
        { QPoint(0,1), QPoint(1,1), QPoint(2,1), QPoint(3,1) },
        // Rotation 3: vertical (same as 1)
        { QPoint(2,0), QPoint(2,1), QPoint(2,2), QPoint(2,3) },
    };

    // O piece - Yellow (all rotations same)
    s_pieces[O] = {
        { QPoint(0,0), QPoint(1,0), QPoint(0,1), QPoint(1,1) },
        { QPoint(0,0), QPoint(1,0), QPoint(0,1), QPoint(1,1) },
        { QPoint(0,0), QPoint(1,0), QPoint(0,1), QPoint(1,1) },
        { QPoint(0,0), QPoint(1,0), QPoint(0,1), QPoint(1,1) },
    };

    // T piece - Purple
    s_pieces[T] = {
        { QPoint(1,0), QPoint(0,1), QPoint(1,1), QPoint(2,1) },
        { QPoint(0,0), QPoint(0,1), QPoint(1,1), QPoint(0,2) },
        { QPoint(0,0), QPoint(1,0), QPoint(2,0), QPoint(1,1) },
        { QPoint(1,0), QPoint(0,1), QPoint(1,1), QPoint(1,2) },
    };

    // S piece - Green
    s_pieces[S] = {
        { QPoint(1,0), QPoint(2,0), QPoint(0,1), QPoint(1,1) },
        { QPoint(0,0), QPoint(0,1), QPoint(1,1), QPoint(1,2) },
        { QPoint(1,0), QPoint(2,0), QPoint(0,1), QPoint(1,1) },
        { QPoint(0,0), QPoint(0,1), QPoint(1,1), QPoint(1,2) },
    };

    // Z piece - Red
    s_pieces[Z] = {
        { QPoint(0,0), QPoint(1,0), QPoint(1,1), QPoint(2,1) },
        { QPoint(1,0), QPoint(0,1), QPoint(1,1), QPoint(0,2) },
        { QPoint(0,0), QPoint(1,0), QPoint(1,1), QPoint(2,1) },
        { QPoint(1,0), QPoint(0,1), QPoint(1,1), QPoint(0,2) },
    };

    // J piece - Blue
    s_pieces[J] = {
        { QPoint(0,0), QPoint(0,1), QPoint(1,1), QPoint(2,1) },
        { QPoint(0,0), QPoint(1,0), QPoint(0,1), QPoint(0,2) },
        { QPoint(0,0), QPoint(1,0), QPoint(2,0), QPoint(2,1) },
        { QPoint(1,0), QPoint(1,1), QPoint(1,2), QPoint(0,2) },
    };

    // L piece - Orange
    s_pieces[L] = {
        { QPoint(2,0), QPoint(0,1), QPoint(1,1), QPoint(2,1) },
        { QPoint(0,0), QPoint(0,1), QPoint(0,2), QPoint(1,2) },
        { QPoint(0,0), QPoint(1,0), QPoint(2,0), QPoint(0,1) },
        { QPoint(0,0), QPoint(1,0), QPoint(1,1), QPoint(1,2) },
    };

    // Colors
    s_colors = {
        QColor(0,   240, 240),   // I - Cyan
        QColor(240, 240, 0),     // O - Yellow
        QColor(160, 0,   240),   // T - Purple
        QColor(0,   240, 0),     // S - Green
        QColor(240, 0,   0),     // Z - Red
        QColor(0,   0,   240),   // J - Blue
        QColor(240, 160, 0),     // L - Orange
    };
}

QColor TetrisPiece::colorForType(int type)
{
    initPieces();
    return s_colors.value(type, Qt::gray);
}

const QVector<QPoint> &TetrisPiece::blocksForType(int type, int rotation)
{
    initPieces();
    return s_pieces[type][rotation % rotationCount(type)];
}

int TetrisPiece::rotationCount(int type)
{
    Q_UNUSED(type);
    return 4;
}

QPoint TetrisPiece::spawnOffset(int type)
{
    // Spawn position offset: center top of board
    // Board is 10 wide, pieces spawn at column ~3-4
    if (type == I) return QPoint(3, -1);
    if (type == O) return QPoint(4, -1);
    return QPoint(3, -1);  // T, S, Z, J, L all fit in 3 wide spawn
}
