#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <errno.h>
#include <fstream>
#include <cmath>
#include <QDebug>
#include <iostream>
#include <stdio.h>
#include <QElapsedTimer>

int *minElements;
#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

char STUDENTDATA[] = "Yaroslav Kachmar 12700969\n";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    //delete DTI;
    //delete TTI;
    delete ui;
}

void* threadStartNfunc(void* arg){
    std::fstream fRes;
    fRes.open("studentInfo.txt", std::fstream::out | std::fstream::app);
    if(fRes.is_open()){
        default_thread_info* DTI = (default_thread_info*)arg;
        for(int i = 0; i < DTI->iter_num; i++)
        {
            for(int k = 0; k < 26; k++)
            {
                fRes << (STUDENTDATA[k]);
            }
        }
       fRes.close();
    }else{
        perror("File could not be opened");
    }
    return (void*)0;
}

void* threadStartTfunc(void* arg){

    std::fstream fRes;
    fRes.open("tabulation.txt", std::fstream::out | std::fstream::app);
    if(fRes.is_open())
    {
        tabulation_data* TTI = (tabulation_data*)arg;

        double A = TTI->a_border, B = TTI->b_border, steps_quantity = TTI->iter_num;
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

void* threadStartSfunc(void* arg){
    search_data* STI = (search_data*)arg;
    int minElem = STI->array[STI->startingPoint];
    for(int i = STI->startingPoint + 1; i < STI->endPoint; ++i)
    {
        if(minElem > STI->array[i])
        minElem = STI->array[i];
    }
    std::fstream fRes;
    fRes.open("search.txt", std::ios::out | std::ios::app);
    if(fRes.is_open()){
        for(int i = STI->startingPoint; i < STI->endPoint; ++i)
            fRes << STI->array[i] << std::endl;
        fRes.close();
    }else{
        perror("File could not be opened");
    }
    minElements[STI->thread] = minElem;

    return (void*)0;
}

void MainWindow::on_CreateThreads_clicked()
{
    int num_of_iterations = ui->steps->toPlainText().toInt(), num_of_threads = ui->numOfThreads->currentText().toInt();
    int steps_division = num_of_iterations/num_of_threads;
    double tabulation_division = 0.8/num_of_threads;
    double a_border = 0.1, b_border;

    minElements = new int[num_of_threads];
    int startingPoint = 0, endpoint = steps_division;
    int *array = new int[num_of_iterations];
    for(int i = 0 ; i < num_of_iterations; i++)
        array[i] = rand() % 1000;

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

    pthread_t handles[num_of_threads];
    sched_param param;

    int priority, policy;


    for(int tnum = 0; tnum < num_of_threads; tnum++)
    {
        int r;
        switch(ui->task->currentIndex())
        {
            case 0:
            {
                default_thread_info *DTI = new default_thread_info();
                DTI->iter_num = steps_division;
                r = pthread_create( &handles[tnum], NULL, threadStartNfunc, (void*)DTI);
                break;
            }
            case 1:
            {
                tabulation_data *TTI = new tabulation_data();
                b_border = a_border + tabulation_division;
                TTI->iter_num = steps_division;
                TTI->a_border = a_border;
                TTI->b_border = b_border;
                r = pthread_create( &handles[tnum], NULL, threadStartTfunc, (void*)TTI);
                a_border = b_border;
                break;
            }
            case 2:
            {
                search_data *STI = new search_data();
                STI->array = array;
                STI->iter_num = num_of_iterations;
                STI->startingPoint = startingPoint;
                STI->endPoint = endpoint;
                STI->thread = tnum;
                r = pthread_create( &handles[tnum], NULL, threadStartSfunc, (void*)STI);
                startingPoint += steps_division;
                endpoint += steps_division;
                break;
            }
            default: r = 0;
        }
        if(r != 0) handle_error_en(r, "pthread_create");

        pthread_getschedparam(handles[tnum], &policy, &param);
        priority = param.sched_priority;

        ui->listOfThreads->setItem(tnum,0,new QTableWidgetItem(QString::number(handles[tnum])));
        ui->listOfThreads->setItem(tnum,1,new QTableWidgetItem(QString::number(priority)));
        ui->listOfThreads->setItem(tnum,2,new QTableWidgetItem("Running"));
    }
    pthread_attr_destroy(&attr);

    QElapsedTimer timer;
    timer.start();
    for(int tnum = 0; tnum < num_of_threads; tnum++)
    {
        pthread_join(handles[tnum], NULL);
        ui->listOfThreads->item(tnum, 2)->setText("Finished");
    }
    timer.nsecsElapsed();

    if(ui->task->currentIndex() == 2)
    {
        std::fstream fRes;
        fRes.open("search.txt", std::ios::out | std::ios::app);
        if(fRes.is_open()){
            int minimal_element = minElements[0];
            for(int i = 1; i < num_of_threads; i++)
                if(minimal_element > minElements[i])
                    minimal_element = minElements[i];
            fRes << std::endl;
            fRes << "Minimal element : " << minimal_element << std::endl;
            fRes.close();
        }else{
            perror("File could not be opened");
        }
    }
    ui->time->setText(QString::number(timer.nsecsElapsed()/1000000.0) + " ms");

}
