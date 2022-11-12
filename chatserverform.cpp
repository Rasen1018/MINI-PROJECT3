#include "chatserverform.h"
#include "ui_chatserverform.h"
#include "chatclientform.h"
#include "logthread.h"

#include <QtNetwork>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QDateTime>
#include <QMenu>
#include <QProgressDialog>
#include <QHashIterator>
#define BLOCK_SIZE 1024

ChatServerForm::ChatServerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatServerForm)
{
    ui->setupUi(this);
    setGeometry(60, 400, 450, 600);
    chatServer = new QTcpServer(this);      // 채팅 서버 생성

/************************채팅서버 연결을 위한 lambda식***************************/
    connect(chatServer, &QTcpServer::newConnection, this, [=](){     // clientConnect
        QTcpSocket *clientConnection = chatServer->nextPendingConnection( );
        connect(clientConnection, SIGNAL(disconnected()), this, SLOT(removeClient()));
        connect(clientConnection, SIGNAL(readyRead()), SLOT(receiveData()));
        ui->welcome->setText(tr("new connection is established..."));
    });
    if (!chatServer->listen(QHostAddress::Any, 2000)) {
        QMessageBox::critical(this, tr("Echo Server"),
                              tr("Unable to start the server: %1.").arg(chatServer->errorString()));
        close();
        return;
    }
/*******************************************************************************/
    fileServer = new QTcpServer(this);      // 파일 서버 생성
    connect(fileServer, SIGNAL(newConnection()), SLOT(acceptConnection()));     // 파일 서버 접속
    if (!fileServer->listen(QHostAddress::Any, 4000)) {                         // 서버에 연결이 안됐을 때
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(fileServer->errorString( )));
        close( );
        return;
    }

    ui->welcome->
            setText(tr("The server is running on port %1.").arg(chatServer->serverPort()));
    //setWindowTitle(tr("Echo Server"));

    menu = new QMenu;
    menu->addAction(ui->actionKickOut);
    ui->connectListWidget->setContextMenuPolicy(Qt::CustomContextMenu);     // 고객 채팅방 강퇴 액션 추가

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);        // 리셋되면 자동 닫힘
    progressDialog->reset();

    logThread = new LogThread(this);    // log 저장하는 스레드 생성
    logThread->start();

/**************************채팅 로그 저장 lambda식*******************************/
    connect(ui->savePushButton, &QPushButton::clicked, logThread, [=](){
        logThread->saveData();
        QMessageBox::information(this, tr("Log Information"),
                              tr("Save Complete"));
    });
/*******************************************************************************/
    qDebug() << tr("The server is running on port %1.").arg(chatServer->serverPort( ));
}

ChatServerForm::~ChatServerForm()
{
    delete ui;
    logThread->terminate();
    chatServer->close();
    fileServer->close();
}

