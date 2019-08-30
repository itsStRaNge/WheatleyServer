#ifndef SERVER_H
#define SERVER_H
#include <QObject>
#include <QTcpServer>
#include <QMap>
#include "RCSwitch.h"
#define ON_RASPBERRY 1

#if (ON_RASPBERRY == 1)
    #include <wiringPi.h>
    #include <softPwm.h>
#endif



class server : public QTcpServer
{
    Q_OBJECT
public:
    explicit server(QObject *parent = 0);
    void setSocket(int, int);

protected:
    int port;
    void incomingConnection(qintptr socketDescriptor);
signals:

private:
    qint8 red,blue,green;
    int makeNumberValid(int);
    int SOCKET_PIN;
    RCSwitch mySwitch;
    QTcpSocket *current_client;

public slots:
    void readyRead();
    void disconnected();
};

#endif // SERVER_H
