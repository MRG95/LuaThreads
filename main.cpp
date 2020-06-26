#include <QCoreApplication>
#include "worker.h"
#include <QtCore>
#include <QObject>
#include <iostream>
#include <QDebug>
#include <QElapsedTimer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qRegisterMetaType<Tag::Type>("TagType");
    qRegisterMetaType<Tag*>("Tag");
    qRegisterMetaType<qint8>("qint8");
    qRegisterMetaType<quint8>("quint8");
    qRegisterMetaType<qint16>("qint16");
    qRegisterMetaType<quint16>("quint16");
    qRegisterMetaType<qint32>("qint32");
    qRegisterMetaType<quint32>("quint32");
    qRegisterMetaType<qint64>("qint64");
    qRegisterMetaType<quint64>("quint64");

    QVector<Worker*> workers;

    int threadCount = 1;

    std::cout << "How many threads do you want to run? ";
    std::cin >> threadCount;
    system("cls");

    std::cout << "Simulating work with " << threadCount << " threads..." << std::endl << std::endl;

    //create workers
    for(int i=0; i<threadCount; i++)
    {
        Worker *worker = new Worker;

        //setup script here
        worker->script = new LuaScript("testScript.lua");
        if(!worker->script->start())
        {
            qDebug() << "ERROR-Script could not start!";
            return 1;
        }

        workers.append(worker);
    }

    QElapsedTimer timer;
    timer.start();

    //simulate work
    for(int i=0; i<1000; i++)
    {
        bool foundWork(false);
        while(!foundWork)
        {
            for(int w=0; w<workers.size(); w++)
            {
                Worker *worker = workers.at(w);

                if(worker->working)
                    continue;

                worker->invokeWork();

                foundWork = true;
                break;
            }
        }
    }

    //wait for all workers to finish
    QEventLoop loop;
    for(int w=0; w<workers.size(); w++)
    {
        Worker *worker = workers.at(w);

        QObject::connect(worker, SIGNAL(stopped()), &loop, SLOT(quit()));
        if(worker->working)
        {
            loop.exec();
        }
        QObject::disconnect(worker, SIGNAL(stopped()), &loop, SLOT(quit()));
    }

    std::cout << "Finished in " << timer.elapsed()/1000.0 << " seconds";

    //delete workers
    for(int i=threadCount-1; i>=0; i--)
    {
        delete workers.at(i);
    }
    workers.clear();

    return a.exec();
}
