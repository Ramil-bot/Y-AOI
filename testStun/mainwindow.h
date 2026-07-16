#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWindget>
#include <QPushButton>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

signals:
    void widgetCicked();

private:
    QPushButton *myButton;
};
#endif // MAINWINDOW_H