void ChatServerForm::receiveData()      // chat_status에 따른 데이터 송수신 함수
{
    // sender()와 사용자 정의 슬롯의 연결을 원활히 하기 위해서
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));
    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);

    Chat_Status type;       // 채팅의 목적
    char data[1010];        // 전송되는 메시지/데이터
    memset(data, 0, 1010);  // 배열을 0으로 초기화

    // 고객 채팅 프로그램에서 데이터 수신을 위해 바이너리 구조의 데이터 스트림 생성
    QDataStream in(&bytearray, QIODevice::ReadOnly);
    // 현재 연결된 디바이스에서 제일 처음 파일을 읽도록 오프셋 설정
    in.device()->seek(0);
    // 스트림에서 저장한 chat protocol을 가져옴
    in >> type;
    // 데이터 읽기
    in.readRawData(data, 1010);

    QString ip = clientConnection->peerAddress().toString();    // 연결된 고객 ip
    quint16 port = clientConnection->peerPort();                // 연결된 고객 port
    QString id = QString::fromStdString(data);                  // chat_status에 따라 고객 id, 고객 채팅 저장

    switch(type) {
    case Chat_Login:    // 고객이 로그인 했을 때
        // 고객 리스트에서 로그인한 ID를 확인해서 고객 정보를 저장
        foreach(auto item, ui->allClientListWidget->findItems(id, Qt::MatchContains)) {
            QString cmp=item->text();
            item->setIcon(QIcon(":/images/connected.png"));     // 연결 아이콘으로 변경
            if(!(ui->connectListWidget->itemWidget(item))) {    // 접속 중인 고객 리스트에 없을 경우
                clientList.append(clientConnection);            // 고객 소켓 리스트에 추가
                clientSocketHash[id] = clientConnection;        // ID가 Key이고 TCP 소켓이 value인 QHash
                clientIdHash[port] = cmp;                       // Port번호가 Key이고 이름과 ID를 value로 하는 QHash
                QListWidgetItem* connectItem = new QListWidgetItem;
                connectItem->setIcon(QIcon(":/images/connected.png"));
                connectItem->setText(clientIdHash[port]);
                connectItem->setTextAlignment(Qt::AlignCenter);
                ui->connectListWidget->addItem(connectItem);    // 접속 중인 고객 리스트에 추가
            }
        }
        break;
    case Chat_In:    // 고객이 채팅에 입장했을 때
        // 고객 ID를 검색해서
        foreach(auto item, ui->allClientListWidget->findItems(id, Qt::MatchContains)) {
            item->setIcon(QIcon(":/images/chat2.png"));     // 채팅 아이콘으로 변경
            }

        foreach(auto item, ui->connectListWidget->findItems(id, Qt::MatchContains)) {
            QStringList tmp=item->text().split(")");
            if(item->text().right(1) !="t") {
                item->setIcon(QIcon(":/images/chat2.png"));     // 채팅 아이콘으로 변경
                item->setText(tmp.at(0)+")" + "    On Chat");   // 채팅중임을 표시
            }
        }
        break;
    case Chat_Talk:     /*고객이 채팅할 때*/ {
        foreach(QTcpSocket *sock, clientList) {
            if(clientIdHash.contains(sock->peerPort()) && sock != clientConnection) {
                QByteArray sendArray;
                sendArray.clear();
                QDataStream out(&sendArray, QIODevice::WriteOnly);
                out << Chat_Talk;
                // 고객 채팅 창에 고객이름, id, 채팅 내용 전달
                sendArray.append("<font color=lightsteelblue>");
                sendArray.append(clientIdHash[port].toStdString().data());
                sendArray.append("</font> : ");
                sendArray.append(id.toStdString().data());
                sock->write(sendArray);
            }
        }
        // 채팅할 때마다 log 추가
        QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
        log->setText(0, ip.split(":").at(3));
        log->setText(1, QDateTime::currentDateTime().toString());
        log->setText(2, clientIdHash[port].left(4));
        log->setText(3, clientIdHash[port].mid(5,3));
        log->setText(4, QString(data));
        log->setToolTip(4, QString(data));

        ui->logTreeWidget->addTopLevelItem(log);

        logThread->appendData(log);     // log 저장을 위해 list에 treewidgetitem 추가

    }
        break;
    case Chat_Out:   // 채팅방에서 퇴장할 때
        foreach(auto item, ui->allClientListWidget->findItems(id, Qt::MatchContains)) {
            item->setIcon(QIcon(":/images/connected.png"));     // 연결 아이콘으로 변경
            }
        foreach(auto item, ui->connectListWidget->findItems(id, Qt::MatchContains)) {
            QStringList tmp=item->text().split(")");
            if(item->text().right(1) =="t") {
                item->setIcon(QIcon(":/images/connected.png"));     // 연결 아이콘으로 변경
                item->setText(tmp.at(0)+")"+"    On Standby");      // 대기 중임을 표시
            }
        }
        break;
    case Chat_LogOut:   /*고객이 로그아웃할 때*/ {
        foreach(auto item, ui->allClientListWidget->findItems(id, Qt::MatchContains)) {
            item->setIcon(QIcon(":/images/disconnect.png"));      // 비접속 아이콘으로 변경
            }

        foreach(auto item, ui->connectListWidget->findItems(id, Qt::MatchContains)) {
                clientList.removeOne(clientConnection);        // 고객 소켓 리스트에서 소켓 삭제
                clientSocketHash.remove(id);                   // ID가 Key이고 TCP 소켓이 value인 QHash
        }
    }
        break;
    }
}

