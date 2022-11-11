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
    chatServer = new QTcpServer(this);

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

    fileServer = new QTcpServer(this);
    connect(fileServer, SIGNAL(newConnection()), SLOT(acceptConnection()));
    if (!fileServer->listen(QHostAddress::Any, 4000)) {
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
    ui->connectListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);        // 리셋되면 자동 닫힘
    progressDialog->reset();

    logThread = new LogThread(this);
    logThread->start();

    connect(ui->savePushButton, &QPushButton::clicked, logThread, [=](){
        logThread->saveData();
        QMessageBox::information(this, tr("Log Information"),
                              tr("Save Complete"));
    });
    qDebug() << tr("The server is running on port %1.").arg(chatServer->serverPort( ));
}

ChatServerForm::~ChatServerForm()
{
    delete ui;
    logThread->terminate();
    chatServer->close();
    fileServer->close();
}

void ChatServerForm::receiveData()
{
    // sender()와 사용자 정의 슬롯의 연결을 원활히 하기 위해서
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));
    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);

    Chat_Status type;       // 채팅의 목적
    char data[1010];        // 전송되는 메시지/데이터
    memset(data, 0, 1010);

    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in.device()->seek(0);
    in >> type;
    in.readRawData(data, 1010);

    QString ip = clientConnection->peerAddress().toString();
    quint16 port = clientConnection->peerPort();
    QString id = QString::fromStdString(data);

    switch(type) {
    case Chat_Login:
        foreach(auto item, ui->allClientListWidget->findItems(id, Qt::MatchContains)) {
            QString cmp=item->text();
            item->setIcon(QIcon(":/images/connected.png"));
            if(!(ui->connectListWidget->itemWidget(item))) {
                clientList.append(clientConnection);        // QList<QTcpSocket*> clientList;
                clientSocketHash[id] = clientConnection;
                clientIdHash[port] = cmp;
                QListWidgetItem* connectItem = new QListWidgetItem;
                connectItem->setIcon(QIcon(":/images/connected.png"));
                connectItem->setText(clientIdHash[port]);
                connectItem->setTextAlignment(Qt::AlignCenter);
                ui->connectListWidget->addItem(connectItem);
                qDebug() << port << clientIdHash[port].left(4) << clientIdHash[port].mid(5,3);
            }
        }
        break;
    case Chat_In:
        foreach(auto item, ui->allClientListWidget->findItems(id, Qt::MatchContains)) {
            item->setIcon(QIcon(":/images/chat2.png"));
            }

        foreach(auto item, ui->connectListWidget->findItems(id, Qt::MatchContains)) {
            QStringList tmp=item->text().split(")");
            if(item->text().right(1) !="t") {
                item->setIcon(QIcon(":/images/chat2.png"));
                item->setText(tmp.at(0)+")" + "    On Chat");
            }
        }
        break;
    case Chat_Talk: {
        foreach(QTcpSocket *sock, clientList) {
            if(clientIdHash.contains(sock->peerPort()) && sock != clientConnection) {
                QByteArray sendArray;
                sendArray.clear();
                QDataStream out(&sendArray, QIODevice::WriteOnly);
                out << Chat_Talk;
                sendArray.append("<font color=lightsteelblue>");
                sendArray.append(clientIdHash[port].toStdString().data());
                sendArray.append("</font> : ");
                sendArray.append(id.toStdString().data());
                sock->write(sendArray);
                //qDebug() << sock->peerPort();
            }
        }

        QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
        log->setText(0, ip.split(":").at(3));
        log->setText(1, QDateTime::currentDateTime().toString());
        log->setText(2, clientIdHash[port].left(4));
        log->setText(3, clientIdHash[port].mid(5,3));
        log->setText(4, QString(data));
        log->setToolTip(4, QString(data));

        for(int i = 0; i < ui->logTreeWidget->columnCount(); i++)       // 현재 column 사이즈 설정
            ui->logTreeWidget->resizeColumnToContents(i);

        ui->logTreeWidget->addTopLevelItem(log);

        logThread->appendData(log);

    }
        break;
    case Chat_Out:
        foreach(auto item, ui->allClientListWidget->findItems(id, Qt::MatchContains)) {
            item->setIcon(QIcon(":/images/connected.png"));
            }
        foreach(auto item, ui->connectListWidget->findItems(id, Qt::MatchContains)) {
            QStringList tmp=item->text().split(")");
            if(item->text().right(1) =="t") {
                item->setIcon(QIcon(":/images/connected.png"));
                item->setText(tmp.at(0)+")"+"    On Standby");
            }
        }
        break;
    case Chat_LogOut: {
        foreach(auto item, ui->allClientListWidget->findItems(id, Qt::MatchContains)) {
            item->setIcon(QIcon(":/images/disconnect.png"));
            }

        foreach(auto item, ui->connectListWidget->findItems(id, Qt::MatchContains)) {
                clientList.removeOne(clientConnection);        // QList<QTcpSocket*> clientList;
                clientSocketHash.remove(id);
        }
    }
        break;
    }
}

void ChatServerForm::openWidget()
{
    emit callClientForm();
}

void ChatServerForm::getAllClient(QStringList list) {

    QListWidgetItem* item = new QListWidgetItem;
    QString cmp = list[0] + "("+ list[1] + ")";
    if((ui->allClientListWidget->findItems(cmp, Qt::MatchFixedString)).count()==0)
    {
        item->setIcon(QIcon(":/images/disconnect.png"));
        item->setText(list[0] + "("+ list[1] + ")");
        item->setTextAlignment(Qt::AlignCenter);
        ui->allClientListWidget->addItem(item);
    }
}

void ChatServerForm::widgetUpdate() {
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
        QStringList tmp=item->text().split(")");
        if(tmp.at(0)+")" == name) {
            int row = ui->connectListWidget->row(item);
            ui->connectListWidget->takeItem(row);
        }
    }
}

void ChatServerForm::on_actionKickOut_triggered()
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
            QStringList tmp=item->text().split(")");
            item->setIcon(QIcon(":/images/connected.png"));
            item->setText(tmp.at(0)+")"+"    On Standby");
        }
    }
}

void ChatServerForm::on_createPushButton_clicked()
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
            QStringList tmp=item->text().split(")");
            item->setIcon(QIcon(":/images/chat2.png"));
            item->setText(tmp.at(0)+")" + "    On Chat");
        }
    }
}

void ChatServerForm::on_connectListWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->connectListWidget->currentItem() != nullptr) {
        QStringList tmp = ui->connectListWidget->currentItem()->text().split(")");
        QString text = tmp.at(0)+")";

        foreach(auto i, ui->allClientListWidget->findItems(text, Qt::MatchFixedString)) {
            int row = ui->allClientListWidget->row(i);
            ui->allClientListWidget->setCurrentRow(row);
        }
    }
    QPoint globalPos = ui->connectListWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void ChatServerForm::acceptConnection()
{
    qDebug("Connected, preparing to receive files!");

    QTcpSocket* receivedSocket = fileServer->nextPendingConnection();
    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

void ChatServerForm::readClient()
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

    foreach(auto i, ui->allClientListWidget->findItems(text, Qt::MatchFixedString)) {
        int row = ui->allClientListWidget->row(i);
        ui->allClientListWidget->setCurrentRow(row);
    }
}


void ChatServerForm::on_newPushButton_clicked()
{
    ChatClientForm *chatClient = new ChatClientForm(0);
    chatClient->show();
    connect(chatClient, SIGNAL(destroyed()),
            chatClient, SLOT(deleteLater()));
}

