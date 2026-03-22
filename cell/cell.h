#ifndef CELL_H
#define CELL_H

#include <QMainWindow>
#include <QPainter>
#include <QTimer>
#include <QMessageBox>

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

// 旗帜状态结构体
struct Flag {
    int x;          // 网格X坐标
    int y;          // 网格Y坐标
    bool isAlive;   // 旗帜是否存在
};

class cell : public QMainWindow
{
    Q_OBJECT

public:
    explicit cell(QWidget *parent = nullptr);
    ~cell() override;
    void paintEvent(QPaintEvent *event) override;

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

    // 初始化函数
    void initGrid();                       // 初始化细胞网格
    void initFlags();                      // 初始化旗帜位置

    // 统计函数
    int countAllAliveCells(int x, int y);  // 统计周围所有活细胞数（红蓝紫）
    int countColorAliveCells(int x, int y, CellType color); // 统计指定颜色细胞数

    // 游戏规则
    void calculateNextGeneration();        // 计算下一代细胞
    void checkFlagOccupation();            // 检查旗帜是否被占领
    bool checkGameOver();                  // 检查游戏是否结束

    // 绘制函数
    void drawCells(QPainter &painter);     // 绘制细胞
    void drawFlags(QPainter &painter);     // 绘制旗帜
};

#endif // CELL_H