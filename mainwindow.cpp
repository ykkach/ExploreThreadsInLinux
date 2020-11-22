#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <errno.h>
#include <fstream>
#include <cmath>
#include <QDebug>
#include <iostream>
#include <stdio.h>
#include <QElapsedTimer>

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

char STUDENTDATA[] = "Yaroslav Kachmar 12700969\n";
std::string PATH = "/home/yaroslav/student/build-ThreadsInLinux-Desktop-Debug/";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete STI;
    delete ui;
}

void* threadStartNfunc(void* arg){
    std::ofstream out(PATH + "studentData.txt");
    std::streambuf *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());

    MainWindow::thread_info* TI = (MainWindow::thread_info*)arg;
    for(int i = 0; i < TI->iter_num; i++)
    {
        for(int k = 0; k < 26; k++)
        {
            std::cout << (STUDENTDATA[k]);
        }
    }
    std::cout.rdbuf(coutbuf);
    return (void*)0;
}

void* threadStartTfunc(void* arg){

    std::fstream fRes;
    std::string filePath = PATH + "tabulation.txt";
    fRes.open(filePath, std::ios::out | std::ios::app);
    if(fRes.is_open())
    {
        MainWindow::thread_info* TI = (MainWindow::thread_info*)arg;

        double A = TI->tab_segment.a_border, B = TI->tab_segment.b_border, steps_quantity = TI->iter_num;
        double eps = 0.001, total{0}, holder{1};
        for(double x = A, step = (B-A)/steps_quantity; x < B; x+= step)
        {
            int sign_ind = 0;
            holder = 1;
            total = 0;
            while(abs(holder) >= eps)
            {
                total += pow(-1, sign_ind)*holder;
                sign_ind++;
                holder = pow(x, sign_ind);
            }
            fRes << x << ' ' << total << ' ' << pow(1+x, -1) << std::endl;
        }
        fRes.close();
    }else{
        perror("File could not be opened");
    }
    return (void*)0;
}

void MainWindow::on_CreateThreads_clicked()
{
    int num_of_iterations = ui->steps->toPlainText().toInt(), num_of_threads = ui->numOfThreads->currentText().toInt();
    int steps_division = num_of_iterations/num_of_threads;
    double tabulation_division = 0.8/num_of_threads;
    double a_border = 0.1, b_border;
    ui->listOfThreads->setRowCount(num_of_threads);
    ui->listOfThreads->setColumnCount(3);
    ui->listOfThreads->setHorizontalHeaderItem(0, new QTableWidgetItem("Thread ID"));
    ui->listOfThreads->setHorizontalHeaderItem(1, new QTableWidgetItem("Priority"));
    ui->listOfThreads->setHorizontalHeaderItem(2, new QTableWidgetItem("State"));
    ui->listOfThreads->setColumnWidth(0, 150);
    ui->listOfThreads->setColumnWidth(1, 80);
    ui->listOfThreads->setColumnWidth(2, 120);
    int stack_size = 1024;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, stack_size);

    STI = new thread_info[num_of_threads];
    sched_param param;

    int priority, policy;
    for(int tnum = 0; tnum < num_of_threads; tnum++)
    {
        STI[tnum].thread_num = tnum + 1;
        STI[tnum].iter_num = steps_division;
        b_border = a_border + tabulation_division;
        STI[tnum].tab_segment.a_border = a_border;
        STI[tnum].tab_segment.b_border = b_border;
        int r;
        ui->listOfThreads->setItem(tnum,2,new QTableWidgetItem("Running"));
        switch(ui->task->currentIndex())
        {
            case 0:
            {
                r = pthread_create( &STI[tnum].thread_id, NULL, threadStartNfunc, (void*)&STI[tnum]);
                break;
            }
            case 1:
            {
                r = pthread_create( &STI[tnum].thread_id, NULL, threadStartTfunc, (void*)&STI[tnum]);
                break;
            }
            default: r = 0;
        }
        if(r != 0) handle_error_en(r, "pthread_create");

        pthread_getschedparam(STI[tnum].thread_id, &policy, &param);
        priority = param.sched_priority;

        a_border = b_border;

        ui->listOfThreads->setItem(tnum,0,new QTableWidgetItem(QString::number(STI[tnum].thread_id)));
        ui->listOfThreads->setItem(tnum,1,new QTableWidgetItem(QString::number(priority)));
        ui->listOfThreads->setItem(tnum,2,new QTableWidgetItem("Finished"));
    }
    pthread_attr_destroy(&attr);

    QElapsedTimer timer;
    timer.start();
    for(int tnum = 0; tnum < num_of_threads; tnum++)
    {
        pthread_join(STI[tnum].thread_id, NULL);
    }
    timer.nsecsElapsed();
    ui->time->setText(QString::number(timer.nsecsElapsed()/1000000.0) + " ms");

}
