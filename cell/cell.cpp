#include "cell.h"
#include "./ui_cell.h"
#include "PatternSlot.h"
#include <QRandomGenerator>
#include <QDebug>
#include <algorithm>

// 常量定义
const int cell::GAME_WIDTH;
const int cell::GAME_HEIGHT;
const int cell::CELL_WIDTH;
const int cell::CELL_HEIGHT;
const int cell::GRID_ROWS;
const int cell::GRID_COLS;
const int cell::FLAG_COUNT;

cell::cell(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::cell)
    , gameOver(false)
    , winner("")
    , isPaused(false)
    , generationCount(0)
    , blueScore(0)
    , redScore(0)
{
    ui->setupUi(this);

    // 设置窗口大小并固定
    this->setGeometry(QRect(0, 0, 1700, 990));
    this->setFixedSize(1700, 990);

    // 加载图片资源（请确保路径正确）
    blueCellImg.load(":/images/Image/blue.bmp");
    redCellImg.load(":/images/Image/red.bmp");
    purpleCellImg.load(":/images/Image/purple.bmp");
    flagImg.load(":/images/Image/flag.bmp");
    backgroundImg.load(":/images/Image/background.bmp");

    // 初始化游戏基础数据
    initGrid();          // 全空网格
    initFlags();         // 初始化旗帜
    initCellPatterns();  // 初始化细胞组合模板
    initEnergy();        // 初始化能量

    // 初始化卡槽管理器
    slotManager = new PatternSlot(this);
    slotManager->initAllPatterns(this);

    // 设置按钮位置
    pauseButtonRect = QRect(1600, 10, 30, 30);    // 右上角暂停按钮
    rotateButtonRect = QRect(20, 10, 20, 20);     // 左上角旋转按钮

    // 设置定时器（每300ms更新一次）
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &cell::updateGameState);
    gameTimer->start(300); // 数值越小速度越快
}

cell::~cell()
{
    delete ui;
}

// 初始化细胞网格：全场空细胞
void cell::initGrid()
{
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            currentGrid[i][j] = EMPTY;
            nextGrid[i][j] = EMPTY;
        }
    }
}

// 初始化旗帜位置：左侧5个蓝旗，右侧5个红旗
void cell::initFlags()
{
    // 蓝色旗帜（左侧固定位置）
    for (int i = 0; i < FLAG_COUNT; ++i) {
        blueFlags[i].x = 0;
        blueFlags[i].y = 14 + 13 * i;
        blueFlags[i].isAlive = true;
    }

    // 红色旗帜（右侧固定位置）
    for (int i = 0; i < FLAG_COUNT; ++i) {
        redFlags[i].x = 199;
        redFlags[i].y = 14 + 13 * i;
        redFlags[i].isAlive = true;
    }
}

// 初始化细胞能量
void cell::initEnergy()
{
    blueEnergy = INITIAL_ENERGY;
    redEnergy = INITIAL_ENERGY;
}

// 旋转组合坐标
QVector<QPoint> cell::rotatePattern(const QVector<QPoint>& original, RotationDirection dir)
{
    QVector<QPoint> rotated;
    for (const QPoint& p : original) {
        int x = p.x();
        int y = p.y();

        switch (dir) {
        case UP_RIGHT:    // 右上旋转90°
            rotated.append(QPoint(-y, x));
            break;
        case DOWN_RIGHT:  // 右下（默认方向）
            rotated.append(QPoint(x, y));
            break;
        case UP_LEFT:     // 左上180°翻转
            rotated.append(QPoint(-x, -y));
            break;
        case DOWN_LEFT:   // 左下旋转270°
            rotated.append(QPoint(y, -x));
            break;
        }
    }
    return rotated;
}

