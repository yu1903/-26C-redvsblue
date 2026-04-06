//cell.h

#ifndef CELL_H
#define CELL_H


class StartWindow;  // 前置声明

#include <QMainWindow>
#include <QPainter>
#include <QTimer>
#include <QMessageBox>
#include <QVector>
#include <QMouseEvent>

// 前置声明卡槽管理类
class PatternSlot;

QT_BEGIN_NAMESPACE
namespace Ui {
class cell;
}
QT_END_NAMESPACE

// 细胞类型枚举
enum CellType {
    EMPTY,      // 空细胞
    BLUE_CELL,  // 蓝色细胞
    RED_CELL,   // 红色细胞
    PURPLE_CELL // 紫色细胞
};

// 旋转方向枚举
enum RotationDirection {
    UP_RIGHT,   // 右上（蓝方优先）
    DOWN_RIGHT, // 右下（蓝方次优先）
    UP_LEFT,    // 左上（红方优先）
    DOWN_LEFT   // 左下（红方次优先）
};

// 旗帜状态结构体
struct Flag {
    int x;          // 网格X坐标
    int y;          // 网格Y坐标
    bool isAlive;   // 旗帜是否存在
};

// 细胞组合结构体
struct CellPattern {
    QString name;                // 组合名称
    QVector<QPoint> cellPositions; // 组合内细胞的相对坐标（以中心点为(0,0)）
    int energyCost;              // 该组合的能量消耗（细胞数*2）
    int priority;                // 组合优先级（枪:4, 飞船:3, 振荡/稳定:2）
};

// ===================== 新增：清除卡牌结构体 =====================
struct ClearCard {
    int size;           // 清除范围 N*N
    int energyCost;     // 消耗能量
    int cooldown;       // 总冷却代
    int remain;         // 剩余冷却
    QString text;       // 显示文字 清除5~10
};

class cell : public QMainWindow
{
    Q_OBJECT

public:
    explicit cell(QWidget *parent = nullptr);
    ~cell() override;
    void paintEvent(QPaintEvent *event) override;
    // 重写鼠标点击事件，实现卡槽/按钮/放置交互
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;    // 鼠标拖动
    void mouseReleaseEvent(QMouseEvent *event) override; // 鼠标释放

    // ===================== 手动放置与界面控制 =====================
    PatternSlot *slotManager;        // 卡槽管理对象
    bool isPaused;                    // 游戏暂停状态
    int generationCount;              // 细胞迭代代数

    // 卡槽UI尺寸常量
    const int SLOT_WIDTH = 80;
    const int SLOT_HEIGHT = 60;
    const int SLOT_MARGIN = 10;

    // 按钮矩形区域
    QRect pauseButtonRect;            // 暂停按钮位置
    QRect rotateButtonRect;           // 旋转按钮位置

private slots:
    // 定时器槽函数，更新游戏状态
    void updateGameState();

private:
    Ui::cell *ui;

    // 游戏区域常量
    static const int GAME_WIDTH = 1600;    // 游戏区域宽度
    static const int GAME_HEIGHT = 960;    // 游戏区域高度
    static const int CELL_WIDTH = 8;       // 细胞/旗帜宽度
    static const int CELL_HEIGHT = 12;     // 细胞/旗帜高度

    // 计算网格行列数
    static const int GRID_ROWS = GAME_HEIGHT / CELL_HEIGHT;
    static const int GRID_COLS = GAME_WIDTH / CELL_WIDTH;

    // 旗帜相关常量
    static const int FLAG_COUNT = 5;       // 每方旗帜数量

    // 细胞状态数组（当前代和下一代）
    CellType currentGrid[GRID_ROWS][GRID_COLS];
    CellType nextGrid[GRID_ROWS][GRID_COLS];

    // 旗帜数组
    Flag blueFlags[FLAG_COUNT];            // 蓝色旗帜
    Flag redFlags[FLAG_COUNT];             // 红色旗帜

    // 图片资源
    QPixmap blueCellImg;
    QPixmap redCellImg;
    QPixmap purpleCellImg;
    QPixmap flagImg;
    QPixmap backgroundImg;

    // 游戏控制
    QTimer *gameTimer;
    bool gameOver;                         // 游戏是否结束
    QString winner;                        // 获胜方

