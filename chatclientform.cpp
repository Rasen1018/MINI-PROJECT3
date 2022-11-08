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
    ui->ipLineEdit->setText("127.0.0.1");    //ip 고정
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

    clientSocket = new QTcpSocket(this);        // 클라이언트 소켓 생성

    connect(clientSocket, &QAbstractSocket::errorOccurred, this, [=](){
        qDebug() << clientSocket->errorString();
    }); // 에러 발생하면 qdebug로 찍어줌

    /*-------- 서버와 연결하는 부분 필요, sendprotocol(login) 필요, data 보내는 부분 필요 --------*/
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

        if(type == Chat_Talk) {
            ui->chatTextEdit->append(QString(data));
            ui->inputLineEdit->setEnabled(true);
            ui->enterPushButton->setEnabled(true);
        }
    });

    connect(clientSocket, &QTcpSocket::disconnected, this, [=](){
        // 소켓의 연결이 끊겼을 때 메세지 박스 띄워주고 위젯 설정
        QMessageBox::critical(this, tr("Chatting Client"),
                              tr("Disconnect from Server"));
        ui->inputLineEdit->setEnabled(false);
        ui->idLineEdit->setReadOnly(false);
        ui->enterPushButton->setEnabled(false);
    });

    fileClient = new QTcpSocket(this);      // 파일 전송을 위한 소켓 생성
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

void ChatClientForm::on_quitPushButton_clicked()
{
    this->close();
}