// 初始化细胞组合模板（含优先级）
void cell::initCellPatterns()
{
    // ------------------- 新增：蓝方专用 single_cell 单个细胞 -------------------
    CellPattern singleCell;
    singleCell.name = "single_cell";
    singleCell.cellPositions = {QPoint(0, 0)};
    singleCell.energyCost = 2;
    singleCell.priority = 1;
    blueStillLifePatterns.prepend(singleCell); // 放在最前面

    // ------------------- 蓝方稳定型组合（优先级2） -------------------
    // 1. 方块（2x2）
    CellPattern block;
    block.name = "方块";
    block.cellPositions = {QPoint(-1, -1), QPoint(-1, 0), QPoint(0, -1), QPoint(0, 0)};
    block.energyCost = block.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
    block.priority = 2;
    blueStillLifePatterns.append(block);

    // 2. 船型
    CellPattern boat;
    boat.name = "船型";
    boat.cellPositions = {QPoint(-1, -1), QPoint(-1, 0), QPoint(0, -1), QPoint(0, 1), QPoint(1, 0)};
    boat.energyCost = boat.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
    boat.priority = 2;
    blueStillLifePatterns.append(boat);

    // 补充稳定型组合
    for (int i = 2; i < 10; ++i) {
        CellPattern pattern;
        pattern.name = QString("稳定型%1").arg(i+1);
        for (int j = 0; j < 3 + i % 4; ++j) {
            pattern.cellPositions.append(QPoint(j%3 -1, j/3 -1));
        }
        pattern.energyCost = pattern.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
        pattern.priority = 2;
        blueStillLifePatterns.append(pattern);
    }

    // ------------------- 蓝方振荡型组合（优先级2） -------------------
    // 1. 闪烁器
    CellPattern blinker;
    blinker.name = "闪烁器";
    blinker.cellPositions = {QPoint(-1, 0), QPoint(0, 0), QPoint(1, 0)};
    blinker.energyCost = blinker.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
    blinker.priority = 2;
    blueOscillatorPatterns.append(blinker);

    // 2. 钟摆
    CellPattern toad;
    toad.name = "钟摆";
    toad.cellPositions = {QPoint(-1, 0), QPoint(0, 0), QPoint(1, 0), QPoint(0, 1), QPoint(1, 1), QPoint(2, 1)};
    toad.energyCost = toad.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
    toad.priority = 2;
    blueOscillatorPatterns.append(toad);

    // 补充振荡型
    for (int i = 2; i < 10; ++i) {
        CellPattern pattern;
        pattern.name = QString("振荡型%1").arg(i+1);
        for (int j = 0; j < 4 + i % 5; ++j) {
            pattern.cellPositions.append(QPoint(j%4 -2, j/4 -1));
        }
        pattern.energyCost = pattern.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
        pattern.priority = 2;
        blueOscillatorPatterns.append(pattern);
    }

    // ------------------- 蓝方飞船型组合（优先级3） -------------------
    // 1. 滑翔机
    CellPattern glider;
    glider.name = "滑翔机";
    glider.cellPositions = {QPoint(0, -1), QPoint(1, 0), QPoint(-1, 1), QPoint(0, 1), QPoint(1, 1)};
    glider.energyCost = glider.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
    glider.priority = 3;
    blueSpaceshipPatterns.append(glider);

    // 补充飞船型
    for (int i = 1; i < 5; ++i) {
        CellPattern pattern;
        pattern.name = QString("飞船型%1").arg(i+1);
        for (int j = 0; j < 5 + i % 3; ++j) {
            pattern.cellPositions.append(QPoint(j%5 -2, j/5 -1));
        }
        pattern.energyCost = pattern.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
        pattern.priority = 3;
        blueSpaceshipPatterns.append(pattern);
    }

    // ------------------- 蓝方枪型组合（优先级4） -------------------
    // 1. 高斯珀滑翔机枪（简化版）
    CellPattern gun;
    gun.name = "高斯珀滑翔机枪";
    gun.cellPositions = {QPoint(-4, -3), QPoint(-3, -3), QPoint(-4, -2), QPoint(-3, -2),
                         QPoint(0, -1), QPoint(1, -1), QPoint(2, -1),
                         QPoint(-1, 0), QPoint(3, 0),
                         QPoint(-2, 1), QPoint(4, 1),
                         QPoint(-2, 2), QPoint(4, 2),
                         QPoint(-1, 3), QPoint(0, 3), QPoint(1, 3), QPoint(2, 3),
                         QPoint(-3, 4), QPoint(-4, 4),
                         QPoint(-3, 5), QPoint(-4, 5)};
    gun.energyCost = gun.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
    gun.priority = 4;
    blueGunPatterns.append(gun);

    // 补充枪型
    for (int i = 1; i < 5; ++i) {
        CellPattern pattern;
        pattern.name = QString("枪型%1").arg(i+1);
        for (int j = 0; j < 10 + i % 6; ++j) {
            pattern.cellPositions.append(QPoint(j%6 -3, j/6 -2));
        }
        pattern.energyCost = pattern.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
        pattern.priority = 4;
        blueGunPatterns.append(pattern);
    }

    // 红方直接复制蓝方模板
    redStillLifePatterns = blueStillLifePatterns;
    redOscillatorPatterns = blueOscillatorPatterns;
    redSpaceshipPatterns = blueSpaceshipPatterns;
    redGunPatterns = blueGunPatterns;

    // 红方删除 single_cell
    if (!redStillLifePatterns.isEmpty() && redStillLifePatterns.first().name == "single_cell") {
        redStillLifePatterns.removeFirst();
    }
}

