#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>

class HelpDialog;  // 前置声明

// 初始主界面窗口类
class StartWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit StartWindow(QWidget *parent = nullptr);

private slots:
    void onStartClicked();  // 开始游戏按钮
    void onHelpClicked();   // 帮助按钮

private:
    // 组件
    QPushButton *btnStart;
    QPushButton *btnHelp;
    QLabel *labelBlue;      // 蓝色细胞预览
    QLabel *labelRed;       // 红色细胞预览
    QLabel *labelPurple;    // 紫色细胞预览

    // 初始化界面
    void initUI();
};

#endif // STARTWINDOW_H