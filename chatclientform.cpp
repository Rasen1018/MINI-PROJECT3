#include "chatclientform.h"
#include "ui_chatclientform.h"

#include <QtGui>
#include <QtNetwork>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>

#define BLOCK_SIZE 1024  // 블록 사이즈 설정

ChatClientForm::ChatClientForm(QWidget *parent) :
    QWidget(parent), isSent(false),
    ui(new Ui::ChatClientForm)
{
    ui->setupUi(this);
    setGeometry(60, 60, 420, 320);              //위젯 크기 설정
    ChatClientForm::setWindowFlags(Qt::WindowTitleHint);
    ui->ipLineEdit->setText("192.168.0.33");    //ip 고정
    QRegularExpression re("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");   //ip 입력을 위한 정규표현식
    QRegularExpressionValidator validator(re);
    ui->ipLineEdit->setValidator(&validator);

    connect(ui->inputLineEdit, SIGNAL(returnPressed()), ui->enterPushButton, SLOT(click()));
    // enter 누르면 버튼 클릭
    ui->inputLineEdit->setEnabled(false);   //위젯 설정
    ui->enterPushButton->setEnabled(false);
    ui->sendPushButton->setDisabled(true);

    clientSocket = new QTcpSocket(this);        // 클라이언트 소켓 생성

    connect(clientSocket, &QAbstractSocket::errorOccurred, this, [=](){
        qDebug() << clientSocket->errorString();
    }); // 에러 발생하면 qdebug로 찍어줌
/*-----------------------------------------------------------------------------------------*/
    connect(clientSocket, &QTcpSocket::readyRead, this, [=](){
        // 클라이언트 소켓이 데이터 읽을 준비를 하면 데이터를 받아옴
        QTcpSocket *clientSocket = dynamic_cast<QTcpSocket *>(sender( ));
        // 받아올 데이터가 블록 사이즈보다 크면 슬롯 종료
        if (clientSocket->bytesAvailable( ) > BLOCK_SIZE) return;
        // 디바이스로부터 블록 사이즈만큼 데이터를 읽어서 bytearray 형태로 반환
        QByteArray bytearray = clientSocket->read(BLOCK_SIZE);

        Chat_Status type;       // 채팅 프로토콜 타입
        char data[1010];        // 전송되는 메시지/데이터
        memset(data, 0, 1010);  // 데이터 값을 0으로 초기화

        // 서버에 데이터 전송을 위해 바이너리 구조의 데이터 스트림 생성
        QDataStream in(&bytearray, QIODevice::ReadOnly);
        // 현재 연결된 디바이스에서 제일 처음 파일을 읽도록 오프셋 설정
        in.device()->seek(0);
        // 스트림에서 저장한 chat protocol을 가져옴
        in >> type;
        // 데이터 읽기
        in.readRawData(data, 1010);

        switch(type) {   // chat status에 따라 위젯 설정
        case Chat_Talk:  // 채팅 중일 때
            ui->chatTextEdit->append(QString(data));    // 메세지 창에 데이터(채팅 내용) 추가
            ui->inputLineEdit->setEnabled(true);
            ui->enterPushButton->setEnabled(true);
            ui->sendPushButton->setEnabled(true);
            break;
        case Chat_KickOut:  // 강퇴 당했을 때
            QMessageBox::critical(this, tr("Chatting Client"),  // 메세지박스 띄워줌
                                  tr("Kick out from Server"));
            ui->inputLineEdit->setDisabled(true);               // 위젯 사용 못하게 설정
            ui->enterPushButton->setDisabled(true);
            ui->sendPushButton->setDisabled(true);
            ui->idLineEdit->setReadOnly(false);
            ui->logInPushButton->setText(tr("Chat in"));
            break;
        case Chat_Invite:   // 채팅에 초대됐을 때
            QMessageBox::information(this, tr("Chatting Client"),
                                  tr("Invited from Server"));
            ui->inputLineEdit->setEnabled(true);                // 위젯 사용 가능하게 설정
            ui->enterPushButton->setEnabled(true);
            ui->sendPushButton->setEnabled(true);
            ui->idLineEdit->setReadOnly(true);
            ui->logInPushButton->setText(tr("Chat Out"));
            break;
        };
    });
/*-----------------------------------------------------------------------------------------*/
    connect(clientSocket, &QTcpSocket::disconnected, this, [=](){
        // 소켓의 연결이 끊겼을 때 메세지 박스 띄워주고 위젯 설정
        QMessageBox::critical(this, tr("Chatting Client"),
                              tr("Disconnect from Server"));
        ui->inputLineEdit->setEnabled(false);
        ui->idLineEdit->setReadOnly(false);
        ui->enterPushButton->setEnabled(false);
        ui->logInPushButton->setText(tr("Log In"));
    });

    fileClient = new QTcpSocket(this);      // 파일 전송을 위한 소켓 생성
/*-----------------------------------------------------------------------------------------*/
    connect(fileClient, &QTcpSocket::bytesWritten, this, [=](qint64 numBytes){
        // 파일 전송을 위해 데이터를 읽을 준비가 됐을 때
        // 데이터 보내면 남은 데이터 전송 공간 감소
        byteToWrite -= numBytes;

        outBlock = file->read(qMin(byteToWrite, numBytes));
        fileClient->write(outBlock);

        progressDialog->setMaximum(totalSize);            // 파일 전체 크기만큼 프로그레스 다이얼로그 설정
        progressDialog->setValue(totalSize-byteToWrite);

        if (byteToWrite == 0) {         // 전송 완료 시
            qDebug("File sending completed!");
            progressDialog->reset();
        }
    });
/*-----------------------------------------------------------------------------------------*/
    progressDialog = new QProgressDialog(0);    // 파일 전송 위젯 새로 띄우기
    progressDialog->setAutoClose(true);
    progressDialog->reset();

}