// 按优先级降序获取组合
QVector<CellPattern> cell::getPatternsByPriority(CellType cellType)
{
    QVector<CellPattern> allPatterns;

    if (cellType == BLUE_CELL) {
        allPatterns << blueGunPatterns << blueSpaceshipPatterns
                    << blueOscillatorPatterns << blueStillLifePatterns;
    } else if (cellType == RED_CELL) {
        allPatterns << redGunPatterns << redSpaceshipPatterns
                    << redOscillatorPatterns << redStillLifePatterns;
    }

    // 按优先级从高到低排序
    std::sort(allPatterns.begin(), allPatterns.end(),
              [](const CellPattern& a, const CellPattern& b) {
                  return a.priority > b.priority;
              });

    return allPatterns;
}

// 统计周围8邻域所有活细胞（红蓝紫）
int cell::countAllAliveCells(int x, int y)
{
    int count = 0;

    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue; // 跳过自身

            int nx = x + dx;
            int ny = y + dy;

            // 边界检查
            if (nx >= 0 && nx < GRID_ROWS && ny >= 0 && ny < GRID_COLS) {
                if (currentGrid[nx][ny] != EMPTY) {
                    count++;
                }
            }
        }
    }
    return count;
}

// 统计周围指定颜色的活细胞
int cell::countColorAliveCells(int x, int y, CellType color)
{
    int count = 0;

    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;

            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < GRID_ROWS && ny >= 0 && ny < GRID_COLS) {
                if (currentGrid[nx][ny] == color) {
                    count++;
                }
            }
        }
    }
    return count;
}

// 统计紫色区（75~124列）己方细胞数量
int cell::countOwnCellsInPurpleZone(CellType color)
{
    int count = 0;
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 75; j <= 124; ++j) {
            if (currentGrid[i][j] == color) {
                count++;
            }
        }
    }
    return count;
}

// 更新双方能量值
void cell::updateEnergy()
{
    // 每次迭代基础能量增长
    blueEnergy += ENERGY_PER_ITERATION;
    redEnergy += ENERGY_PER_ITERATION;

    // 紫色区细胞加成
    blueEnergy += countOwnCellsInPurpleZone(BLUE_CELL) * ENERGY_PER_CELL;
    redEnergy += countOwnCellsInPurpleZone(RED_CELL) * ENERGY_PER_CELL;

    // 能量不能为负
    blueEnergy = qMax(0, blueEnergy);
    redEnergy = qMax(0, redEnergy);
}

