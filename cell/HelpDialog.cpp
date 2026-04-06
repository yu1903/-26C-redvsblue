#include "HelpDialog.h"

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("游戏帮助");
    setFixedSize(800, 600);   // 帮助窗口大小

    // 布局
    QVBoxLayout *layout = new QVBoxLayout(this);

    // 富文本编辑器（支持滚动、多行、HTML格式）
    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);  // 只读
    textEdit->setStyleSheet("font-size:16px; padding:10px;");

    // 帮助内容（后续你可以随便加，支持换行、HTML、长文本）
    textEdit->setHtml(R"(
        <h2>游戏规则说明</h2>
        <p>这是一个帮助内容，你可以在这里填写：</p>
        <p>1. 游戏目标</p>
        <p>2. 操作方法</p>
        <p>3. 细胞类型介绍</p>
        <p>4. 能量与得分规则</p>
        <p>5. 胜利条件</p>
        <p>6. 其他说明...</p>
        <p>支持无限长文本，自动出现滚动条。</p>
    )");

    // 关闭按钮
    btnClose = new QPushButton("关闭", this);
    btnClose->setStyleSheet("font-size:16px; padding:6px 20px;");
    connect(btnClose, &QPushButton::clicked, this, &QDialog::close);

    layout->addWidget(textEdit);
    layout->addWidget(btnClose, 0, Qt::AlignCenter);
}