#include "cell.h"
#include "./ui_cell.h"
#include <QRandomGenerator>

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
{
    ui->setupUi(this);

    // 设置窗口大小
    this->setGeometry(QRect(0, 0, 1700, 990));
    this->setFixedSize(1700, 990); // 固定窗口大小

    // 加载图片资源（请确保路径正确）
    blueCellImg.load(":/images/Image/blue.bmp");
    redCellImg.load(":/images/Image/red.bmp");
    purpleCellImg.load(":/images/Image/purple.bmp");
    flagImg.load(":/images/Image/flag.bmp");
    backgroundImg.load(":/images/Image/background.bmp");

    // 初始化游戏
    initGrid();
    initFlags();

    // 设置定时器（每300ms更新一次）
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &cell::updateGameState);
    gameTimer->start(300); // 可调整速度，数值越小越快
}

cell::~cell()
{
    delete ui;
}

// 初始化细胞网格：全场随机生成红蓝细胞
// 初始化细胞网格：左半场蓝、中间紫、右半场红
void cell::initGrid()
{
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            // 随机生成细胞：10%概率生成细胞（红/蓝/紫各10%）
            int randVal = QRandomGenerator::global()->bounded(100);
            // 根据网格列（X坐标）划分细胞类型
            if (j < 75) {
                // 左半场（0~74）：蓝色细胞
                if (randVal < 10) {
                    currentGrid[i][j] = BLUE_CELL;}
            } else if (j <= 124) {
                // 中间区域（75~124）：紫色细胞
                if (randVal < 10) {
                    currentGrid[i][j] = PURPLE_CELL;}
            } else {
                // 右半场（125~199）：红色细胞
                if (randVal < 10) {
                    currentGrid[i][j] = RED_CELL;}
            }
            // 下一代网格初始化为空（后续按规则更新）
            nextGrid[i][j] = EMPTY;
        }
    }
}
/*void cell::initGrid()
{
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            // 随机生成细胞：30%概率生成细胞（红/蓝各15%）
            int randVal = QRandomGenerator::global()->bounded(100);
            if (randVal < 15) {
                currentGrid[i][j] = BLUE_CELL;
            } else if (randVal < 30) {
                currentGrid[i][j] = RED_CELL;
            } else {
                currentGrid[i][j] = EMPTY;
            }
            nextGrid[i][j] = EMPTY;
        }
    }
}*/
// 初始化旗帜位置：左侧5个蓝旗，右侧5个红旗
void cell::initFlags()
{
    // 蓝色旗帜（左侧固定位置：网格X=30，网格Y按间隔13行排列）

    for (int i = 0; i < FLAG_COUNT; ++i) {
        blueFlags[i].x = 0;       // 网格X坐标（列）
        blueFlags[i].y = 14+13*i;             // 网格Y坐标（行）
        blueFlags[i].isAlive = true;
    }

    // 红色旗帜（右侧固定位置：网格X=GRID_COLS-30，网格Y和蓝旗对称）


    for (int i = 0; i < FLAG_COUNT; ++i) {
        redFlags[i].x =199;        // 网格X坐标（列）
        redFlags[i].y = 14+13*i;          // 网格Y坐标（行）
        redFlags[i].isAlive = true;
    }
}

// 统计指定位置周围所有活细胞数（红蓝紫都算）
int cell::countAllAliveCells(int x, int y)
{
    int count = 0;

    // 遍历8个邻居
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            // 跳过自身
            if (dx == 0 && dy == 0) continue;

            int nx = x + dx;
            int ny = y + dy;

            // 边界检查
            if (nx >= 0 && nx < GRID_ROWS && ny >= 0 && ny < GRID_COLS) {
                // 统计所有非空细胞
                if (currentGrid[nx][ny] != EMPTY) {
                    count++;
                }
            }
        }
    }
    return count;
}

// 统计指定位置周围指定颜色的细胞数
int cell::countColorAliveCells(int x, int y, CellType color)
{
    int count = 0;

    // 遍历8个邻居
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            // 跳过自身
            if (dx == 0 && dy == 0) continue;

            int nx = x + dx;
            int ny = y + dy;

            // 边界检查
            if (nx >= 0 && nx < GRID_ROWS && ny >= 0 && ny < GRID_COLS) {
                // 统计指定颜色细胞
                if (currentGrid[nx][ny] == color) {
                    count++;
                }
            }
        }
    }
    return count;
}

// 检查旗帜是否被对方细胞占领
void cell::checkFlagOccupation()
{
    // 检查蓝旗是否被红旗/紫旗占领
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (blueFlags[i].isAlive) {
            int x = blueFlags[i].x;
            int y = blueFlags[i].y;
            // 红旗或紫旗占领蓝旗位置，蓝旗消失
            if (currentGrid[y][x] == RED_CELL || currentGrid[y][x] == PURPLE_CELL) {
                blueFlags[i].isAlive = false;
            }
        }
    }

    // 检查红旗是否被蓝旗/紫旗占领
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (redFlags[i].isAlive) {
            int x = redFlags[i].x;
            int y = redFlags[i].y;
            // 蓝旗或紫旗占领红旗位置，红旗消失
            if (currentGrid[y][x] == BLUE_CELL || currentGrid[y][x] == PURPLE_CELL) {
                redFlags[i].isAlive = false;
            }
        }
    }
}