// 检查细胞组合放置是否合法（带旋转）
bool cell::isPlacementValid(int centerX, int centerY, const CellPattern& pattern, CellType cellType, RotationDirection dir)
{
    // 阵营区域限制
    int minCol = 0, maxCol = 0;
    if (cellType == BLUE_CELL) {
        minCol = 0;
        maxCol = 74; // 蓝方区域
    } else if (cellType == RED_CELL) {
        minCol = 125;
        maxCol = 199; // 红方区域
    }

    // 获取旋转后坐标
    QVector<QPoint> rotatedPos = rotatePattern(pattern.cellPositions, dir);

    // 检查每个细胞位置
    for (const QPoint& p : rotatedPos) {
        int x = centerX + p.x();
        int y = centerY + p.y();

        // 越界判断
        if (x < 0 || x >= GRID_ROWS || y < 0 || y >= GRID_COLS) {
            return false;
        }

        // 阵营区域判断
        if (y < minCol || y > maxCol) {
            return false;
        }

        // 不能与非己方细胞重叠
        CellType existing = currentGrid[x][y];
        if (existing != EMPTY && existing != cellType) {
            return false;
        }
    }

    return true;
}

// 放置细胞组合（扣除能量、修改网格）
bool cell::placeCellPattern(int centerX, int centerY, const CellPattern& pattern, CellType cellType, RotationDirection dir)
{
    // 能量检查
    if ((cellType == BLUE_CELL && blueEnergy < pattern.energyCost) ||
        (cellType == RED_CELL && redEnergy < pattern.energyCost)) {
        qDebug() << "能量不足：" << (cellType==BLUE_CELL?"蓝方":"红方") << "需要" << pattern.energyCost;
        return false;
    }

    // 位置合法性检查
    if (!isPlacementValid(centerX, centerY, pattern, cellType, dir)) {
        qDebug() << "位置非法：" << centerX << "," << centerY;
        return false;
    }

    // 旋转后放置
    QVector<QPoint> rotatedPos = rotatePattern(pattern.cellPositions, dir);
    for (const QPoint& p : rotatedPos) {
        int x = centerX + p.x();
        int y = centerY + p.y();
        currentGrid[x][y] = cellType;
    }

    // 扣除能量
    if (cellType == BLUE_CELL) {
        blueEnergy -= pattern.energyCost;
        qDebug() << "蓝方放置成功，剩余能量：" << blueEnergy;
    } else if (cellType == RED_CELL) {
        redEnergy -= pattern.energyCost;
        qDebug() << "红方放置成功，剩余能量：" << redEnergy;
    }

    return true;
}

// 随机自动放置细胞组合（红蓝双方逻辑）
void cell::randomPlacePatterns()
{
    // ------------------- 蓝方：按优先级 + 固定朝向顺序 -------------------//蓝方不再自动放置


    // ------------------- 红方：概率选择 + 等概率朝向 + 多数量放置 -------------------
    if (redEnergy > 10) {
        // 随机方向：左上 / 左下 各50%
        RotationDirection redDir = (QRandomGenerator::global()->bounded(2) == 0) ? UP_LEFT : DOWN_LEFT;

        int randVal = QRandomGenerator::global()->bounded(100);
        QVector<CellPattern> selectedList;
        bool placeNothing = false;

        // 按能量区间分配概率
        if (redEnergy < 500) {
            if (randVal < 40)      selectedList = redGunPatterns;
            else if (randVal < 65) selectedList = redSpaceshipPatterns;
            else if (randVal < 75) selectedList = redOscillatorPatterns;
            else if (randVal < 90) selectedList = redStillLifePatterns;
            else                   placeNothing = true;
        } else {
            if (randVal < 40)      selectedList = redGunPatterns;
            else if (randVal < 70) selectedList = redSpaceshipPatterns;
            else if (randVal < 84) selectedList = redOscillatorPatterns;
            else if (randVal < 99) selectedList = redStillLifePatterns;
            else                   placeNothing = true;
        }

        if (placeNothing || selectedList.isEmpty()) return;

        // 随机选一个模板
        CellPattern pattern = selectedList[QRandomGenerator::global()->bounded(selectedList.size())];

        // 高能量时一次放多个：前三位数字之和
        int placeCount = 1;
        if (redEnergy >= 500) {
            int n = redEnergy;
            int a = n / 1000 % 10;
            int b = n / 100 % 10;
            int c = n / 10 % 10;
            placeCount = a + b + c;
            if (placeCount < 1) placeCount = 1;
        }

        // 批量尝试放置
        int maxAttempts = 50;
        for (int i = 0; i < placeCount; ++i) {
            if (redEnergy < pattern.energyCost) break;

            bool ok = false;
            int tryCnt = 0;
            while (tryCnt++ < maxAttempts && !ok) {
                int cx = QRandomGenerator::global()->bounded(GRID_ROWS);
                int cy = QRandomGenerator::global()->bounded(125, 200); // 红方区域
                ok = placeCellPattern(cx, cy, pattern, RED_CELL, redDir);
            }
        }
    }
}

