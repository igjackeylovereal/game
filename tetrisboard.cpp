#include "tetrisboard.h"
#include "tetrispiece.h"

#include <QKeyEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>

TetrisBoard::TetrisBoard(QWidget *parent)
    : QWidget(parent)
    , m_state(Idle)
    , m_score(0)
    , m_level(1)
    , m_lines(0)
    , m_totalLinesCleared(0)
{
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(Width * 30 + 2, Height * 30 + 2);

    resetBoard();
    m_nextType = QRandomGenerator::global()->bounded(TetrisPiece::TypeCount);

    connect(&m_timer, &QTimer::timeout, this, &TetrisBoard::gameTick);
}

void TetrisBoard::resetBoard()
{
    for (int r = 0; r < Height; ++r) {
        for (int c = 0; c < Width; ++c) {
            m_board[r][c] = 0;
        }
    }
}

void TetrisBoard::startGame()
{
    resetBoard();
    m_score = 0;
    m_level = 1;
    m_lines = 0;
    m_totalLinesCleared = 0;

    emit scoreChanged(m_score);
    emit levelChanged(m_level);
    emit linesChanged(m_lines);

    spawnPiece();
    m_state = Playing;
    m_timer.start(dropSpeed());

    emit gameStarted();
    emit stateChanged(m_state);
    update();
}

void TetrisBoard::pauseGame()
{
    if (m_state != Playing) return;
    m_state = Paused;
    m_timer.stop();
    emit gamePaused(true);
    emit stateChanged(m_state);
    update();
}

void TetrisBoard::resumeGame()
{
    if (m_state != Paused) return;
    m_state = Playing;
    m_timer.start(dropSpeed());
    emit gamePaused(false);
    emit stateChanged(m_state);
    update();
}

void TetrisBoard::togglePause()
{
    if (m_state == Playing) {
        pauseGame();
    } else if (m_state == Paused) {
        resumeGame();
    }
}

void TetrisBoard::gameTick()
{
    if (m_state != Playing) return;

    if (!movePiece(0, 1)) {
        // Piece can't move down — lock it
        lockPiece();
        int cleared = clearLines();
        if (cleared > 0) {
            updateScore(cleared);
        }
        spawnPiece();
    }
    update();
}

void TetrisBoard::spawnPiece()
{
    m_curType = m_nextType;
    m_curRotation = 0;

    QPoint offset = TetrisPiece::spawnOffset(m_curType);
    m_curX = offset.x();
    m_curY = offset.y();

    m_nextType = QRandomGenerator::global()->bounded(TetrisPiece::TypeCount);
    emit nextPieceChanged(m_nextType);

    // Check if new piece collides immediately → game over
    if (isCollision(m_curType, m_curRotation, m_curX, m_curY)) {
        m_state = GameOver;
        m_timer.stop();
        emit gameOver();
        emit stateChanged(m_state);
    }
}

bool TetrisBoard::movePiece(int dx, int dy)
{
    if (!isCollision(m_curType, m_curRotation, m_curX + dx, m_curY + dy)) {
        m_curX += dx;
        m_curY += dy;
        return true;
    }
    return false;
}

bool TetrisBoard::rotatePiece()
{
    // O piece doesn't rotate
    if (m_curType == TetrisPiece::O) return false;

    int newRotation = (m_curRotation + 1) % 4;

    // Try basic rotation first
    if (!isCollision(m_curType, newRotation, m_curX, m_curY)) {
        m_curRotation = newRotation;
        return true;
    }

    // Wall kick: try offsets (-1,0), (1,0), (0,-1), (-2,0), (2,0)
    QVector<QPoint> kicks = {
        QPoint(-1, 0), QPoint(1, 0), QPoint(0, -1),
        QPoint(-2, 0), QPoint(2, 0)
    };

    for (const QPoint &kick : kicks) {
        if (!isCollision(m_curType, newRotation, m_curX + kick.x(), m_curY + kick.y())) {
            m_curRotation = newRotation;
            m_curX += kick.x();
            m_curY += kick.y();
            return true;
        }
    }

    return false;
}

