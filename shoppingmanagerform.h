#ifndef SHOPPINGMANAGERFORM_H
#define SHOPPINGMANAGERFORM_H

#include <QTreeWidgetItem>
#include <QStandardItem>
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
    void on_showLineEdit_returnPressed();
    /* 다른 객체에 데이터 전달을 위한 슬롯 */
    void removeItem();
    void receiveData(QList<QStandardItem *>);
    void shopReceiveData(QList<QStandardItem *>);
    /*QTreeView를 위한 슬롯*/
    void on_clientTreeView_clicked(const QModelIndex &index);
    void on_productTreeView_clicked(const QModelIndex &index);
    /* QAction을 위한 슬롯 */
    void on_addPushButton_clicked();
    void on_modifyPushButton_clicked();
    void on_clearPushButton_clicked();
    void on_searchPushButton_clicked();
    /*QTableView을 위한 슬롯*/
    void setHeaderStyle();
    void setClientHeader();
    void setProductHeader();
    void on_shopTableView_customContextMenuRequested(const QPoint &pos);
    void on_shopTableView_clicked(const QModelIndex &index);
    void on_searchTableView_clicked(const QModelIndex &index);

private:
    int makeId();   //lastKey()를 활용한 key 생성
    QMenu* menu;
    QSqlQueryModel *q;
    QSqlQueryModel *searchQuery;
    QStandardItemModel* clientModel;
    QStandardItemModel* productModel;
    Ui::ShoppingManagerForm *ui;
};

#endif // SHOPPINGMANAGERFORM_H