// 检查旗帜是否被对方占领
void cell::checkFlagOccupation()
{
    // 蓝旗被红/紫占领
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (blueFlags[i].isAlive) {
            int x = blueFlags[i].x;
            int y = blueFlags[i].y;
            if (currentGrid[y][x] == RED_CELL || currentGrid[y][x] == PURPLE_CELL) {
                blueFlags[i].isAlive = false;
            }
        }
    }

    // 红旗被蓝/紫占领
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (redFlags[i].isAlive) {
            int x = redFlags[i].x;
            int y = redFlags[i].y;
            if (currentGrid[y][x] == BLUE_CELL || currentGrid[y][x] == PURPLE_CELL) {
                redFlags[i].isAlive = false;
            }
        }
    }
}

// 判断游戏结束：一方旗帜全灭 或 达到10000代
bool cell::checkGameOver()
{
    // 条件1：旗帜全灭
    bool allBlueFlagsGone = true;
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (blueFlags[i].isAlive) {
            allBlueFlagsGone = false;
            break;
        }
    }

    bool allRedFlagsGone = true;
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (redFlags[i].isAlive) {
            allRedFlagsGone = false;
            break;
        }
    }

    // 条件2：达到10000代
    if (generationCount >= 10000) {
        if (blueScore > redScore) {
            winner = "蓝色方（分数更高）";
        } else if (redScore > blueScore) {
            winner = "红色方（分数更高）";
        } else {
            winner = "平局";
        }
        return true;
    }

    if (allBlueFlagsGone) {
        winner = "红色方";
        return true;
    }
    if (allRedFlagsGone) {
        winner = "蓝色方";
        return true;
    }

    return false;
}

// 计算下一代细胞状态（康威规则+颜色生成规则）
void cell::calculateNextGeneration()
{
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            CellType currentType = currentGrid[i][j];
            int allAlive = countAllAliveCells(i, j);
            int blueAlive = countColorAliveCells(i, j, BLUE_CELL);
            int redAlive = countColorAliveCells(i, j, RED_CELL);

            if (currentType != EMPTY) {
                // 活细胞：周围2~3个存活，否则死亡
                nextGrid[i][j] = (allAlive == 2 || allAlive == 3) ? currentType : EMPTY;
            } else {
                // 死细胞复活规则
                bool blueBorn = (blueAlive == 3);
                bool redBorn = (redAlive == 3);

                if (blueBorn && redBorn)
                    nextGrid[i][j] = PURPLE_CELL;
                else if (blueBorn)
                    nextGrid[i][j] = BLUE_CELL;
                else if (redBorn)
                    nextGrid[i][j] = RED_CELL;
                else
                    nextGrid[i][j] = EMPTY;
            }
        }
    }

    // 覆盖当前代
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            currentGrid[i][j] = nextGrid[i][j];
        }
    }

    // 检查旗帜与游戏结束
    checkFlagOccupation();
    if (!gameOver) {
        gameOver = checkGameOver();
        if (gameOver) {
            gameTimer->stop();
            QMessageBox::information(this, "游戏结束", winner + "获胜！");
        }
    }
}

