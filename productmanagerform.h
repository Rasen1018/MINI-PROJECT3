#ifndef PRODUCTMANAGERFORM_H
#define PRODUCTMANAGERFORM_H

#include <QWidget>
#include <QHash>

class QMenu;
class QSqlQueryModel;
class QTreeWidgetItem;

namespace Ui {
    class ProductManagerForm;
}

class ProductManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProductManagerForm(QWidget* parent = nullptr);
    ~ProductManagerForm();

signals:
    // shoppingManagerForm에서 id나 제품 이름, 품목을 가져오면
    // 그에 맞는 item을 전달해주는 시그널
    void productItemSent(QTreeWidgetItem*);

private slots:
    /* QTreeWidget을 위한 슬롯 */
    void removeItem();
    void nameReceived(QString);
    void categoryReceived(QString);
    void idReceived(int);
    void inventoryChanged(int, int);
   /* QAction을 위한 슬롯 */
    void on_clearPushButton_clicked();
    void on_addPushButton_clicked();
    void on_modifyPushButton_clicked();
    void on_searchPushButton_clicked();
    void on_tableView_clicked(const QModelIndex &index);

    void on_tableView_customContextMenuRequested(const QPoint &pos);

private:
    int makeId();   //lastKey()를 활용한 key 생성
    Ui::ProductManagerForm* ui;
    QSqlQueryModel *q;
    QSqlQueryModel *searchQuery;
    QMenu* menu;
};

#endif // PRODUCTMANAGERFORM_H

