
//cell.cpp


#include "cell.h"
#include "./ui_cell.h"
#include "PatternSlot.h"
#include "StartWindow.h"

#include <QRandomGenerator>
#include <QDebug>
//#include <algorithm>

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
    , currentSpeed(1)  // 默认正常速度
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
    initTerritory();     // 【新增】初始化领地：初始区域=领地
    initFlags();         // 初始化旗帜
    initCellPatterns();  // 初始化细胞组合模板
    initEnergy();        // 初始化能量

    // 初始化卡槽管理器
    slotManager = new PatternSlot(this);
    slotManager->initAllPatterns(this);

    // ===================== 初始化清除卡牌 =====================
    initClearCards();

    // 设置按钮位置
    pauseButtonRect = QRect(1670, 0, 30, 30);    // 右上角暂停按钮
    rotateButtonRect = QRect(20, 10, 20, 20);     // 左上角旋转按钮

    // ===================== 新增：倍速按钮位置（右上角并排） =====================
    speed2xButtonRect = QRect(1640, 0, 30, 30);    // >> 2倍速
    speed3xButtonRect = QRect(1610, 0, 30, 30);    // >>> 3倍速
    speed10xButtonRect = QRect(1580, 0, 30, 30);    // ×10 10倍速

    // ===================== 新增：返回主界面按钮 =====================
    backToMenuButtonRect = QRect(790, 475, 120, 40);

    // 设置定时器（每300ms更新一次）
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &cell::updateGameState);
    gameTimer->start(300); // 数值越小速度越快

    // ===================== 初始化游戏开场提示 =====================
    tipStep = 1;         // 第1步：准备好了吗
    showReady0Text = true;
    showReadyText = false;
    showStartText = false;
    // 刚进游戏先暂停，提示走完再自动开始
    isPaused = true;

    // ===================== 背景音乐：游戏初始播放正常音乐 =====================
    AudioManager::instance().playGameNormalMusic();
}

cell::~cell()
{
    delete ui;

    // 退出游戏界面时停止音乐
    AudioManager::instance().stopMusic();
}

// ===================== 初始化领地 新增 =====================
void cell::initTerritory()
{
    for (int i = 0; i < GRID_COLS; ++i) {
        for (int j = 0; j < GRID_ROWS; ++j) {
            if (i <= 74) {
                // 蓝方初始领地
                territoryGrid[i][j] = BLUE_TERRITORY;
            } else if (i >= 125) {
                // 红方初始领地
                territoryGrid[i][j] = RED_TERRITORY;
            } else {
                // 中立领地
                territoryGrid[i][j] = NEUTRAL_TERRITORY;
            }
        }
    }
}

// ===================== 更新领地：细胞生存过的格子变为己方领地 新增 =====================
void cell::updateTerritory()
{
    for (int i = 0; i < GRID_COLS; ++i) {
        for (int j = 0; j < GRID_ROWS; ++j) {
            if (currentGrid[i][j] == BLUE_CELL) {
                // 蓝色细胞生存过 → 蓝方领地
                territoryGrid[i][j] = BLUE_TERRITORY;
            } else if (currentGrid[i][j] == RED_CELL) {
                // 红色细胞生存过 → 红方领地
                territoryGrid[i][j] = RED_TERRITORY;
            }
            // 紫色细胞不改变领地
        }
    }
}

// ===================== 统计指定类型领地数量 新增 =====================
int cell::countTerritory(TerritoryType type)
{
    int count = 0;
    for (int i = 0; i < GRID_COLS; ++i) {
        for (int j = 0; j < GRID_ROWS; ++j) {
            if (territoryGrid[i][j] == type) {
                count++;
            }
        }
    }
    return count;
}

// ===================== 初始化清除卡牌 5*5 ~ 10*10 =====================
void cell::initClearCards()
{
    m_clearCards.clear();//vector的函数clear（），使得vector的大小（size）变为0.
    QList<int> sizes = {5,6,7,8,9,10};
    for(int s : sizes){
        ClearCard card;
        card.size = s;
        card.energyCost = s*s;
        card.cooldown = 200;
        card.remain = 150;
        card.text = QString("清除%1").arg(s);
        m_clearCards.append(card);//在容器末尾追加元素，也就是这里在把5,6,7,8，9,10都一个个的加进去。
    }
}

