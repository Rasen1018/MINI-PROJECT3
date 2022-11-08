#ifndef PRODUCTITEM_H
#define PRODUCTITEM_H

#include <QTreeWidgetItem>

class ProductItem : public QTreeWidgetItem
{
public:
    explicit ProductItem(int =0, QString="", int=0, int=0, QString="");

    int id() const;
    QString getName() const;
    void setName(QString productName);
    int getPrice() const;
    void setPrice(int price);
    int getStock() const;
    void setStock(int stock);
    QString getCategory() const;
    void setCategory(QString category);
    bool operator==(const ProductItem& other) const;
};

#endif // PRODUCTITEM_H
