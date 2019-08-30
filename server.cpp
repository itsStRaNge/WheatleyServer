#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>

#define RED_PIN 4//24
#define BLUE_PIN 1//23
#define GREEN_PIN 5//18

#define ON_RASPBERRY 1
#if (ON_RASPBERRY == 1)
    #include <wiringPi.h>
    #include <softPwm.h>
#endif


server::server(QObject *parent) : QTcpServer(parent)
{

    this->port = 5005;
    if(!this->listen(QHostAddress::AnyIPv4, this->port)){
        qCritical() << "\"SERVER\" Could not start";
    }else{
        qDebug()<< "\"SERVER\" Listening to port" << this->serverPort();
        qDebug()<< "Listening to IP:"<<this->serverAddress();
    }
    //setup pins
    if(wiringPiSetup() == -1){
        qDebug()<<"ERROR cant setup wiringPi";
    }

    //Socket setups
    SOCKET_PIN = 0;//17 , 11
    piHiPri(20);
    this->mySwitch = RCSwitch();
    mySwitch.setPulseLength(300);
    mySwitch.enableTransmit(0);
    mySwitch.setRepeatTransmit(20);
}

void server::incomingConnection(qintptr socketDescriptor)
{
    current_client = new QTcpSocket;
    current_client->setSocketDescriptor(socketDescriptor);
    connect( current_client, SIGNAL( readyRead() ) , this, SLOT( readyRead() ), Qt::QueuedConnection );
    connect( current_client, SIGNAL( disconnected() ), this, SLOT( disconnected() ) );
}

void server::readyRead(){
    char inputBuffer[1000];
    QJsonObject answer;
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    qint64 bytesRead = socket->read(inputBuffer, 1000);
    QString jsonString(QByteArray(inputBuffer, bytesRead));
    qDebug()<<"string: "<<jsonString;
    if(jsonString.at(0) != '{'){
        jsonString.prepend('{');
    }
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    qDebug()<<"received: "<<doc;

    if(!doc.isEmpty() && !doc.isNull()){
        QJsonObject dataObject = doc.object();
        int socket = dataObject.take("Socket").toInt();
        int state = dataObject.take("State").toInt();
        setSocket(socket, state);
    }else{
        qWarning()<<"Error in reading Socket";
    }
}

void server::disconnected(){
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    socket->deleteLater();
}

void server::setSocket(int socket, int state){
    char nGroupNumber[] = "11100";

    if(state){
        //turn socket on
        mySwitch.switchOn(nGroupNumber,socket);
    }else{
        //turn socket off
        mySwitch.switchOff(nGroupNumber,socket);
    }
    qDebug()<<"Update Socket "<<socket<< " with state "<<state;
}

