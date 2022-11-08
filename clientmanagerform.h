#ifndef CLIENTMANAGERFORM_H
#define CLIENTMANAGERFORM_H

#include <QWidget>
#include <QHash>

#include "clientitem.h"

class QMenu;
class QTableView;
class QTreeWidgetItem;
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
    bool createConnection();

signals:
    void getAllClient(QStringList);

private slots:
    /* QTreeWidget을 위한 슬롯 */
    void removeItem();
    /* QAction을 위한 슬롯 */
    void on_addPushButton_clicked();
    void on_modifyPushButton_clicked();
    void on_searchPushButton_clicked();
    void on_clearPushButton_clicked();

    void on_tableView_customContextMenuRequested(const QPoint &pos);

    void on_tableView_clicked(const QModelIndex &index);

private:
    int makeId();
    QMap<int, ClientItem*> clientList;
    Ui::ClientManagerForm *ui;
    QSqlQueryModel *q;
    QSqlQueryModel *searchQuery;
    QMenu* menu;
};

#endif // CLIENTMANAGERFORM_H
