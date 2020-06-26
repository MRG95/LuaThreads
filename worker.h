#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QThread>
#include "Lua/lua_script.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr) : QObject(parent), thread(new QThread), working(false)
    {
        moveToThread(thread);
        thread->start();

        connect(this, &Worker::startWork, this, &Worker::doWork);
        connect(this, SIGNAL(finished()), this, SLOT(catchFinished()));
    }

    ~Worker(){
        if(thread->isRunning())
        {
            thread->quit();
            thread->wait();
        }

        script->collectGarbage();
        delete script;
    }

    QThread *thread;
    bool working;
    LuaScript *script;

public slots:

    void doWork()
    {
        TagCompound *inputData = new TagCompound();
        TagCompound *outputData;

        //CALL SCRIPT FUNCTION HERE
        Tag* returnTag;
        QGenericReturnArgument returnArg = Q_RETURN_ARG(Tag*, returnTag);
        script->CallLuaFunction("TestWork", returnArg, Q_ARG(Tag*, inputData));

        outputData = qobject_cast<TagCompound*>(returnTag);

        //no memory clearing is necessary because lua now controls the memory

        emit finished();
    }

    void invokeWork()
    {
        working = true;

        emit startWork();
    }

private slots:
    void catchFinished()
    {
        working = false;

        emit stopped();
    }

signals:

    void startWork();

    void stopped();

    void finished();

};

#endif // WORKER_H
