#ifndef OSERVERCORE_H
#define OSERVERCORE_H

#include <QObject>
#include <QTextStream>
#include <QTcpServer>
#include "../public/OSettings.h"
#include "global.h"
#include "OClient.h"
#include "OProtocolForSC.h"

class OServerCore:public QObject
{
    Q_OBJECT
public:
    explicit OServerCore();
    void init();
    void start();
    void checkMsg(OClient::Connect *connect);
private:
    void log(QString msg);
    QMap<QString,OClient*> cl;
    QTextStream cin;
    QTextStream cout;
    QTcpServer server;
    OSettings config;
    OProtocolForSC protocol;
signals:

private slots:
    void onNewConn();
    void onError(OClient::Connect *connect,QString msg,QAbstractSocket::SocketError s);
};

#endif // OSERVERCORE_H