ChatClientForm::~ChatClientForm()
{
    delete ui;
    clientSocket->close();
}

void ChatClientForm::sendProtocol(Chat_Status type, char* data, int size)
{
    QByteArray dataArray;               // 데이터를 읽기 위한 bytearray 생성
    QDataStream out(&dataArray, QIODevice::WriteOnly);  // 서버로 보낼 데이터 스트림 생성
    out.device()->seek(0);              // 설정된 입출력 장치에서 첫번째 파일을 읽도록 오프셋 설정
    out << type;                        // chat status를 스트림에 저장
    out.writeRawData(data, size);       // chat status에 맞는 데이터 읽기
    clientSocket->write(dataArray);     // 서버로 전송
    clientSocket->flush();              // 버퍼에 있던 데이터 모두 송신
    while(clientSocket->waitForBytesWritten());     // 데이터 다 읽을 때까지 기다리기
}

void ChatClientForm::on_logInPushButton_clicked()       // 로그인 버튼 클릭했을 때
{
    if(ui->idLineEdit->text()=="") return;
    if(ui->logInPushButton->text() == tr("Log In")) {   // 로그인 버튼의 텍스트가 Log In 이라면
        clientSocket->connectToHost(ui->ipLineEdit->text( ),        // 서버 연결(ip, port 전송)
                                    ui->portLineEdit->text( ).toInt( ));
        clientSocket->waitForConnected();
        //chat status : chat login, 클라이언트 id 전송
        sendProtocol(Chat_Login, ui->idLineEdit->text().toStdString().data(), 1010);
        ui->logInPushButton->setText(tr("Chat in"));    // 로그인 버튼 Chat in 텍스트 변경
        ui->inputLineEdit->setDisabled(true);
        ui->idLineEdit->setReadOnly(true);
    }
    else if(ui->logInPushButton->text() == tr("Chat in"))  {    // 로그인 버튼의 텍스트가 Chat in 이라면
        //chat status : chat in, 클라이언트 id 전송
        sendProtocol(Chat_In, ui->idLineEdit->text().toStdString().data(), 1010);
        ui->logInPushButton->setText(tr("Chat Out"));   // 로그인 버튼 Chat Out 텍스트 변경
        ui->inputLineEdit->setEnabled(true);
        ui->enterPushButton->setEnabled(true);
        ui->sendPushButton->setEnabled(true);
    }
    else if(ui->logInPushButton->text() == tr("Chat Out"))  {   // 로그인 버튼의 텍스트가 Chat Out 이라면
        //chat status : chat out, 클라이언트 id 전송
        sendProtocol(Chat_Out, ui->idLineEdit->text().toStdString().data(), 1010);
        ui->logInPushButton->setText(tr("Chat in"));    // 로그인 버튼 Chat in 텍스트 변경
        ui->inputLineEdit->setDisabled(true);
        ui->enterPushButton->setDisabled(true);
        ui->sendPushButton->setDisabled(true);
    }
}