// 检查游戏是否结束（某一方旗帜全部消失）
bool cell::checkGameOver()
{
    // 检查蓝旗是否全部消失
    bool allBlueFlagsGone = true;
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (blueFlags[i].isAlive) {
            allBlueFlagsGone = false;
            break;
        }
    }

    // 检查红旗是否全部消失
    bool allRedFlagsGone = true;
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (redFlags[i].isAlive) {
            allRedFlagsGone = false;
            break;
        }
    }

    // 判定胜负
    if (allBlueFlagsGone) {
        winner = "红色方";
        return true;
    } else if (allRedFlagsGone) {
        winner = "蓝色方";
        return true;
    }

    return false;
}

// 计算下一代细胞状态（新规则）
void cell::calculateNextGeneration()
{
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            CellType currentType = currentGrid[i][j];
            int allAlive = countAllAliveCells(i, j); // 所有活细胞数（死亡判定）
            int blueAlive = countColorAliveCells(i, j, BLUE_CELL); // 蓝色细胞数
            int redAlive = countColorAliveCells(i, j, RED_CELL);   // 红色细胞数

            if (currentType != EMPTY) {
                // 活细胞死亡规则：周围细胞数不是2-3则死亡
                if (allAlive == 2 || allAlive == 3) {
                    nextGrid[i][j] = currentType; // 存活
                } else {
                    nextGrid[i][j] = EMPTY; // 死亡
                }
            } else {
                // 死细胞生成规则
                bool blueBorn = (blueAlive == 3); // 蓝色生成条件
                bool redBorn = (redAlive == 3);   // 红色生成条件

                if (blueBorn && redBorn) {
                    nextGrid[i][j] = PURPLE_CELL; // 同时满足生成紫色
                } else if (blueBorn) {
                    nextGrid[i][j] = BLUE_CELL;   // 仅蓝色满足生成蓝色
                } else if (redBorn) {
                    nextGrid[i][j] = RED_CELL;    // 仅红色满足生成红色
                } else {
                    nextGrid[i][j] = EMPTY;       // 都不满足保持空
                }
            }
        }
    }

    // 将下一代状态复制到当前状态
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            currentGrid[i][j] = nextGrid[i][j];
        }
    }

    // 检查旗帜占领状态
    checkFlagOccupation();

    // 检查游戏是否结束
    if (!gameOver) {
        gameOver = checkGameOver();
        if (gameOver) {
            gameTimer->stop(); // 停止游戏
            QMessageBox::information(this, "游戏结束", winner + "获胜！");
        }
    }
}

// 绘制细胞
void cell::drawCells(QPainter &painter)
{
    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            // 计算细胞在窗口中的实际坐标
            int x = 100 + j * CELL_WIDTH;   // 100是背景偏移量
            int y = 30 + i * CELL_HEIGHT;    // 30是背景偏移量

            // 根据细胞类型绘制对应图片
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
                break; // 空细胞不绘制
            }
        }
    }
}

// 绘制旗帜
void cell::drawFlags(QPainter &painter)
{
    // 绘制蓝色旗帜
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (blueFlags[i].isAlive) {
            int x = 100 + blueFlags[i].x * CELL_WIDTH;
            int y = 30 + blueFlags[i].y * CELL_HEIGHT;
            painter.drawPixmap(x, y, CELL_WIDTH, CELL_HEIGHT, flagImg);
        }
    }

    // 绘制红色旗帜
    for (int i = 0; i < FLAG_COUNT; ++i) {
        if (redFlags[i].isAlive) {
            int x = 100 + redFlags[i].x * CELL_WIDTH;
            int y = 30 + redFlags[i].y * CELL_HEIGHT;
            painter.drawPixmap(x, y, CELL_WIDTH, CELL_HEIGHT, flagImg);
        }
    }
}

// 绘制事件
void cell::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // 设置抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制外框
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, 1700, 990);

    // 绘制游戏区域背景
    painter.drawRect(100, 30, GAME_WIDTH, GAME_HEIGHT);
    painter.drawPixmap(100, 30, GAME_WIDTH, GAME_HEIGHT, backgroundImg);

    // 绘制细胞
    drawCells(painter);

    // 绘制旗帜（在细胞上层）
    drawFlags(painter);
}

// 更新游戏状态（定时器触发）
void cell::updateGameState()
{
    if (!gameOver) {
        calculateNextGeneration();
        update(); // 触发重绘
    }
}