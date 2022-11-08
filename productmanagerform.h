#ifndef PRODUCTMANAGERFORM_H
#define PRODUCTMANAGERFORM_H

#include <QWidget>
#include <QHash>

#include "productitem.h"

class QMenu;
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
    void producdataSent(ProductItem*);

private slots:
    void showContextMenu(const QPoint&);
    void removeItem();
    void receiveName(QString);
    void receiveName(int);
    void receiveCategory(QString);
    void inventoryChanged(int, int);
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_clearPushButton_clicked();
    void on_addPushButton_clicked();
    void on_modifyPushButton_clicked();
    void on_searchPushButton_clicked();

private:
    int makeId();
    QMap<int, ProductItem*> productList;
    Ui::ProductManagerForm* ui;
    QMenu* menu;
};

#endif // PRODUCTMANAGERFORM_H

