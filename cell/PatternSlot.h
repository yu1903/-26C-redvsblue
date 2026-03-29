#ifndef PATTERNSLOT_H
#define PATTERNSLOT_H

#include <QObject>
#include <QPoint>
#include <QVector>
#include "cell.h"

// 卡槽管理类：管理所有蓝方细胞模板、选中状态、旋转方向
class PatternSlot : public QObject
{
    Q_OBJECT
public:
    explicit PatternSlot(QObject *parent = nullptr);

    // 所有蓝方可放置的细胞组合
    QVector<CellPattern> allBluePatterns;

    // 当前选中卡片索引，-1表示未选中
    int selectedIndex;
    // 当前选中图案的旋转方向
    RotationDirection currentRotate;

    // ========== 新增：卡槽滚动 ==========
    int scrollOffset;       // 滚动偏移量
    int dragStartY;         // 拖动起始Y
    bool isDragging;        // 是否正在拖动

    // 从游戏对象加载所有蓝方模板
    void initAllPatterns(const cell* game);
    // 旋转当前选中的图案
    void rotateSelected();
};

#endif // PATTERNSLOT_H