// 初始化细胞网格：全场空细胞
void cell::initGrid()
{
    for (int i = 0; i < GRID_COLS; ++i) {
        for (int j = 0; j < GRID_ROWS; ++j) {
            currentGrid[i][j] = EMPTY;
            nextGrid[i][j] = EMPTY;
        }
    }

    //初始时的紫色细胞放置
    // 只遍历中间紫色区域 75~124 列
    for(int i = 75; i <= 124; ++i)  {
        for(int j = 0; j < GRID_ROWS; ++j)  {
            int randVal = QRandomGenerator::global()->bounded(100);
            // 10%概率生成紫色细胞
            if (randVal < 20) {
                currentGrid[i][j] = PURPLE_CELL;
            }
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
    // ------------------- 蓝方专用 single_cell 单个细胞 -------------------
    CellPattern singleCell;
    singleCell.name = "single_cell";
    singleCell.cellPositions = {QPoint(0, 0)};
    singleCell.energyCost = ENERGY_PER_CELL_PLACEMENT;
    singleCell.priority = 1;
    bluesingleLifePatterns.prepend(singleCell); // 放在最前面

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



    // ------------------- 蓝方飞船型组合（优先级3） -------------------
    // 1. 滑翔机
    CellPattern glider;
    glider.name = "滑翔机";
    glider.cellPositions = {QPoint(0, -1), QPoint(1, 0), QPoint(-1, 1), QPoint(0, 1), QPoint(1, 1)};
    glider.energyCost = glider.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
    glider.priority = 3;
    blueSpaceshipPatterns.append(glider);

    // 2. 轻型飞船 (Lightweight spaceship)
    CellPattern lwss;
    lwss.name = "轻型飞船";
    lwss.cellPositions = {
        QPoint(1,0), QPoint(4,0),
        QPoint(0,1),
        QPoint(0,2),QPoint(4,2),
        QPoint(0,3), QPoint(1,3), QPoint(2,3), QPoint(3,3)
    };
    lwss.energyCost = lwss.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
    lwss.priority = 3;
    blueSpaceshipPatterns.append(lwss);

    // 3. 中型飞船 (Middleweight spaceship)
    CellPattern mwss;
    mwss.name = "中型飞船";
    mwss.cellPositions = {
        QPoint(3, 0),
        QPoint(1, 1), QPoint(5, 1),
        QPoint(0, 2),
        QPoint(0, 3),
        QPoint(5, 3),
        QPoint(0, 4), QPoint(1, 4), QPoint(2, 4), QPoint(3, 4), QPoint(4, 4)
    };
    mwss.energyCost = mwss.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
    mwss.priority = 3;
    blueSpaceshipPatterns.append(mwss);

    // 4. 空间犁 (Backward space rake)
    CellPattern spaceRake;
    spaceRake.name = "空间犁";
    spaceRake.cellPositions = {
        QPoint(11, 0), QPoint(12, 0), QPoint(18, 0), QPoint(19, 0), QPoint(20, 0), QPoint(21, 0),
    QPoint(9, 1), QPoint(10, 1), QPoint(12, 1), QPoint(13, 1), QPoint(17, 1), QPoint(21, 1),
        QPoint(9, 2), QPoint(10, 2), QPoint(11, 2), QPoint(12, 2), QPoint(21, 2),
        QPoint(10, 3), QPoint(11, 3), QPoint(17, 3), QPoint(20, 3),

        QPoint(8, 5),
        QPoint(7, 6), QPoint(8, 6), QPoint(17, 6), QPoint(18, 6),
        QPoint(6, 7), QPoint(16, 7), QPoint(19, 7),
        QPoint(7, 8), QPoint(8, 8), QPoint(9, 8), QPoint(10, 8), QPoint(11, 8), QPoint(16, 8), QPoint(19, 8),
        QPoint(8, 9), QPoint(9, 9), QPoint(10, 9), QPoint(11, 9), QPoint(15, 9), QPoint(16, 9), QPoint(18, 9), QPoint(19, 9),
        QPoint(11, 10), QPoint(16, 10), QPoint(17, 10),

        QPoint(18, 14), QPoint(19, 14), QPoint(20, 14), QPoint(21, 14),
        QPoint(17, 15), QPoint(21, 15),
        QPoint(0, 16), QPoint(1, 16), QPoint(2, 16), QPoint(3, 16), QPoint(21, 16),
        QPoint(-1, 17), QPoint(3, 17), QPoint(17, 17), QPoint(20, 17),
        QPoint(3, 18),
        QPoint(-1, 19), QPoint(2, 19)
    };
    spaceRake.energyCost = spaceRake.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
    spaceRake.priority = 3;
    blueSpaceshipPatterns.append(spaceRake);

    // ------------------- 蓝方枪型组合（优先级4） -------------------
    // 1. 高斯珀滑翔机枪（简化版）
    CellPattern gun;
    gun.name = "高斯珀滑翔机枪";
    gun.cellPositions = {QPoint(5,1),QPoint(6,1),
                         QPoint(5,2),QPoint(6,2),

                         QPoint(5,11),QPoint(6,11),QPoint(7,11),
                         QPoint(4,12),QPoint(8,12),
                         QPoint(3,13),QPoint(9,13),
                         QPoint(3,14),QPoint(9,14),
                         QPoint(6,15),
                         QPoint(4,16),QPoint(8,16),
                         QPoint(5,17),QPoint(6,17),QPoint(7,17),
                         QPoint(6,18),

                         QPoint(3,21),QPoint(4,21),QPoint(5,21),
                         QPoint(3,22),QPoint(4,22),QPoint(5,22),
                         QPoint(2,23),QPoint(6,23),
                         QPoint(1,25),QPoint(2,25),QPoint(6,25),QPoint(7,25),

                         QPoint(3,35),QPoint(4,35),
                         QPoint(3,36),QPoint(4,36),
    };
    gun.energyCost = gun.cellPositions.size() * ENERGY_PER_CELL_PLACEMENT;
    gun.priority = 4;
    blueGunPatterns.append(gun);



  /*  // 红方直接复制蓝方模板（不好，两个应当分别处理，因为红蓝并没有设计成一样）
    redStillLifePatterns = blueStillLifePatterns;
    redOscillatorPatterns = blueOscillatorPatterns;
    redSpaceshipPatterns = blueSpaceshipPatterns;
    redGunPatterns = blueGunPatterns;

    // 红方删除 single_cell
    if (!redStillLifePatterns.isEmpty() && redStillLifePatterns.first().name == "single_cell") {
        redStillLifePatterns.removeFirst();
    }*/

    // ------------------- 红方稳定型组合（优先级2） -------------------
    // 1. 方块（2x2）
    redStillLifePatterns.append(block);

    // 2. 船型
    redStillLifePatterns.append(boat);

    // 补充稳定型组合



    // ------------------- 红方振荡型组合（优先级1） -------------------
    // 1. 闪烁器
    redOscillatorPatterns.append(blinker);

    // 2. 钟摆
    redOscillatorPatterns.append(toad);

    // 补充振荡型


    // ------------------- 红方飞船型组合（优先级3） -------------------
    // 1. 滑翔机

    redSpaceshipPatterns.append(glider);

    // 2. 轻型飞船 (Lightweight spaceship)
    redSpaceshipPatterns.append(lwss);

    // 3. 中型飞船 (Middleweight spaceship)
    redSpaceshipPatterns.append(mwss);

    // 4. 空间犁 (Backward space rake)
    redSpaceshipPatterns.append(spaceRake);


    // ------------------- 红方枪型组合（优先级4） -------------------
    // 1. 高斯珀滑翔机枪
    redGunPatterns.append(gun);

    // 补充枪型

}
/*//不再按固定优先级，而是有一定概率。优先的概率高。

*/

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
            if (nx >= 0 && nx < GRID_COLS && ny >= 0 && ny < GRID_ROWS) {
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

            if (nx >= 0 && nx < GRID_COLS && ny >= 0 && ny < GRID_ROWS) {
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
    for (int i = 75; i <= 124; ++i) {
        for(int j = 0; j < GRID_ROWS; ++j)  {
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
    // 获取旋转后坐标
    QVector<QPoint> rotatedPos = rotatePattern(pattern.cellPositions, dir);

    // 检查每个细胞位置
    for (const QPoint& p : rotatedPos) {
        int x = centerX + p.x();
        int y = centerY + p.y();

        // 越界判断
        if (x < 0 || x >= GRID_COLS || y < 0 || y >= GRID_ROWS) {
            return false;
        }

        // ===================== 核心规则：只能放在自己领地内 =====================
        if (cellType == BLUE_CELL && territoryGrid[x][y] != BLUE_TERRITORY) {
            return false;
        }
        if (cellType == RED_CELL && territoryGrid[x][y] != RED_TERRITORY) {
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
    if (redEnergy > 50) {
        // 随机方向：左上 / 左下 各50%
        RotationDirection redDir = (QRandomGenerator::global()->bounded(2) == 0) ? UP_LEFT : DOWN_LEFT;

        int randVal = QRandomGenerator::global()->bounded(100);
        QVector<CellPattern> selectedList;
        bool placeNothing = false;

        // 按能量区间分配概率
        if (redEnergy < 500) {
            if (randVal < 40)      selectedList = redGunPatterns;
            else if (randVal < 80) selectedList = redSpaceshipPatterns;
            else if (randVal < 85) selectedList = redOscillatorPatterns;
            else if (randVal < 90) selectedList = redStillLifePatterns;
            else                   placeNothing = true;
        } else {
            if (randVal < 40)      selectedList = redGunPatterns;
            else if (randVal < 90) selectedList = redSpaceshipPatterns;
            else if (randVal < 95) selectedList = redOscillatorPatterns;
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
                int cx = QRandomGenerator::global()->bounded(0, GRID_COLS);
                int cy = QRandomGenerator::global()->bounded(GRID_ROWS); // 红方区域
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
            if (currentGrid[x][y] == RED_CELL || currentGrid[x][y] == PURPLE_CELL) {
                blueFlags[i].isAlive = false;
            }
        }
    }

    // 红旗被蓝/紫占领
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (redFlags[i].isAlive) {
            int x = redFlags[i].x;
            int y = redFlags[i].y;
            if (currentGrid[x][y] == BLUE_CELL || currentGrid[x][y] == PURPLE_CELL) {
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
    for (int i = 0; i < GRID_COLS; ++i) {
        for (int j = 0; j < GRID_ROWS; ++j) {
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
    for (int i = 0; i < GRID_COLS; ++i) {
        for (int j = 0; j < GRID_ROWS; ++j) {
            currentGrid[i][j] = nextGrid[i][j];
        }
    }

    // 更新领地 新增
    updateTerritory();

    // 检查旗帜与游戏结束
    checkFlagOccupation();
    if (!gameOver) {
        gameOver = checkGameOver();
        if (gameOver) {
            gameTimer->stop();
            QMessageBox::information(this, "游戏结束", winner + "获胜！");

            // 新增：游戏结束后允许返回主界面
            isPaused = true;
            update(); // 刷新界面显示返回按钮
        }
    }
}

// ===================== 绘制领地 新增 =====================
void cell::drawTerritory(QPainter &painter)
{
    for (int i = 0; i < GRID_COLS; ++i) {
        for (int j = 0; j < GRID_ROWS; ++j) {
            int x = 100 + i * CELL_WIDTH;
            int y = 30 + j * CELL_HEIGHT;

            switch (territoryGrid[i][j]) {
            case BLUE_TERRITORY:
                painter.fillRect(x, y, CELL_WIDTH, CELL_HEIGHT, QColor(0, 0, 255, 30));
                break;
            case RED_TERRITORY:
                painter.fillRect(x, y, CELL_WIDTH, CELL_HEIGHT, QColor(255, 0, 0, 30));
                break;
            case NEUTRAL_TERRITORY:
            default:
                break;
            }
        }
    }
}


// 绘制所有细胞
void cell::drawCells(QPainter &painter)
{
    for (int i = 0; i < GRID_COLS; ++i) {
        for (int j = 0; j < GRID_ROWS; ++j) {
            int x = 100 + i * CELL_WIDTH;
            int y = 30 + j * CELL_HEIGHT;

            switch (currentGrid[i][j]) {
            case BLUE_CELL:
                painter.drawPixmap(x, y, CELL_WIDTH, CELL_HEIGHT, blueCellImg);
                break;
            case RED_CELL:
                painter.drawPixmap(x, y, CELL_WIDTH, CELL_HEIGHT, redCellImg);
                break;
            case PURPLE_CELL:
                painter.drawPixmap(x, y, CELL_WIDTH, CELL_HEIGHT, purpleCellImg);
                break;
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

    // 绘制领地 新增
    drawTerritory(painter);

    // 绘制游戏元素
    drawCells(painter);
    drawFlags(painter);

    // 绘制能量文本
    painter.setPen(Qt::blue);
    painter.drawText(100, 11, QString("蓝方能量: %1").arg(blueEnergy));
    painter.setPen(Qt::red);
    painter.drawText(100, 28, QString("红方能量: %1").arg(redEnergy));

    // 新增UI绘制
    drawPatternSlots(painter);
    drawPauseButton(painter);
    drawGenerationInfo(painter);
    drawScoreInfo(painter); // 绘制分数

    // ===================== 绘制倍速按钮 =====================
    drawSpeedButtons(painter);

    drawBackToMenuButton(painter); // 绘制返回按钮

    // ===================== 绘制提示 =====================
    if (showReady0Text || showReadyText || showStartText)
    {
        painter.setFont(QFont("Microsoft YaHei", 48, QFont::Bold));
        painter.setPen(Qt::red);
        if (showReady0Text)
            painter.drawText(rect(), Qt::AlignCenter, "准备好了吗");
        else if (showReadyText)
            painter.drawText(rect(), Qt::AlignCenter, "预备！");//你真的能打败随机放置的无脑电脑吗？
        else if (showStartText)
            painter.drawText(rect(), Qt::AlignCenter, "开始游戏");
    }
}

// ===================== 清除卡牌冷却更新 =====================
void cell::updateClearCooldown()
{
    for(auto &c : m_clearCards){
        if(c.remain > 0) c.remain--;
    }
}

// 定时器驱动的游戏主循环
void cell::updateGameState()
{
    // ===================== 自动提示：准备好了吗 → 开始游戏 =====================
    if (generationCount == 0 && tipStep != 0)
    {        if (tipStep == 1)
        {
            tipStep = 99;//关键：只执行一次，防止重复创建定时器
            // 显示“准备好了吗”持续1秒
            QTimer::singleShot(1000, this,[=]()  {
                tipStep = 2;
                showReady0Text = false;
                showReadyText = true;
                showStartText = false;
                update();
            });
        }
        else if (tipStep == 2)
        {
            tipStep = 99;//关键：只执行一次，防止重复创建定时器
            // 显示“准备好了吗”持续1秒//显示“你真的能打败随机放置的无脑电脑吗?”持续2秒
            QTimer::singleShot(2000, this,[=]()  {
                tipStep = 3;
                showReady0Text = false;
                showReadyText = false;
                showStartText = true;
                update();
            });
        }
        else if (tipStep == 3)
        {
            // 显示“开始游戏”持续1秒
            QTimer::singleShot(1000, this, [=]() {
                tipStep = 0;
                showReady0Text = false;
                showReadyText = false;
                showStartText = false;
                isPaused = false; // 提示结束，解除暂停
                update();
            });
        }

        update();
        return; // 提示期间不跑游戏逻辑
    }

    // 未结束且未暂停时才迭代
    if (!gameOver && !isPaused) {
        generationCount++;       // 代数自增
        updateEnergy();          // 更新能量
        updateClearCooldown();   // 每代更新清除卡冷却！
        updateScore();           // 更新分数
        randomPlacePatterns();   // 随机放置
        calculateNextGeneration();// 计算下一代
        update();                // 重绘

        // ===================== 背景音乐：8000代后切换音乐 =====================
        if (generationCount == 8000) {
            AudioManager::instance().playGameLateMusic();
        }
    }
}

// ===================== 绘制清除卡牌（顶部显示） =====================
void cell::drawClearSlots(QPainter& painter, int yOffset)
{
    painter.save();
    int xBase = SLOT_MARGIN;
    int yBase = yOffset;
    painter.setFont(QFont("Arial",9));

    for(int i=0;i<m_clearCards.size();i++){
        auto &c = m_clearCards[i];
        QRect r(xBase, yBase + i*(SLOT_HEIGHT+10), SLOT_WIDTH, SLOT_HEIGHT);

        // 背景
        painter.setPen(Qt::black);
        if(c.remain <=0 && blueEnergy >= c.energyCost){
            painter.setBrush(QColor(200,230,255));
        }else{
            painter.setBrush(QColor(240,240,240));
        }
        painter.drawRect(r);

        // 中上：剩余冷却
        painter.setPen(Qt::darkRed);
        painter.drawText(r.adjusted(0,5,0,-35),Qt::AlignHCenter,
                         c.remain>0 ? QString("%1代").arg(c.remain) : "就绪");

        // 中下：文字
        painter.setPen(Qt::black);
        painter.drawText(r.adjusted(0,30,0,0),Qt::AlignHCenter,c.text);
    }
    painter.restore();
}


// 绘制左侧细胞卡槽（支持滚动）
void cell::drawPatternSlots(QPainter& painter)
{



    // ===================== 先画【总容器大边框 + 背景】=====================
    QRect containerRect(SLOT_MARGIN-5, 40-5, SLOT_WIDTH+10, height() - 80+10);
    QRect innercontainerRect(SLOT_MARGIN-1, 40-1, SLOT_WIDTH+2, height() - 80+2);

    // 设置：边框 + 背景
    QPen pen(QColor(139, 69, 19)); // 棕色
    pen.setWidth(2);
    painter.setPen(pen);
    painter.setBrush(QColor(210, 180, 140)); // 背景
    painter.drawRect(containerRect); // 画出总容器外框

    painter.setPen(QColor(210, 180, 140)); // 浅棕色边框
    painter.setBrush(QColor(255,255,255)); // 背景
    pen.setWidth(1);
    painter.drawRect(innercontainerRect); // 画出总容器内框

    // 6 张清除卡，每张 = SLOT_HEIGHT + 10 间距[1]
    int CLEAR_SLOTS_TOTAL_HEIGHT = m_clearCards.size() * (SLOT_HEIGHT + 10);

    // ===================== 绘制普通细胞团卡牌 =====================
    painter.setPen(Qt::blue);
    painter.setBrush(Qt::transparent);


    int startY = 40 + slotManager->scrollOffset + CLEAR_SLOTS_TOTAL_HEIGHT;
    int slotCount = slotManager->allBluePatterns.size();
    int totalHeight = slotCount * (SLOT_HEIGHT + SLOT_MARGIN);

    // 裁剪区域，只显示可视区域
    painter.save();
    painter.setClipRect(SLOT_MARGIN, 40, SLOT_WIDTH, height() - 80);

    // ===================== 绘制清除卡牌（最顶部） =====================
    drawClearSlots(painter,40 + slotManager->scrollOffset);

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
        QVector<QPoint> pts = rotatePattern(pat.cellPositions, slotManager->currentRotate);
        int cx = slotRect.center().x();
        int cy = slotRect.center().y();

        painter.setPen(Qt::blue);
        painter.setBrush(Qt::blue);
        for (const QPoint& p : pts) {
            int px = cx + p.x() * 2;
            int py = cy + p.y() * 2;
            painter.drawEllipse(px - 5, py - 5, 2, 2);
        }
        // ====================== 显示卡片名字 ======================
        painter.setPen(Qt::black);          // 文字颜色黑色
        painter.setFont(QFont("Arial", 8)); // 小号字体，不挡缩略图
        painter.drawText(slotRect.adjusted(0, 35, 0, 0),  // 向下偏移一点，不盖住图案
                         Qt::AlignHCenter | Qt::AlignTop,
                         pat.name);  // 直接显示图案名字
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

// ===================== 绘制倍速按钮 =====================
void cell::drawSpeedButtons(QPainter& painter)
{
    // 绘制 2倍速按钮 >>
    painter.setPen(Qt::black);
    if(currentSpeed == 2){
        painter.setBrush(QColor(255, 220, 100)); // 选中高亮
    } else {
        painter.setBrush(QColor(220, 220, 220));
    }
    painter.drawRect(speed2xButtonRect);
    painter.drawText(speed2xButtonRect, Qt::AlignCenter, ">>");

    // 绘制 3倍速按钮 >>>
    if(currentSpeed == 3){
        painter.setBrush(QColor(255, 180, 80));
    } else {
        painter.setBrush(QColor(220, 220, 220));
    }
    painter.drawRect(speed3xButtonRect);
    painter.drawText(speed3xButtonRect, Qt::AlignCenter, ">>>");

    // 绘制 10倍速按钮 ×10
    if(currentSpeed == 10){
        painter.setBrush(QColor(255, 180, 80));
    } else {
        painter.setBrush(QColor(220, 220, 220));
    }
    painter.drawRect(speed10xButtonRect);
    painter.drawText(speed10xButtonRect, Qt::AlignCenter, "×10");
}

// 绘制细胞代数
void cell::drawGenerationInfo(QPainter& painter)
{
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(1400, 25, QString("代数: %1").arg(generationCount));
}

// 寻找距离最近的可放置位置
bool cell::findNearestValidPos(int& cx, int& cy, const CellPattern& pat, RotationDirection dir)
{
    int bestX = cx, bestY = cy;
    int minDist = 99999;
    bool found = false;

    // 遍历蓝方全区域搜索
    for (int i = 0; i < GRID_COLS; ++i) {
        for (int j = 0; j < GRID_ROWS; ++j) {
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

// ===================== 使用清除卡牌 =====================
bool cell::useClearCard(int idx, int cx, int cy)
{
    if(idx <0 || idx >= m_clearCards.size()) return false;
    auto &card = m_clearCards[idx];
    if(card.remain > 0) return false;
    if(blueEnergy < card.energyCost) return false;

    // 扣除能量
    blueEnergy -= card.energyCost;

    // 清除 2N*2N 范围
    int n = card.size;
    for(int dx=-n; dx<=n; dx++){
        for(int dy=-n; dy<=n; dy++){
            int x = cx+dx;
            int y = cy+dy;
            if(x>=0 && x<GRID_COLS && y>=0 && y<GRID_ROWS){
                currentGrid[x][y] = EMPTY;
            }
        }
    }

    // 进入冷却
    card.remain = card.cooldown;
    qDebug() << "使用清除"<<card.size<<"x"<<card.size<<" 剩余能量"<<blueEnergy;
    return true;
}

// 鼠标点击事件：卡槽/旋转/暂停/放置
void cell::mousePressEvent(QMouseEvent *event)
{
    // 6 张清除卡，每张 = SLOT_HEIGHT + 10 间距[2]
    int CLEAR_SLOTS_TOTAL_HEIGHT = m_clearCards.size() * (SLOT_HEIGHT + 10);

    QPoint clickPos = event->pos();




    // ===================== 点击地图 = 执行清除 =====================
    if (selectedClearCardIndex != -1) {
        // 把鼠标坐标 → 地图网格坐标
        int mx = event->x() - 100;
        int my = event->y() - 30;
        int gx = mx / CELL_WIDTH;
        int gy = my / CELL_HEIGHT;
     if (mx >= 0 && my >= 0) {
        // 执行清除
        useClearCard(selectedClearCardIndex, gx, gy);

        // 清除完取消选中
        selectedClearCardIndex = -1;

        update();
        event->accept();
        return;
      }
    }

    // 允许在卡槽区域拖动
    const int slotAreaLeft = SLOT_MARGIN;
    const int slotAreaRight = SLOT_MARGIN + SLOT_WIDTH;
    const int slotAreaTop = 40;
    const int slotAreaBottom = height() - 40;

    //判断是否在卡槽范围内
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
        if (isPaused) {
            // 暂停音乐...
            AudioManager::instance().pauseMusic();
        } else {
            // 恢复音乐...
            AudioManager::instance().resumeMusic();
        }
        update();
        return;
    }

    // ===================== 点击清除卡牌 = 选中卡牌 =====================
    int clearCardHeight = SLOT_HEIGHT + 10;// 判断是否点在卡槽区域内
    if (clickPos.x() >= slotAreaLeft && clickPos.x() <= slotAreaRight &&
        clickPos.y() >= slotAreaTop && clickPos.y() <= slotAreaBottom)
    {//（减去滚动偏移）!!
        if (clickPos.x() >= SLOT_MARGIN && clickPos.x() <= SLOT_MARGIN + SLOT_WIDTH) {
            int y = clickPos.y() - 40 - slotManager->scrollOffset;
            if (y >= 0 && y < m_clearCards.size() * clearCardHeight) {
                int idx = y / clearCardHeight;

                // 只选中，不立即清除！
                selectedClearCardIndex = idx;

                update();
                event->accept();
                return;
            }
        }
    }

    // 3. 点击卡槽卡片（修复：滚动后正确计算索引）
    // 判断是否点在卡槽区域内
    if (clickPos.x() >= slotAreaLeft && clickPos.x() <= slotAreaRight &&
        clickPos.y() >= slotAreaTop && clickPos.y() <= slotAreaBottom)
    {
        // 计算点击位置在卡槽区域内的相对Y（减去滚动偏移）
        int localY = clickPos.y() - slotAreaTop - CLEAR_SLOTS_TOTAL_HEIGHT - slotManager->scrollOffset;
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
        int gridX = mx / CELL_WIDTH;
        int gridY = my / CELL_HEIGHT;

        const CellPattern& pat = slotManager->allBluePatterns[slotManager->selectedIndex];
        RotationDirection dir = slotManager->currentRotate;

        // 检查原位置是否可放
        bool valid = isPlacementValid(gridX, gridY, pat, BLUE_CELL, dir);
        if (!valid) {
            findNearestValidPos(gridX, gridY, pat, dir); // 找最近可放位置
        }

        // 放置并不取消选中（可以连续放置）
        if (placeCellPattern(gridX, gridY, pat, BLUE_CELL, dir)) {
            //slotManager->selectedIndex = -1;
            update();
        }
    }

    // ===================== 点击倍速（可切回1倍） =====================
    if(speed2xButtonRect.contains(clickPos)){
        if(currentSpeed == 2){
            // 已经是2倍 → 切回1倍原速
            currentSpeed = 1;
            gameTimer->setInterval(300);
        }else{
            // 切到2倍
            currentSpeed = 2;
            gameTimer->setInterval(150);
        }
        update();
        return;
    }
    if(speed3xButtonRect.contains(clickPos)){
        if(currentSpeed == 3){
            // 已经是3倍 → 切回1倍原速
            currentSpeed = 1;
            gameTimer->setInterval(300);
        }else{
            // 切到3倍
            currentSpeed = 3;
            gameTimer->setInterval(100);
        }
        update();
        return;
    }
    if(speed10xButtonRect.contains(clickPos)){
        if(currentSpeed == 10){
            // 已经是10倍 → 切回1倍原速
            currentSpeed = 1;
            gameTimer->setInterval(300);
        }else{
            // 切到10倍
            currentSpeed = 10;
            gameTimer->setInterval(30);
        }
        update();
        return;
    }

    // ===================== 点击返回主界面 =====================
    if(gameOver && backToMenuButtonRect.contains(clickPos))
    {
        this->close();
        StartWindow* w = new StartWindow;
        w->show();
        return;
    }
}

// 鼠标拖动（滚动卡槽）
void cell::mouseMoveEvent(QMouseEvent *event)
{
    if (slotManager->isDragging) {
        int dy = event->pos().y() - slotManager->dragStartY;
        slotManager->scrollOffset += dy;
        slotManager->dragStartY = event->pos().y();

        // 6 张清除卡，每张 = SLOT_HEIGHT + 10 间距[3]
        int CLEAR_SLOTS_TOTAL_HEIGHT = m_clearCards.size() * (SLOT_HEIGHT + 10);

        // 边界限制
        int maxScroll = -(slotManager->allBluePatterns.size() * (SLOT_HEIGHT + SLOT_MARGIN)) + height() - 80 - CLEAR_SLOTS_TOTAL_HEIGHT;
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
    for (int i = 0; i < GRID_COLS; i++)
        for (int j = 0; j < GRID_ROWS; j++)
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

    // 领地分：每格10分 新增
    int blueTerritoryCount = countTerritory(BLUE_TERRITORY);
    int redTerritoryCount = countTerritory(RED_TERRITORY);

    // 总分数 = 细胞分 + 旗帜分 + 领地分
    blueScore = blueCellCount * 10 + destroyedRedFlags * 3000 + blueTerritoryCount * 5;
    redScore  = redCellCount * 10 + destroyedBlueFlags * 3000 + redTerritoryCount * 5;
}

// 绘制分数
void cell::drawScoreInfo(QPainter& painter)
{
    painter.setPen(Qt::blue);
    painter.drawText(700, 20, QString("蓝方分数：%1").arg(blueScore));
    painter.setPen(Qt::red);
    painter.drawText(900, 20, QString("红方分数：%1").arg(redScore));
}

// ===================== 绘制返回主界面按钮 =====================
void cell::drawBackToMenuButton(QPainter& painter)
{
    if(gameOver) // 只有游戏结束才显示
    {
        painter.setPen(Qt::black);
        painter.setBrush(Qt::white);
        painter.drawRect(backToMenuButtonRect);
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        painter.drawText(backToMenuButtonRect, Qt::AlignCenter, "返回主界面");
    }
}