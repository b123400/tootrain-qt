#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "settingwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void preferencesTriggered(bool check);

private:
    SettingWindow *settingWindow = nullptr;
};
#endif // MAINWINDOW_H
