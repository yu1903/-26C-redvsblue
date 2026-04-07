#include "StartWindow.h"
#include "cell.h"
#include "HelpDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include<QPainter>
//#include <QTimer>

StartWindow::StartWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置窗口大小与标题
    this->setFixedSize(1700, 990);
    this->setWindowTitle("细胞游戏 - 主界面");

    // 初始化UI
    initUI();
}

// 初始化主界面UI
void StartWindow::initUI()
{

    this->setStyleSheet("QMainWindow { background-color: white; }");

    // 中心部件 + 主布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(50);
    mainLayout->setContentsMargins(100, 100, 100, 150);
    this->setCentralWidget(centralWidget);

    // ===================== 顶部：三种细胞预览（放大20倍） =====================
    QHBoxLayout *cellLayout = new QHBoxLayout();
    cellLayout->setSpacing(80);
    cellLayout->setAlignment(Qt::AlignCenter);

    // 蓝色细胞
    labelBlue = new QLabel(this);
    QPixmap bluePix(":/images/Image/blue1.bmp");

    labelBlue->setPixmap(bluePix.scaled(8*20, 12*20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    labelBlue->setAlignment(Qt::AlignCenter);

    // 红色细胞
    labelRed = new QLabel(this);
    QPixmap redPix(":/images/Image/red1.bmp");

    labelRed->setPixmap(redPix.scaled(8*20, 12*20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    labelRed->setAlignment(Qt::AlignCenter);

    // 紫色细胞
    labelPurple = new QLabel(this);
    QPixmap purplePix(":/images/Image/purple1.bmp");

    labelPurple->setPixmap(purplePix.scaled(8*20, 12*20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    labelPurple->setAlignment(Qt::AlignCenter);

    cellLayout->addWidget(labelBlue);
    cellLayout->addWidget(labelPurple);
    cellLayout->addWidget(labelRed);

    // ===================== 底部：按钮区域 =====================
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(60);
    btnLayout->setAlignment(Qt::AlignCenter);

    // 开始按钮
    btnStart = new QPushButton("START", this);
    btnStart->setFixedSize(220, 80);
    btnStart->setStyleSheet("font-size:28px; font-weight:bold;");
    connect(btnStart, &QPushButton::clicked, this, &StartWindow::onStartClicked);

    // 帮助按钮
    btnHelp = new QPushButton("HELP", this);
    btnHelp->setFixedSize(220, 80);
    btnHelp->setStyleSheet("font-size:28px; font-weight:bold;");
    connect(btnHelp, &QPushButton::clicked, this, &StartWindow::onHelpClicked);

    btnLayout->addWidget(btnStart);
    btnLayout->addWidget(btnHelp);

    // ===================== 组装主布局 =====================
    mainLayout->addLayout(cellLayout);
    mainLayout->addLayout(btnLayout);
}

// 点击START：关闭主界面，打开游戏界面
void StartWindow::onStartClicked()
{
            this->close();          // 关闭启动界面
            cell *gameWin = new cell; // 创建游戏窗口
            gameWin->show();        // 显示游戏
}

// 点击HELP：打开自定义滚动帮助窗口
void StartWindow::onHelpClicked()
{
    HelpDialog dlg(this);
    dlg.exec();  // 模态弹窗
}