// 绘制所有细胞
void cell::drawCells(QPainter &painter)
{
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            int x = 100 + j * CELL_WIDTH;
            int y = 30 + i * CELL_HEIGHT;

            switch (currentGrid[i][j]) {
            case BLUE_CELL:
                painter.drawPixmap(x, y, CELL_WIDTH, CELL_HEIGHT, blueCellImg);
                break;
            case RED_CELL:
                painter.drawPixmap(x, y, CELL_WIDTH, CELL_HEIGHT, redCellImg);
                break;
            case PURPLE_CELL:
                painter.drawPixmap(x, y, CELL_WIDTH, CELL_HEIGHT, purpleCellImg);
            case EMPTY:
            default:
                break;
            }
        }
    }
}

// 绘制旗帜
void cell::drawFlags(QPainter &painter)
{
    // 蓝旗
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (blueFlags[i].isAlive) {
            int x = 100 + blueFlags[i].x * CELL_WIDTH;
            int y = 30 + blueFlags[i].y * CELL_HEIGHT;
            painter.drawPixmap(x, y, CELL_WIDTH, CELL_HEIGHT, flagImg);
        }
    }
    // 红旗
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (redFlags[i].isAlive) {
            int x = 100 + redFlags[i].x * CELL_WIDTH;
            int y = 30 + redFlags[i].y * CELL_HEIGHT;
            painter.drawPixmap(x, y, CELL_WIDTH, CELL_HEIGHT, flagImg);
        }
    }
}

// 主绘制事件
void cell::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 外框与背景
    painter.drawRect(0, 0, 1700, 990);
    painter.drawRect(100, 30, GAME_WIDTH, GAME_HEIGHT);
    painter.drawPixmap(100, 30, GAME_WIDTH, GAME_HEIGHT, backgroundImg);

    // 绘制游戏元素
    drawCells(painter);
    drawFlags(painter);

    // 绘制能量文本
    painter.setPen(Qt::blue);
    painter.drawText(20, 50, QString("蓝方能量: %1").arg(blueEnergy));
    painter.setPen(Qt::red);
    painter.drawText(20, 70, QString("红方能量: %1").arg(redEnergy));

    // 新增UI绘制
    drawPatternSlots(painter);
    drawPauseButton(painter);
    drawGenerationInfo(painter);
    drawScoreInfo(painter); // 绘制分数
}

// 定时器驱动的游戏主循环
void cell::updateGameState()
{
    // 未结束且未暂停时才迭代
    if (!gameOver && !isPaused) {
        generationCount++;       // 代数自增
        updateEnergy();          // 更新能量
        updateScore();           // 更新分数
        randomPlacePatterns();   // 随机放置
        calculateNextGeneration();// 计算下一代
        update();                // 重绘
    }
}

