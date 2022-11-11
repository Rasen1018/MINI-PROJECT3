#ifndef SHOPPINGMANAGERFORM_H
#define SHOPPINGMANAGERFORM_H

#include <QTreeWidgetItem>
#include <QWidget>

class QMenu;
class ProductManagerForm;
class QSqlQueryModel;

namespace Ui {
class ShoppingManagerForm;
}

class ShoppingManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ShoppingManagerForm(QWidget *parent = nullptr);
    ~ShoppingManagerForm();

signals:
    void clientDataSent(QString);
    void dataSent(QString);
    void clientIdSent(int);
    void productIdSent(int);
    void inventorySent(int, int);
    void categoryDataSent(QString);

private slots:
    /* QTreeWidget을 위한 슬롯 */
    void removeItem();
    void receiveData(QTreeWidgetItem*);
    void shopReceiveData(QTreeWidgetItem*);
    void on_showLineEdit_returnPressed();
    void on_clientTreeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_productTreeWidget_itemClicked(QTreeWidgetItem *item, int column);
    /* QAction을 위한 슬롯 */
    void on_addPushButton_clicked();
    void on_modifyPushButton_clicked();
    void on_clearPushButton_clicked();
    void on_searchPushButton_clicked();

    void on_shopTableView_customContextMenuRequested(const QPoint &pos);

    void on_shopTableView_clicked(const QModelIndex &index);

private:
    int makeId();   //lastKey()를 활용한 key 생성
    QMenu* menu;
    QSqlQueryModel *q;
    QSqlQueryModel *searchQuery;
    Ui::ShoppingManagerForm *ui;
};

#endif // SHOPPINGMANAGERFORM_H
