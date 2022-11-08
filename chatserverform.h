#ifndef CHATSERVERFORM_H
#define CHATSERVERFORM_H

#include <QWidget>
#include <QList>

class QTreeWidgetItem;
class QTcpServer;
class QTcpSocket;
class QLabel;
class QFile;
class QProgressDialog;
class LogThread;
class QListWidgetItem;
class ChatClientForm;

//typedef enum {
//    Chat_Login,           // 로그인(서버 접속)
//    Chat_In,              // 채팅방 입장
//    Chat_Talk,            // 채팅
//    Chat_Out,             // 채팅방 퇴장 --> 초대 가능
//    Chat_LogOut,          // 로그 아웃(서버 끊김) --> 초대 불가능
//    Chat_Invite,          // 초대
//    Chat_KickOut          // 강퇴
//} Chat_Status;

namespace Ui {
class ChatServerForm;
}

class ChatServerForm : public QWidget
{
    Q_OBJECT

private:
    ChatServerForm(QWidget *parent = nullptr);
    ChatServerForm(const ChatServerForm& ref);
    ChatServerForm& operator=(const ChatServerForm& ref);
    ~ChatServerForm();

public:
    static ChatServerForm& getIncetance() {
        static ChatServerForm ch;
        return ch;
    }
    void openWidget();

signals:
    void callClientForm();

private slots:
    /* 채팅 서버 */
    void receiveData();
    void removeClient();

    /* 파일서버 */
    void readClient();
    void acceptConnection();

    /* ui 슬롯 */
    void on_actionKickOut_triggered();
    void on_createPushButton_clicked();
    void on_connectListWidget_customContextMenuRequested(const QPoint &pos);

    void on_connectListWidget_itemClicked(QListWidgetItem *item);

private:
    const int BLOCK_SIZE = 1024;
    QLabel *infoLabel;
    QTcpServer *chatServer;
    QTcpServer *fileServer;
    QList<QTcpSocket*> clientList;
    QHash<quint16, QString> clientIdHash;
    QHash<QString, QTcpSocket*> clientSocketHash;
    Ui::ChatServerForm *ui;
    QMenu* menu;
    QFile* file;
    QProgressDialog* progressDialog;
    qint64 totalSize;
    qint64 byteReceived;
    QByteArray inBlock;
    LogThread* logThread;
};

#endif // CHATSERVERFORM_H
