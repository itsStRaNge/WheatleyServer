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


typedef struct {
    int bright;
    int pin;
}colorPins;
class server : public QTcpServer
{
    Q_OBJECT
public:
    explicit server(QObject *parent = 0);
    void setColor(int,int);
    qint8 getRed();
    qint8 getGreen();
    qint8 getBlue();
    void toggleSocket(int);
    void socketsOff();
    void socketsOn();
protected:
    int port;
     void incomingConnection(qintptr socketDescriptor);
signals:

private:
    qint8 red,blue,green;
    int makeNumberValid(int);
    int SOCKET_PIN;
    QStringList commands;
    RCSwitch mySwitch;
    colorPins list[3];
    bool socketState[NUM_OF_SOCKETS];

public slots:
    void readyRead();
    void disconnected();
};

#endif // SERVER_H