// 绘制左侧细胞卡槽（支持滚动）
void cell::drawPatternSlots(QPainter& painter)
{
    painter.setPen(QColor(210, 180, 140)); // 浅棕色边框
    painter.setBrush(Qt::transparent);

    int startY = 40 + slotManager->scrollOffset;
    int slotCount = slotManager->allBluePatterns.size();
    int totalHeight = slotCount * (SLOT_HEIGHT + SLOT_MARGIN);

    // 裁剪区域，只显示可视区域
    painter.save();
    painter.setClipRect(SLOT_MARGIN, 40, SLOT_WIDTH, height() - 80);

    for (int i = 0; i < slotCount; ++i) {
        int x = SLOT_MARGIN;
        int y = startY + i * (SLOT_HEIGHT + SLOT_MARGIN);
        QRect slotRect(x, y, SLOT_WIDTH, SLOT_HEIGHT);

        // 只绘制可见区域
        if (slotRect.bottom() < 40 || slotRect.top() > height() - 40)
            continue;

        // 选中卡片高亮
        if (slotManager->selectedIndex == i) {
            painter.setBrush(QColor(255, 255, 0, 60));
        } else {
            painter.setBrush(Qt::transparent);
        }
        painter.drawRect(slotRect);

        // 绘制细胞缩略图
        const CellPattern& pat = slotManager->allBluePatterns[i];
        QVector<QPoint> pts = rotatePattern(pat.cellPositions, DOWN_RIGHT);
        int cx = slotRect.center().x();
        int cy = slotRect.center().y();

        painter.setPen(Qt::blue);
        painter.setBrush(Qt::blue);
        for (const QPoint& p : pts) {
            int px = cx + p.y() * 3;
            int py = cy + p.x() * 3;
            painter.drawEllipse(px - 2, py - 2, 4, 4);
        }
    }
    painter.restore();

    // 绘制旋转按钮（R）
    painter.setPen(Qt::black);
    painter.setBrush(QColor(220, 220, 220));
    painter.drawRect(rotateButtonRect);
    painter.drawText(rotateButtonRect, Qt::AlignCenter, "R");
}

// 绘制右上角暂停/继续按钮
void cell::drawPauseButton(QPainter& painter)
{
    painter.setPen(Qt::black);
    // 绿色=暂停，红色=运行
    painter.setBrush(isPaused ? QColor(100, 255, 100) : QColor(255, 120, 120));
    painter.drawRect(pauseButtonRect);
    // 显示图标：▶ / ||
    painter.drawText(pauseButtonRect, Qt::AlignCenter, isPaused ? "▶" : "||");
}

// 绘制细胞代数
void cell::drawGenerationInfo(QPainter& painter)
{
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(1400, 50, QString("代数: %1").arg(generationCount));
}

// 寻找距离最近的可放置位置
bool cell::findNearestValidPos(int& cx, int& cy, const CellPattern& pat, RotationDirection dir)
{
    int bestX = cx, bestY = cy;
    int minDist = 99999;
    bool found = false;

    // 遍历蓝方全区域搜索
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < 75; ++j) {
            if (isPlacementValid(i, j, pat, BLUE_CELL, dir)) {
                int dist = (i - cx) * (i - cx) + (j - cy) * (j - cy);
                if (dist < minDist) {
                    minDist = dist;
                    bestX = i;
                    bestY = j;
                    found = true;
                }
            }
        }
    }

    if (found) {
        cx = bestX;
        cy = bestY;
    }
    return found;
}