    // 新增：细胞能量相关
    int blueEnergy;                        // 蓝方能量
    int redEnergy;                         // 红方能量
    const int INITIAL_ENERGY = 30;        // 初始能量
    const int ENERGY_PER_ITERATION = 1;    // 每次迭代增加的能量
    const int ENERGY_PER_CELL = 1;         // 己方细胞在紫色区的能量加成
    const int ENERGY_PER_CELL_PLACEMENT = 5; // 放置单个细胞消耗的能量

    // 新增：细胞组合模板
    QVector<CellPattern> bluesingleLifePatterns;    // 蓝方单细胞组合
    QVector<CellPattern> blueStillLifePatterns;    // 蓝方稳定型组合
    QVector<CellPattern> blueOscillatorPatterns;   // 蓝方振荡型组合
    QVector<CellPattern> blueSpaceshipPatterns;    // 蓝方飞船型组合
    QVector<CellPattern> blueGunPatterns;          // 蓝方枪型组合

    QVector<CellPattern> redStillLifePatterns;     // 红方稳定型组合
    QVector<CellPattern> redOscillatorPatterns;    // 红方振荡型组合
    QVector<CellPattern> redSpaceshipPatterns;     // 红方飞船型组合
    QVector<CellPattern> redGunPatterns;           // 红方枪型组合

    // ========== 计分系统 ==========
    int blueScore;
    int redScore;
    void updateScore();        // 更新分数
    int countAliveCells(CellType type); // 统计指定颜色存活细胞数

    // 初始化函数
    void initGrid();                       // 初始化细胞网格（全空）
    void initFlags();                      // 初始化旗帜位置
    void initCellPatterns();               // 初始化细胞组合模板
    void initEnergy();                     // 初始化能量

    // 统计函数
    int countAllAliveCells(int x, int y);  // 统计周围所有活细胞数（红蓝紫）
    int countColorAliveCells(int x, int y, CellType color); // 统计指定颜色细胞数
    int countOwnCellsInPurpleZone(CellType color); // 统计紫色区己方细胞数

    // 游戏规则
    void calculateNextGeneration();        // 计算下一代细胞
    void checkFlagOccupation();            // 检查旗帜是否被占领
    bool checkGameOver();                  // 检查游戏是否结束

    // 细胞组合相关
    bool placeCellPattern(int centerX, int centerY, const CellPattern& pattern, CellType cellType, RotationDirection dir = UP_RIGHT); // 放置细胞组合（带旋转）
    void randomPlacePatterns();            // 随机放置细胞组合（按优先级）
    bool isPlacementValid(int x, int y, const CellPattern& pattern, CellType cellType, RotationDirection dir); // 检查放置是否合法（带旋转）
    QVector<QPoint> rotatePattern(const QVector<QPoint>& original, RotationDirection dir); // 旋转组合坐标
    QVector<CellPattern> getPatternsByPriority(CellType cellType); // 按优先级排序获取组合

    // 绘制函数
    void drawCells(QPainter &painter);     // 绘制细胞
    void drawFlags(QPainter &painter);     // 绘制旗帜
    void updateEnergy();                   // 更新能量值

    // 新增UI绘制
    void drawPatternSlots(QPainter& painter);  // 绘制左侧卡槽
    void drawPauseButton(QPainter& painter);   // 绘制暂停按钮
    void drawGenerationInfo(QPainter& painter);// 绘制细胞代数
    void drawScoreInfo(QPainter& painter);   // 绘制分数
    bool findNearestValidPos(int& cx, int& cy, const CellPattern& pat, RotationDirection dir); // 寻找最近可放置位置

    // ===================== 新增：清除细胞卡牌系统 =====================
    QVector<ClearCard> m_clearCards;
    void initClearCards();                  // 初始化清除卡牌
    void drawClearSlots(QPainter& painter); // 绘制清除卡牌UI
    void updateClearCooldown();             // 每代更新冷却
    bool useClearCard(int idx, int cx, int cy); // 使用清除卡牌

    int selectedClearCardIndex = -1;//是否点击清除细胞卡片

public:
    const QVector<CellPattern>& getBlueStillLifePatterns() const { return blueStillLifePatterns; }
    const QVector<CellPattern>& getBlueOscillatorPatterns() const { return blueOscillatorPatterns; }
    const QVector<CellPattern>& getBlueSpaceshipPatterns() const { return blueSpaceshipPatterns; }
    const QVector<CellPattern>& getBlueGunPatterns() const { return blueGunPatterns; }
};

#endif // CELL_H