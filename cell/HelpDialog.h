#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>

class HelpDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HelpDialog(QWidget *parent = nullptr);

private:
    QTextEdit *textEdit;    // 富文本+滚动区域
    QPushButton *btnClose;  // 关闭按钮
};

#endif // HELPDIALOG_H