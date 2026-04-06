#include "PatternSlot.h"

PatternSlot::PatternSlot(QObject *parent)
    : QObject{parent}
    , selectedIndex(-1)        // 初始未选中任何卡片
    , currentRotate(DOWN_RIGHT)// 默认方向
    , scrollOffset(0)          // 滚动初始位置
    , dragStartY(0)
    , isDragging(false)
{}

// 加载所有蓝方细胞组合
void PatternSlot::initAllPatterns(const cell* game)
{
    allBluePatterns.clear();
    allBluePatterns.append(game->getbluesingleLifePatterns());
    allBluePatterns.append(game->getBlueStillLifePatterns());
    allBluePatterns.append(game->getBlueOscillatorPatterns());
    allBluePatterns.append(game->getBlueSpaceshipPatterns());
    allBluePatterns.append(game->getBlueGunPatterns());
}

// 循环旋转当前选中图案方向
void PatternSlot::rotateSelected()
{
    // 无选中则直接返回
    if (selectedIndex < 0 || selectedIndex >= allBluePatterns.size())
        return;

    // 四个方向循环切换
    switch (currentRotate) {
    case UP_RIGHT:
        currentRotate = DOWN_RIGHT;
        break;
    case DOWN_RIGHT:
        currentRotate = DOWN_LEFT;
        break;
    case DOWN_LEFT:
        currentRotate = UP_LEFT;
        break;
    case UP_LEFT:
        currentRotate = UP_RIGHT;
        break;
    }
}