// 채팅 서버가 생성될 때 고객 리스트를 부르는 함수
void ChatServerForm::openWidget()
{
    emit callClientForm();
}

// 모든 고객 리스트를 가지고 오는 슬롯
void ChatServerForm::getAllClient(QStringList list) {

    QListWidgetItem* item = new QListWidgetItem;
    QString cmp = list[0] + "("+ list[1] + ")";     // '고객이름(아이디)' 형태로 리스트 item 설정
    if((ui->allClientListWidget->findItems(cmp, Qt::MatchFixedString)).count()==0)
    {
        item->setIcon(QIcon(":/images/disconnect.png"));
        item->setText(list[0] + "("+ list[1] + ")");
        item->setTextAlignment(Qt::AlignCenter);
        ui->allClientListWidget->addItem(item);
    }
}

void ChatServerForm::widgetUpdate() {   // 고객 정보가 추가, 변경, 삭제될 때마다 위젯 업데이트해주는 슬롯
    ui->allClientListWidget->clear();
}

void ChatServerForm::removeClient()     // 채팅 프로그램과 연결이 끊어지면(무조건, 어떤상황이든지)
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));
    clientList.removeOne(clientConnection);
    clientConnection->deleteLater();
    // 고객 지울때 connect list에서 삭제하기, chat out 상태일때는 connect list에서 icon만 변경
    QString name = clientIdHash[clientConnection->peerPort()];
    foreach(auto item, ui->connectListWidget->findItems(name, Qt::MatchContains)) {
        QStringList tmp=item->text().split(")");        // '고객이름(아이디)' 형태로 파싱
        if(tmp.at(0)+")" == name) {
            int row = ui->connectListWidget->row(item);
            ui->connectListWidget->takeItem(row);
        }
    }
}

void ChatServerForm::on_actionKickOut_triggered()       // 고객을 강퇴할 때 슬롯(고객이 채팅방을 나갈때와 같이 기능을 구현)
{
    if(ui->connectListWidget->currentItem() != nullptr) {
        QString id = ui->allClientListWidget->currentItem()->text().left(4);
        QTcpSocket* sock = clientSocketHash[id];

        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << Chat_KickOut;
        out.writeRawData("", 1010);

        sock->write(sendArray);
        QListWidgetItem* connectItem = ui->allClientListWidget->currentItem();
        connectItem->setIcon(QIcon(":/images/connected.png"));

        Q_FOREACH(auto item, ui->connectListWidget->findItems(id, Qt::MatchContains))
        {
            QStringList tmp=item->text().split(")");        // '고객이름(아이디)' 형태로 파싱
            item->setIcon(QIcon(":/images/connected.png"));
            item->setText(tmp.at(0)+")"+"    On Standby");
        }
    }
}

void ChatServerForm::on_createPushButton_clicked()      // 고객을 초대할 때 슬롯(고객이 채팅에 입장했을 때와 같이 기능을 구현)
{
    if(ui->allClientListWidget->count()) {
        QString id = ui->allClientListWidget->currentItem()->text().left(4);

        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << Chat_Invite;
        out.writeRawData("", 1010);
        QTcpSocket* sock = clientSocketHash[id];
        if(sock == nullptr) return;

        sock->write(sendArray);
        QListWidgetItem* connectItem = ui->allClientListWidget->currentItem();
        connectItem->setIcon(QIcon(":/images/chat2.png"));

        foreach(auto item, ui->connectListWidget->findItems(id, Qt::MatchContains)) {
            QStringList tmp=item->text().split(")");        // '고객이름(아이디)' 형태로 파싱
            item->setIcon(QIcon(":/images/chat2.png"));
            item->setText(tmp.at(0)+")" + "    On Chat");
        }
    }
}

