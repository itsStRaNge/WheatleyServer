#ifndef SERVER_H
#define SERVER_H
#include <QObject>
#include <QTcpServer>
#include <QMap>
#include "RCSwitch.h"
#define RED_PIN 24//24
#define BLUE_PIN 23//23
#define GREEN_PIN 18//18
#define ON_RASPBERRY 1
#define NUM_OF_SOCKETS 5
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

    enum{
        SOCKET_OFF = 0,
        SOCKET_ON = 1
    };
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