// 鼠标点击事件：卡槽/旋转/暂停/放置
void cell::mousePressEvent(QMouseEvent *event)
{
    QPoint clickPos = event->pos();

    // 允许在卡槽区域拖动
    const int slotAreaLeft = SLOT_MARGIN;
    const int slotAreaRight = SLOT_MARGIN + SLOT_WIDTH;
    const int slotAreaTop = 40;
    const int slotAreaBottom = height() - 40;

    if (clickPos.x() >= slotAreaLeft && clickPos.x() <= slotAreaRight &&
        clickPos.y() >= slotAreaTop && clickPos.y() <= slotAreaBottom)
    {
        slotManager->isDragging = true;
        slotManager->dragStartY = clickPos.y();
    }

    // 1. 点击旋转按钮
    if (rotateButtonRect.contains(clickPos)) {
        slotManager->rotateSelected();
        update();
        return;
    }

    // 2. 点击暂停按钮
    if (pauseButtonRect.contains(clickPos)) {
        isPaused = !isPaused;
        update();
        return;
    }

    // 3. 点击卡槽卡片（修复：滚动后正确计算索引）
    // 判断是否点在卡槽区域内
    if (clickPos.x() >= slotAreaLeft && clickPos.x() <= slotAreaRight &&
        clickPos.y() >= slotAreaTop && clickPos.y() <= slotAreaBottom)
    {
        // 计算点击位置在卡槽区域内的相对Y（减去滚动偏移）
        int localY = clickPos.y() - slotAreaTop - slotManager->scrollOffset;
        int slotIndex = localY / (SLOT_HEIGHT + SLOT_MARGIN);
        int totalSlots = slotManager->allBluePatterns.size();

        // 索引合法才选中
        if (slotIndex >= 0 && slotIndex < totalSlots) {
            // 再次点击同一张=取消选中
            slotManager->selectedIndex = (slotManager->selectedIndex == slotIndex) ? -1 : slotIndex;
            update();
        }
        event->accept();
        return;
    }

    // 4. 不管是否处于暂停状态下，都可以点击场地放置细胞
    if (slotManager->selectedIndex != -1) {
        // 鼠标坐标转网格坐标
        int mx = clickPos.x() - 100;
        int my = clickPos.y() - 30;
        int gridX = my / CELL_HEIGHT;
        int gridY = mx / CELL_WIDTH;

        const CellPattern& pat = slotManager->allBluePatterns[slotManager->selectedIndex];
        RotationDirection dir = slotManager->currentRotate;

        // 检查原位置是否可放
        bool valid = isPlacementValid(gridX, gridY, pat, BLUE_CELL, dir);
        if (!valid) {
            findNearestValidPos(gridX, gridY, pat, dir); // 找最近可放位置
        }

        // 放置并取消选中
        if (placeCellPattern(gridX, gridY, pat, BLUE_CELL, dir)) {
            slotManager->selectedIndex = -1;
            update();
        }
    }
}

// 鼠标拖动（滚动卡槽）
void cell::mouseMoveEvent(QMouseEvent *event)
{
    if (slotManager->isDragging) {
        int dy = event->pos().y() - slotManager->dragStartY;
        slotManager->scrollOffset += dy;
        slotManager->dragStartY = event->pos().y();

        // 边界限制
        int maxScroll = -(slotManager->allBluePatterns.size() * (SLOT_HEIGHT + SLOT_MARGIN)) + height() - 80;
        slotManager->scrollOffset = qMin(0, qMax(slotManager->scrollOffset, maxScroll));

        update();
        event->accept();
    }
}

// 鼠标释放
void cell::mouseReleaseEvent(QMouseEvent *event)
{
    if (slotManager->isDragging) {
        slotManager->isDragging = false;
        event->accept();
    }
}

// 统计指定颜色细胞总数
int cell::countAliveCells(CellType type)
{
    int cnt = 0;
    for (int i = 0; i < GRID_ROWS; i++)
        for (int j = 0; j < GRID_COLS; j++)
            if (currentGrid[i][j] == type) cnt++;
    return cnt;
}

// 更新分数
void cell::updateScore()
{
    // 细胞分：每个5分
    int blueCellCount = countAliveCells(BLUE_CELL);
    int redCellCount = countAliveCells(RED_CELL);

    // 旗帜分：摧毁一面+1000
    int destroyedBlueFlags = 0;
    for (auto& f : blueFlags) if (!f.isAlive) destroyedBlueFlags++;

    int destroyedRedFlags = 0;
    for (auto& f : redFlags) if (!f.isAlive) destroyedRedFlags++;

    blueScore = blueCellCount * 5 + destroyedRedFlags * 1000;
    redScore  = redCellCount * 5 + destroyedBlueFlags * 1000;
}

// 绘制分数
void cell::drawScoreInfo(QPainter& painter)
{
    painter.setPen(Qt::blue);
    painter.drawText(600, 50, QString("蓝方分数：%1").arg(blueScore));
    painter.setPen(Qt::red);
    painter.drawText(800, 50, QString("红方分数：%1").arg(redScore));
}