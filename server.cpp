#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>

#define RED_PIN 24//24
#define BLUE_PIN 23//23
#define GREEN_PIN 18//18

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
    //set valid commands
    commands<<"red"<<"green"<<"blue"<<"lightsOff"<<"fade"<<"socket";
    //setup pins
   wiringPiSetupGpio();
 /*   if(wiringPiSetup() == -1){
	qDebug()<<"ERROR cant setup wiringPi";
    }    */
    list[0].bright=0;
    list[0].pin = RED_PIN;
    softPwmCreate(list[0].pin,0,100);
    list[1].bright=0;
    list[1].pin = GREEN_PIN;
    softPwmCreate(list[1].pin,0,100);
    list[2].bright=0;
    list[2].pin = BLUE_PIN;
    softPwmCreate(list[2].pin,0,100);

    //Socket setups
    SOCKET_PIN = 17;//17 , 11
    piHiPri(20);
    this->mySwitch = RCSwitch();
    mySwitch.setPulseLength(300);
    mySwitch.enableTransmit(SOCKET_PIN);

    //turn all sockets on
    for(int i = 0;i<NUM_OF_SOCKETS;i++){
        socketState[i]=true;
//        mySwitch.switchOn(nGroupNumber,i+1);
    }
}

void server::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *clientConnection = new QTcpSocket;
    clientConnection->setSocketDescriptor(socketDescriptor);
    connect( clientConnection, SIGNAL( readyRead() ) , this, SLOT( readyRead() ), Qt::QueuedConnection );
    connect( clientConnection, SIGNAL( disconnected() ), this, SLOT( disconnected() ) );
}

void server::readyRead(){
    char inputBuffer[1000];
    qint64 bytesRead = this->socket.read(inputBuffer, 1000);
    QString jsonString(QByteArray(inputBuffer, bytesRead));
    if(jsonString.at(0) != '{'){
        jsonString.prepend('{');
    }
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    qDebug()<<"received: "<<doc;

    if(!doc.isEmpty() && !doc.isNull()){
        QJsonObject dataObject = doc.object();
        QString cmd = dataObject.take("command").toString();

        switch(commands.indexOf(cmd)){
            case 0:
                //set red
                bright = makeNumberValid(dataObject.take("value").toInt());
                this->setColor(0,bright);
                break;
            case 1:
                //set green
                bright = makeNumberValid(dataObject.take("value").toInt());
                this->setColor(1,bright);
                break;
            case 2:
                //set blue
                bright = makeNumberValid(dataObject.take("value").toInt());
                this->setColor(2,bright);
                break;
            case 3:
                //lightsOff
                setColor(0,0);
                setColor(1,0);
                setColor(2,0);
                break;
            case 4:
                //fade colors
                qDebug()<<"fade is not implemented yet";
                break;
            case 5:
                //sockets
                QString socketCmd = dataObject.take("id").toString();
                if(socketCmd == "off"){
                    socketsOff();
                }else{
                    toggleSocket(socketCmd.toInt());
                }
                break;
            default:
                qWarning()<<"Unknown Color";
        }
    }else{
        qWarning()<<"Error in reading Socket";
    }
}

void server::disconnected(){
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    socket->deleteLater();
}

int server::makeNumberValid(int bright){
    if(bright<0){
        return 0;
    }
    if(bright>100){
        return 100;
    }
    return bright;
}

void server::toggleSocket(int socketNr){
   char nGroupNumber[] = "11100";

    if(socketState[socketNr-1]){
        //turn socket off
        mySwitch.switchOffBinary(nGroupNumber,socketNr);
        socketState[socketNr-1]=false;
    }else{
        //turn socket on
        mySwitch.switchOnBinary(nGroupNumber,socketNr);
        socketState[socketNr-1]=true;
    }
    qDebug()<<nGroupNumber<<socketNr<<socketState[socketNr-1];
}

void server::socketsOff(){
    for(int i = 0;i<NUM_OF_SOCKETS;i++){
        char nGroupNumber[] = "11100";
        mySwitch.switchOffBinary(nGroupNumber,socketNr+1);
        socketState[socketNr]=false;
    }
    qDebug()<<"Turn off all Sockets";
}

void server::setColor(int pos,int newBright){
    qDebug()<<commands.at(0)<<"to"<<newBright;
    list[pos].bright = newBright;
#if (ON_RASPBERRY == 1)
    softPwmWrite(list[pos].pin,list[pos].bright);
#endif
}
