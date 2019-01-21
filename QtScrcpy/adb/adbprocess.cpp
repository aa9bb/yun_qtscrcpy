#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>
#include "adbprocess.h"

QString AdbProcess::s_adbPath="";
AdbProcess::AdbProcess(QObject *parent)
    :QProcess(parent)
{
    initSignals();
    getAdbPath();
}

QString AdbProcess::getAdbPath()
{
    if(s_adbPath.isEmpty()){
        s_adbPath=QString::fromLocal8Bit(qgetenv("QTSCRCPY_ADB_PATH"));
        QFileInfo fileInfo(s_adbPath);
        if(s_adbPath.isEmpty() || !fileInfo.isFile()){
            s_adbPath=QCoreApplication::applicationDirPath()+"/adb";
        }
    }
    return s_adbPath;
}

void AdbProcess::execute(const QString &serial, const QStringList &args)
{
    QStringList adbArgs;
    if(!serial.isEmpty()){
        adbArgs<<"-s"<<serial;
    }
    adbArgs<<args;
    qDebug()<<getAdbPath()<<adbArgs.join(" ");
    start(getAdbPath(),adbArgs);
}

void AdbProcess::push(const QString &serial, const QString &local, const QString &remote)
{
    QStringList adbArgs;
    adbArgs<<"push";
    adbArgs<<local;
    adbArgs<<remote;
    execute(serial,adbArgs);
}

void AdbProcess::removePath(const QString &serial, const QString &path)
{
    QStringList adbArgs;
    adbArgs<<"shell";
    adbArgs<<"rm";
    adbArgs<<path;
    execute(serial,adbArgs);
}

void AdbProcess::reverse(const QString &serial, const QString &deviceSocketName, quint16 localPort)
{
    QStringList adbArgs;
    adbArgs<<"reverse";
    adbArgs<<QString("localabstract:%1").arg(deviceSocketName);
    adbArgs<<QString("tcp:%1").arg(localPort);
    execute(serial,adbArgs);
}

void AdbProcess::reverseRemove(const QString &serial, const QString &deviceSocketName)
{
    QStringList adbArgs;
    adbArgs<<"reverse";
    adbArgs<<"--remove";
    adbArgs<<QString("localabstract:%1").arg(deviceSocketName);
    execute(serial,adbArgs);
}

void AdbProcess::initSignals()
{
    connect(this,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){
        if(QProcess::FailedToStart==error)
            emit adbProcessResult(AER_ERROR_MISSING_BINARY);
        else
            emit adbProcessResult(AER_ERROR_START);
        qDebug()<<error;
    });

    connect(this, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
         this,[this](int exitCode, QProcess::ExitStatus exitStatus){
        if(QProcess::NormalExit==exitStatus && exitCode==0)
            emit adbProcessResult(AER_SUCCESS_EXEC);
        else
            emit adbProcessResult(AER_ERROR_EXEC);
        qDebug()<<exitCode<<exitStatus;
    });

    connect(this,&QProcess::readyReadStandardError,this,[this](){
        qDebug()<<readAllStandardError();
    });
    connect(this,&QProcess::readyReadStandardOutput,this,[this](){
        qDebug()<<readAllStandardOutput();
    });
    connect(this,&QProcess::started,this,[this](){
         emit adbProcessResult(AER_SUCCESS_START);
    });
}