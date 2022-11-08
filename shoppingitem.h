#ifndef SHOPPINGITEM_H
#define SHOPPINGITEM_H

#include <QTreeWidgetItem>

class ShoppingItem : public QTreeWidgetItem
{
public:
    explicit ShoppingItem(int =0, int =0, int=0, QString="", int=0, int=0);

    int id() const;
    int getCID() const;
    void setClient(int clinetId);
    int getPID() const;
    void setProduct(int productId);
    QString getTime() const;
    void setTime(QString sellTime);
    int getAmount() const;
    void setAmount(int sellAmount);
    int getTotalPrice() const;
    void setTotalPrice(int totalPrice);
    bool operator==(const ShoppingItem& other) const;
};

#endif // SHOPPINGITEM_H
