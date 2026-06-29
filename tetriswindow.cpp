#include "tetriswindow.h"
#include "tetrispiece.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
#include <QVBoxLayout>

// ─── NextPiecePreview ────────────────────────────────────────

NextPiecePreview::NextPiecePreview(QWidget *parent)
    : QWidget(parent)
    , m_pieceType(0)
{
    setFixedSize(120, 120);
    setStyleSheet("background-color: #0a0a0a; border: 1px solid #333;");
}

void NextPiecePreview::setPieceType(int type)
{
    m_pieceType = type;
    update();
}

void NextPiecePreview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QVector<QPoint> &blocks = TetrisPiece::blocksForType(m_pieceType, 0);
    QColor color = TetrisPiece::colorForType(m_pieceType);

    int cellSize = 22;
    int totalW = 4 * cellSize;
    int totalH = 4 * cellSize;
    int offsetX = (width()  - totalW) / 2;
    int offsetY = (height() - totalH) / 2;

    for (const QPoint &b : blocks) {
        int x = offsetX + b.x() * cellSize;
        int y = offsetY + b.y() * cellSize;
        QRect r(x + 1, y + 1, cellSize - 2, cellSize - 2);

        QLinearGradient grad(x, y, x, y + cellSize);
        grad.setColorAt(0.0, color.lighter(140));
        grad.setColorAt(0.4, color);
        grad.setColorAt(1.0, color.darker(140));
        painter.setPen(Qt::NoPen);
        painter.setBrush(grad);
        painter.drawRect(r);

        painter.setPen(QPen(color.lighter(180), 1));
        painter.drawLine(x + 2, y + 2, x + cellSize - 3, y + 2);
        painter.drawLine(x + 2, y + 2, x + 2, y + cellSize - 3);
    }
}

// ─── TetrisWindow ─────────────────────────────────────────────

TetrisWindow::TetrisWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void TetrisWindow::setupUi()
{
    setWindowTitle(QStringLiteral("俄罗斯方块 - Tetris"));
    setFixedSize(520, 680);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(12);

    // ── Left: Game Board ──
    m_board = new TetrisBoard(this);
    m_board->setFixedSize(300, 600);
    mainLayout->addWidget(m_board);

    // ── Right: Info Panel ──
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(10);

    // Next piece group
    QGroupBox *nextGroup = new QGroupBox(QStringLiteral("下一块"), this);
    nextGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444; "
                             "padding-top: 16px; margin-top: 8px; }");
    QVBoxLayout *nextLayout = new QVBoxLayout(nextGroup);
    m_nextPreview = new NextPiecePreview(this);
    nextLayout->addWidget(m_nextPreview, 0, Qt::AlignCenter);
    rightLayout->addWidget(nextGroup);

    // Stats group
    QGroupBox *statsGroup = new QGroupBox(QStringLiteral("统计"), this);
    statsGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444; "
                              "padding-top: 16px; margin-top: 8px; }");
    QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);

    auto makeStatLabel = [this](const QString &prefix) {
        QLabel *label = new QLabel(prefix + "0", this);
        label->setStyleSheet("color: #ccc; font-size: 13px;");
        return label;
    };

    m_scoreLabel = makeStatLabel(QStringLiteral("分数："));
    m_levelLabel = makeStatLabel(QStringLiteral("等级："));
    m_linesLabel = makeStatLabel(QStringLiteral("行数："));

    statsLayout->addWidget(m_scoreLabel);
    statsLayout->addWidget(m_levelLabel);
    statsLayout->addWidget(m_linesLabel);
    rightLayout->addWidget(statsGroup);

    // Spacer
    rightLayout->addStretch();

    // Buttons
    m_startBtn = new QPushButton(QStringLiteral("开始游戏"), this);
    m_startBtn->setFixedHeight(36);
    m_startBtn->setStyleSheet(
        "QPushButton { background-color: #2d7d2d; color: white; border: none; "
        "border-radius: 4px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #3a9a3a; }"
        "QPushButton:pressed { background-color: #1f5f1f; }");

    m_pauseBtn = new QPushButton(QStringLiteral("暂停"), this);
    m_pauseBtn->setFixedHeight(36);
    m_pauseBtn->setEnabled(false);
    m_pauseBtn->setStyleSheet(
        "QPushButton { background-color: #555; color: white; border: none; "
        "border-radius: 4px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #666; }"
        "QPushButton:pressed { background-color: #444; }"
        "QPushButton:disabled { background-color: #333; color: #666; }");

    rightLayout->addWidget(m_startBtn);
    rightLayout->addWidget(m_pauseBtn);

    mainLayout->addLayout(rightLayout);

    // ── Style the main window ──
    setStyleSheet("background-color: #1a1a2e;");

    // ── Connections ──
    connect(m_board, &TetrisBoard::scoreChanged, this, [this](int score) {
        m_scoreLabel->setText(QStringLiteral("分数：%1").arg(score));
    });
    connect(m_board, &TetrisBoard::levelChanged, this, [this](int level) {
        m_levelLabel->setText(QStringLiteral("等级：%1").arg(level));
    });
    connect(m_board, &TetrisBoard::linesChanged, this, [this](int lines) {
        m_linesLabel->setText(QStringLiteral("行数：%1").arg(lines));
    });
    connect(m_board, &TetrisBoard::nextPieceChanged, this, [this](int type) {
        m_nextPreview->setPieceType(type);
    });
    connect(m_board, &TetrisBoard::stateChanged, this, &TetrisWindow::updateStatusFromState);

    connect(m_startBtn, &QPushButton::clicked, this, &TetrisWindow::onStartClicked);
    connect(m_pauseBtn, &QPushButton::clicked, this, &TetrisWindow::onPauseClicked);

    // Initialize next piece preview
    m_nextPreview->setPieceType(m_board->nextPieceType());
}

void TetrisWindow::onStartClicked()
{
    m_board->startGame();
    m_board->setFocus();
}

void TetrisWindow::onPauseClicked()
{
    m_board->togglePause();
    m_board->setFocus();
}

void TetrisWindow::updateStatusFromState(TetrisBoard::GameState state)
{
    switch (state) {
    case TetrisBoard::Idle:
        m_startBtn->setText(QStringLiteral("开始游戏"));
        m_startBtn->setEnabled(true);
        m_pauseBtn->setText(QStringLiteral("暂停"));
        m_pauseBtn->setEnabled(false);
        break;
    case TetrisBoard::Playing:
        m_startBtn->setText(QStringLiteral("重新开始"));
        m_startBtn->setEnabled(false);
        m_pauseBtn->setText(QStringLiteral("暂停"));
        m_pauseBtn->setEnabled(true);
        break;
    case TetrisBoard::Paused:
        m_startBtn->setEnabled(false);
        m_pauseBtn->setText(QStringLiteral("继续"));
        m_pauseBtn->setEnabled(true);
        break;
    case TetrisBoard::GameOver:
        m_startBtn->setText(QStringLiteral("重新开始"));
        m_startBtn->setEnabled(true);
        m_pauseBtn->setText(QStringLiteral("暂停"));
        m_pauseBtn->setEnabled(false);
        break;
    }
}
