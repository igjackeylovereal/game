#ifndef TETRISPIECE_H
#define TETRISPIECE_H

#include <QColor>
#include <QPoint>
#include <QVector>

class TetrisPiece
{
public:
    enum Type { I = 0, O, T, S, Z, J, L, TypeCount };

    TetrisPiece() = default;

    static QColor colorForType(int type);
    static const QVector<QPoint> &blocksForType(int type, int rotation);
    static int rotationCount(int type);
    static QPoint spawnOffset(int type);

private:
    static void initPieces();
    static bool s_initialized;

    // pieces[type][rotation] = list of block offsets
    static QVector<QVector<QVector<QPoint>>> s_pieces;
    static QVector<QColor> s_colors;
};

#endif // TETRISPIECE_H
