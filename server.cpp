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
    //set valid commands
    commands<<"red"<<"green"<<"blue"<<"lightsOff"<<"fade"<<"socket"<<"socketStates";
    //setup pins
    if(wiringPiSetup() == -1){
	qDebug()<<"ERROR cant setup wiringPi";
    }
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
    SOCKET_PIN = 0;//17 , 11
    piHiPri(20);
    this->mySwitch = RCSwitch();
    mySwitch.setPulseLength(300);
    mySwitch.enableTransmit(0);

    //turn all sockets on
    for(int i = 0;i<NUM_OF_SOCKETS;i++){
        socketState[i]=true;
//        mySwitch.switchOn(nGroupNumber,i+1);
    }
}

void server::incomingConnection(qintptr socketDescriptor)
{
    current_client = new QTcpSocket;
    current_client->setSocketDescriptor(socketDescriptor);
    connect( current_client, SIGNAL( readyRead() ) , this, SLOT( readyRead() ), Qt::QueuedConnection );
    connect( current_client, SIGNAL( disconnected() ), this, SLOT( disconnected() ) );
}

void server::send_answer(QJsonObject packet){
    QJsonDocument doc(packet);

    QByteArray bytes  = doc.toJson(QJsonDocument::Compact);
    qDebug()<<"sending: "<<QString(bytes);
    if(current_client->write(bytes) == -1){
        qWarning()<<"cannot send answer to client";
    }
    if(!current_client->waitForBytesWritten(3000))
        qWarning()<<"sending answer failed";
    current_client->disconnectFromHost();

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
        QString cmd = dataObject.take("command").toString();
	int bright;

        switch(commands.indexOf(cmd)){
            case 0:{
                //set red
                bright = makeNumberValid(dataObject.take("value").toInt());
                this->setColor(0,bright);
                }
		break;
            case 1:{
                //set green
                bright = makeNumberValid(dataObject.take("value").toInt());
                this->setColor(1,bright);
                }
		break;
            case 2:{
                //set blue
                bright = makeNumberValid(dataObject.take("value").toInt());
                this->setColor(2,bright);
                }
		break;
            case 3:{
                //lightsOff
                setColor(0,0);
                setColor(1,0);
                setColor(2,0);
                }
		break;
            case 4:{
                //fade colors
                qDebug()<<"fade is not implemented yet";
                }
		break;
            case 5:{
                //sockets
                QString soCmd = dataObject.take("id").toString();
                if(soCmd == "off"){
                    socketsOff();
                }else if(soCmd == "on"){
                    socketsOn();
                }else{
                    setSocket(soCmd.toInt(), dataObject.take("state").toInt());
                }

                answer["command"]="success";
                send_answer(answer);
		}
                break;
        case 6:{
            //get socket states
            answer["command"] = "socketStates";

            for(int i=0;i<NUM_OF_SOCKETS;i++){
                answer[QString("%1").arg(i)] = socketState[i];
            }
            send_answer(answer);
            break;
        }
            default:{
                qWarning()<<"Unknown Color";
		}
		break;
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

void server::setSocket(int socket, int state){
    char nGroupNumber[] = "11100";

    for(int i=0;i<2;i++){
        if(state){
            //turn socket on
            mySwitch.switchOn(nGroupNumber,socket);
            socketState[socket-1]=true;
        }else{
            //turn socket off
            mySwitch.switchOff(nGroupNumber,socket);
            socketState[socket-1]=false;
        }
        QThread::msleep(20);
    }
    qDebug()<<nGroupNumber<<socket<<socketState[socket-1];
}

void server::socketsOff(){
    for(int i = 0;i<NUM_OF_SOCKETS;i++){
        setSocket(i, SOCKET_OFF);
    }
    qDebug()<<"Turn off all Sockets";
}

void server::socketsOn(){
    for(int i = 0;i<NUM_OF_SOCKETS;i++){
        setSocket(i, SOCKET_ON);
    }
    qDebug()<<"Turn on all Sockets";
}

void server::setColor(int pos,int newBright){
    qDebug()<<commands.at(0)<<"to"<<newBright;
    list[pos].bright = newBright;
#if (ON_RASPBERRY == 1)
    softPwmWrite(list[pos].pin,list[pos].bright);
#endif
}
