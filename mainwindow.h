#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QTextBlock>
#include "cpu.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_openFileAction_triggered();
    void on_nextStepButton_clicked();

    void on_pushButton_clicked();

    void on_aboutAction_triggered();

    void on_actionAboutQt_triggered();

private:
    void updateState(int mode = 1);
    Ui::MainWindow *ui;
    QString fileName;
    QByteArray program;
    QFile file;
    CPU *cpu;
    int lineNumber;
};

#endif // MAINWINDOW_H
