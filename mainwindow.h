#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <pthread.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_CreateThreads_clicked();

private:

    Ui::MainWindow *ui;
};
struct default_thread_info {
    int iter_num;
};
struct tabulation_data
{
    int iter_num;
   double a_border;
   double b_border;
};
struct search_data
{
    int iter_num;
   int *array;
   int startingPoint;
   int endPoint;
   int thread;
};
#endif // MAINWINDOW_H