void ChatClientForm::on_enterPushButton_clicked()       // 메세지 전송 버튼 클릭
{
    QString str = ui->inputLineEdit->text();
    if(str.length()) {
        QByteArray bytearray = clientSocket->read(BLOCK_SIZE);
        bytearray = str.toUtf8();
        ui->chatTextEdit->append("<font color=red>나</font> : " + str);
        sendProtocol(Chat_Talk, bytearray.data(), 1010);
    }
    ui->inputLineEdit->clear();
}

void ChatClientForm::on_pushButton_clicked()            // 로그아웃 버튼 클릭
{
    sendProtocol(Chat_LogOut, ui->idLineEdit->text().toStdString().data(), 1010);
    clientSocket->close();
    ui->logInPushButton->setEnabled(true);
}

void ChatClientForm::on_sendPushButton_clicked()        // 파일 전송 버튼 클릭
{
    loadSize = 0;       // 한 주기마다 데이터를 보낼 수 있는 사이즈
    byteToWrite = 0;    // 데이터 전송까지 남은 사이즈
    totalSize = 0;      // 파일 크기
    outBlock.clear();   // bytearray 청소

    QString filename = QFileDialog::getOpenFileName(this);
    if(filename.length()) {          // 파일이 존재한다면
        file = new QFile(filename);
        file->open(QFile::ReadOnly); // 파일 오픈

        qDebug() << QString("file %1 is opened").arg(filename);
        progressDialog->setValue(0);

        if (!isSent) {  /* 파일을 처음 보내는 건지 확인(초기값 : false)
                           파일을 처음 보낸다면 ip, port 번호 전송 */
            fileClient->connectToHost(ui->ipLineEdit->text( ),
                                      ui->portLineEdit->text( ).toInt( ) + 2000);
            isSent = true;
        }

        byteToWrite = totalSize = file->size(); // 파일 크기만큼 사이즈 설정
        loadSize = 1024; // 한 주기당 1024만큼 데이터 전송

        //
        QDataStream out(&outBlock, QIODevice::WriteOnly);
        out << qint64(0) << qint64(0) << filename << ui->idLineEdit->text();

        totalSize += outBlock.size();   // 첫번째 qint64(0)에 덮어쓰기
        byteToWrite += outBlock.size(); // 두번째 qint64(0)에 덮어쓰기

        out.device()->seek(0);          // 파일 검색 커서 첫번째로 이동
        out << totalSize << qint64(outBlock.size());    // 파일 읽어오기

        fileClient->write(outBlock);    // 소켓으로 파일 보내기

        progressDialog->setMaximum(totalSize);          // 프로그레스바의 전체 크기를 파일 크기로 설정
        progressDialog->setValue(totalSize-byteToWrite);
        progressDialog->show();
    }

    qDebug() << QString("Sending file %1").arg(filename);
}

void ChatClientForm::on_quitPushButton_clicked()
{
    sendProtocol(Chat_LogOut, ui->idLineEdit->text().toStdString().data(), 1010);
    clientSocket->close();
    ui->logInPushButton->setEnabled(true);
}

