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

    struct thread_info {
        pthread_t thread_id;
        int thread_num;
        int iter_num;
        struct tabulation_data_segment
        {
            double a_border;
            double b_border;
        }tab_segment;
    };

private slots:
    void on_CreateThreads_clicked();

private:

    Ui::MainWindow *ui;
    thread_info *STI;
};

#endif // MAINWINDOW_H