void TetrisBoard::dropPiece()
{
    // Hard drop: move down until collision
    int dropDistance = 0;
    while (!isCollision(m_curType, m_curRotation, m_curX, m_curY + 1)) {
        m_curY++;
        dropDistance++;
    }
    m_score += dropDistance * 2;
    emit scoreChanged(m_score);

    // Immediately lock and process
    lockPiece();
    int cleared = clearLines();
    if (cleared > 0) {
        updateScore(cleared);
    }
    spawnPiece();
    update();
}

bool TetrisBoard::isCollision(int type, int rotation, int x, int y) const
{
    const QVector<QPoint> &blocks = TetrisPiece::blocksForType(type, rotation);
    for (const QPoint &b : blocks) {
        int bx = x + b.x();
        int by = y + b.y();

        // Check horizontal bounds
        if (bx < 0 || bx >= Width) return true;

        // Check bottom bound
        if (by >= Height) return true;

        // Allow pieces above the board (negative y)
        if (by < 0) continue;

        // Check collision with locked blocks
        if (m_board[by][bx] != 0) return true;
    }
    return false;
}

void TetrisBoard::lockPiece()
{
    const QVector<QPoint> &blocks = TetrisPiece::blocksForType(m_curType, m_curRotation);
    for (const QPoint &b : blocks) {
        int bx = m_curX + b.x();
        int by = m_curY + b.y();
        if (by >= 0 && by < Height && bx >= 0 && bx < Width) {
            m_board[by][bx] = m_curType + 1;  // +1 so 0 = empty
        }
    }
}

int TetrisBoard::clearLines()
{
    int cleared = 0;
    for (int r = Height - 1; r >= 0; --r) {
        bool full = true;
        for (int c = 0; c < Width; ++c) {
            if (m_board[r][c] == 0) {
                full = false;
                break;
            }
        }

        if (full) {
            cleared++;
            // Shift all rows above down by one
            for (int sr = r; sr > 0; --sr) {
                for (int c = 0; c < Width; ++c) {
                    m_board[sr][c] = m_board[sr - 1][c];
                }
            }
            // Clear top row
            for (int c = 0; c < Width; ++c) {
                m_board[0][c] = 0;
            }
            // Re-check this row since it now contains the row from above
            r++;
        }
    }
    return cleared;
}

void TetrisBoard::updateScore(int cleared)
{
    const int baseScores[] = { 0, 100, 300, 500, 800 };
    m_score += baseScores[cleared] * m_level;
    m_totalLinesCleared += cleared;
    m_lines += cleared;

    int newLevel = 1 + m_totalLinesCleared / 10;
    if (newLevel != m_level) {
        m_level = newLevel;
        m_timer.setInterval(dropSpeed());
        emit levelChanged(m_level);
    }

    emit scoreChanged(m_score);
    emit linesChanged(m_lines);
}

int TetrisBoard::dropSpeed() const
{
    return qMax(50, 800 - (m_level - 1) * 70);
}

// ─── Rendering ───────────────────────────────────────────────

