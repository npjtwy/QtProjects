#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTextBrowser>
#include <QFontComboBox>
#include <QComboBox>
#include <QTextLine>
#include <QColor>
#include <QToolButton>
#include <QString>
#include <QLineEdit>

//聊天窗口类

class ToolBox1;
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(const QIcon icon, int ID, QString name,
           ToolBox1 *w, QWidget *parent = 0);
    ~Widget();
    QPushButton *pushButton;
    //弹出对话框内容
    void add_msg(QString delivername, QString msg);
private:
    QTextBrowser *textBrowser;
    QFontComboBox *fontComboBox;
    QComboBox *comboBox;

    QToolButton *toolButton_1;
    QToolButton *toolButton_2;
    QToolButton *toolButton_3;
    QToolButton *toolButton_4;
    QToolButton *toolButton;

    QLineEdit *lineEdit;

    int userID;
    QString userName;
    QColor color;
    ToolBox1 *main_w;

    void init_widget();

private slots:
    //发送按钮槽函数
    void on_pushButton_clicked();
    //修改字体槽函数
    void on_fontComboBox_currentFontChanged(const QFont &f);
    void on_toolButton_clicked();
    void on_toolButton_4_clicked();//修改textBrowser字体颜色
    void on_toolButton_3_clicked(bool checked);//修改textBrowser字体是否下划线
    void on_toolButton_2_clicked(bool checked);//修改textBrowser字体是否斜体
    void on_toolButton_1_clicked(bool checked);//修改textBrowser字体是否加粗
    void on_comboBox_currentIndexChanged(const QString &arg1);//修改textBrowser字号
    void on_lineEdit_returnPressed();


};

#endif // WIDGET_H