void ChatServerForm::on_connectListWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->connectListWidget->currentItem() != nullptr) {
        QStringList tmp = ui->connectListWidget->currentItem()->text().split(")");  // '고객이름(아이디)' 형태로 파싱
        QString text = tmp.at(0)+")";

        // 접속 중인 고객에서 고객 리스트를 우클릭하면 전체 고객 리스트의 고객 리스트도 선택됨
        foreach(auto i, ui->allClientListWidget->findItems(text, Qt::MatchFixedString)) {
            int row = ui->allClientListWidget->row(i);
            ui->allClientListWidget->setCurrentRow(row);
        }
    }
    QPoint globalPos = ui->connectListWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void ChatServerForm::acceptConnection()     // 파일 서버 접속 슬롯
{
    qDebug("Connected, preparing to receive files!");

    QTcpSocket* receivedSocket = fileServer->nextPendingConnection();
    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

void ChatServerForm::readClient()       // 파일 전송 슬롯
{
    qDebug("Receiving file ...");
    QTcpSocket* receivedSocket = dynamic_cast<QTcpSocket *>(sender( ));
    QString filename;
    QString id;

    if (byteReceived == 0) { // just started to receive data, this data is file information
        progressDialog->reset();
        progressDialog->show();

        QString ip = receivedSocket->peerAddress().toString();
        quint16 fileport = receivedSocket->peerPort();

        QDataStream in(receivedSocket);
        in >> totalSize >> byteReceived >> filename >> id;
        progressDialog->setMaximum(totalSize);

        QString cmp;
        foreach(auto item, ui->allClientListWidget->findItems(id, Qt::MatchContains)) {
            cmp=item->text();
        }

        // 파일 저장할 때 파일 명을 포함해서 로그에 추가
        QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
        log->setText(0, ip.split(":").at(3));
        log->setText(1, QDateTime::currentDateTime().toString());
        log->setText(2, id);
        log->setText(3, cmp.mid(5,3));
        log->setText(4, filename.split("/").last());
        log->setToolTip(4, filename);

        for(int i = 0; i < ui->logTreeWidget->columnCount(); i++)
            ui->logTreeWidget->resizeColumnToContents(i);

        ui->logTreeWidget->addTopLevelItem(log);

        logThread->appendData(log);

        QFileInfo info(filename);
        QString currentFileName = info.fileName();
        file = new QFile(currentFileName);
        file->open(QFile::WriteOnly);
    }
    else { // Officially read the file content
        inBlock = receivedSocket->readAll();

        byteReceived += inBlock.size();
        file->write(inBlock);
        file->flush();
    }

    progressDialog->setValue(byteReceived);

    if (byteReceived == totalSize) {
        qDebug() << QString("%1 receive completed").arg(filename);

        inBlock.clear();
        byteReceived = 0;
        totalSize = 0;
        progressDialog->reset();
        progressDialog->hide();
        file->close();
        delete file;
    }
}


void ChatServerForm::on_connectListWidget_itemClicked(QListWidgetItem *item)
{
    QStringList tmp=item->text().split(")");
    QString text = tmp.at(0)+")";

    // 전체 고객 리스트도 선택
    foreach(auto i, ui->allClientListWidget->findItems(text, Qt::MatchFixedString)) {
        int row = ui->allClientListWidget->row(i);
        ui->allClientListWidget->setCurrentRow(row);
    }
}


void ChatServerForm::on_newPushButton_clicked()     // 관리자 채팅창 열기
{
    ChatClientForm *chatClient = new ChatClientForm(0);
    chatClient->show();
    connect(chatClient, SIGNAL(destroyed()),
            chatClient, SLOT(deleteLater()));
}