void TetrisBoard::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int cellW = width() / Width;
    int cellH = height() / Height;
    int cellSize = qMin(cellW, cellH);

    int offsetX = (width()  - cellSize * Width)  / 2;
    int offsetY = (height() - cellSize * Height) / 2;

    // Background
    painter.fillRect(rect(), QColor(30, 30, 30));

    // Draw board background
    QRect boardRect(offsetX, offsetY, cellSize * Width, cellSize * Height);
    painter.fillRect(boardRect, QColor(10, 10, 10));

    // Draw grid lines
    painter.setPen(QPen(QColor(50, 50, 50), 1));
    for (int c = 0; c <= Width; ++c) {
        int x = offsetX + c * cellSize;
        painter.drawLine(x, offsetY, x, offsetY + Height * cellSize);
    }
    for (int r = 0; r <= Height; ++r) {
        int y = offsetY + r * cellSize;
        painter.drawLine(offsetX, y, offsetX + Width * cellSize, y);
    }

    auto drawBlock = [&](int col, int row, const QColor &color, bool ghost = false) {
        int x = offsetX + col * cellSize;
        int y = offsetY + row * cellSize;
        QRect r(x + 1, y + 1, cellSize - 2, cellSize - 2);

        if (ghost) {
            painter.setPen(QPen(color.lighter(60), 1));
            painter.setBrush(Qt::NoBrush);
        } else {
            painter.setPen(Qt::NoPen);
            // Gradient effect for 3D look
            QLinearGradient grad(x, y, x, y + cellSize);
            grad.setColorAt(0.0, color.lighter(140));
            grad.setColorAt(0.4, color);
            grad.setColorAt(1.0, color.darker(140));
            painter.setBrush(grad);
        }
        painter.drawRect(r);

        // Inner highlight
        if (!ghost) {
            painter.setPen(QPen(color.lighter(180), 1));
            painter.drawLine(x + 2, y + 2, x + cellSize - 3, y + 2);
            painter.drawLine(x + 2, y + 2, x + 2, y + cellSize - 3);
        }
    };

    // Draw locked blocks
    for (int r = 0; r < Height; ++r) {
        for (int c = 0; c < Width; ++c) {
            if (m_board[r][c] != 0) {
                QColor color = TetrisPiece::colorForType(m_board[r][c] - 1);
                drawBlock(c, r, color);
            }
        }
    }

    // Draw ghost piece (shadow) and current piece
    if (m_state == Playing || m_state == Paused) {
        // Calculate ghost position
        int ghostY = m_curY;
        while (!isCollision(m_curType, m_curRotation, m_curX, ghostY + 1)) {
            ghostY++;
        }

        QColor curColor = TetrisPiece::colorForType(m_curType);
        const QVector<QPoint> &blocks = TetrisPiece::blocksForType(m_curType, m_curRotation);

        // Draw ghost
        if (ghostY != m_curY) {
            for (const QPoint &b : blocks) {
                int bx = m_curX + b.x();
                int by = ghostY + b.y();
                if (by >= 0 && bx >= 0 && bx < Width) {
                    drawBlock(bx, by, curColor, true);
                }
            }
        }

        // Draw active piece
        for (const QPoint &b : blocks) {
            int bx = m_curX + b.x();
            int by = m_curY + b.y();
            if (by >= 0 && bx >= 0 && bx < Width) {
                drawBlock(bx, by, curColor);
            }
        }
    }

    // Overlay messages
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(14);
    font.setBold(true);
    painter.setFont(font);

    if (m_state == Idle) {
        painter.fillRect(rect(), QColor(0, 0, 0, 150));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("按 [开始] 开始游戏"));
    } else if (m_state == Paused) {
        painter.fillRect(rect(), QColor(0, 0, 0, 150));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("已暂停\n按 P 继续"));
    } else if (m_state == GameOver) {
        painter.fillRect(rect(), QColor(0, 0, 0, 180));
        QString text = QStringLiteral("游戏结束\n得分：%1\n按 [开始] 重新开始").arg(m_score);
        painter.drawText(rect(), Qt::AlignCenter, text);
    }
}

// ─── Keyboard Input ──────────────────────────────────────────

void TetrisBoard::keyPressEvent(QKeyEvent *event)
{
    if (m_state != Playing) {
        QWidget::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_A:
        movePiece(-1, 0);
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        movePiece(1, 0);
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        if (movePiece(0, 1)) {
            m_score += 1;  // soft drop bonus
            emit scoreChanged(m_score);
        }
        break;
    case Qt::Key_Up:
    case Qt::Key_W:
        rotatePiece();
        break;
    case Qt::Key_Space:
        dropPiece();
        break;
    case Qt::Key_P:
        togglePause();
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }

    update();
}
