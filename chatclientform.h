#ifndef CHATCLIENTFORM_H
#define CHATCLIENTFORM_H

#include <QWidget>
#include <QDataStream>

class QTextEdit;
class QLineEdit;
class QTcpSocket;
class QFile;
class QProgressDialog;

/* 서버와 채팅 프로그램 간 데이터 송수신을 원활하게 하기 위해 프로토콜 타입 설정*/
typedef enum {
    Chat_Login,           // 로그인(서버 접속)
    Chat_In,              // 채팅방 입장
    Chat_Talk,            // 채팅
    Chat_Out,             // 채팅방 퇴장 --> 초대 가능
    Chat_LogOut,          // 로그 아웃(서버 끊김) --> 초대 불가능
    Chat_Invite,          // 초대
    Chat_KickOut          // 강퇴
} Chat_Status;

namespace Ui {
class ChatClientForm;
}

class ChatClientForm : public QWidget
{
    Q_OBJECT

public:
    explicit ChatClientForm(QWidget *parent = nullptr);
    ~ChatClientForm();

private slots:
    // 데이터 송수신을 위한 슬롯
    void sendProtocol(Chat_Status type, char* data, int size);
    // 버튼 눌렀을 때
    void on_logInPushButton_clicked();
    void on_enterPushButton_clicked();
    void on_pushButton_clicked();
    void on_sendPushButton_clicked();

    void on_quitPushButton_clicked();

private:
    Ui::ChatClientForm *ui;
    QTcpSocket *clientSocket;
    QFile* file;
    QTcpSocket *fileClient;
    QProgressDialog* progressDialog;    // 파일 진행 확인
    qint64 loadSize;                    // 파일을 쪼개서 전송하기 위한 사이즈 설정
    qint64 byteToWrite;                 // 남은 데이터 사이즈
    qint64 totalSize;                   // 파일 전체 사이즈
    QByteArray outBlock;                // 파일 전송을 위한 bytearray
    bool isSent;

};

#endif // CHATCLIENTFORM_H
