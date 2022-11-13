#ifndef CLIENTMANAGERFORM_H
#define CLIENTMANAGERFORM_H

#include <QWidget>
#include <QHash>
#include <QStandardItem>

class QMenu;
class QSqlQueryModel;

namespace Ui {
class ClientManagerForm;
}

class ClientManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ClientManagerForm(QWidget *parent = nullptr);
    ~ClientManagerForm();

signals:
    /* 다른 객체에 데이터 전달을 위한 시그널 */
    void updateList();
    void getAllClient(QStringList);
    void clientItemSent(QList<QStandardItem *>);

private slots:
    /* 다른 객체에 데이터 전달을 위한 슬롯 */
    void removeItem();
    void sendClientList();
    void receiveData(QString);
    void receiveData(int);
    /* QAction을 위한 슬롯 */
    void on_addPushButton_clicked();
    void on_modifyPushButton_clicked();
    void on_searchPushButton_clicked();
    void on_clearPushButton_clicked();
    /* QTableView를 위한 슬롯 */
    void setHeaderStyle();
    void on_tableView_customContextMenuRequested(const QPoint &pos);
    void on_tableView_clicked(const QModelIndex &index);

private:
    int makeId();   //rowcount()를 활용한 key 생성
    Ui::ClientManagerForm *ui;
    QSqlQueryModel *q;
    QSqlQueryModel *searchQuery;
    QMenu* menu;
};

#endif // CLIENTMANAGERFORM_H
