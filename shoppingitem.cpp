#include "shoppingitem.h"
#include <QLabel>
#include <iostream>

using namespace std;

ShoppingItem::ShoppingItem(int id, int clientId, int productId, QString sellTime, int sellAmount, int totalPrice) {
	setText(0, QString::number(id));
	setText(1, QString::number(clientId));
	setText(2, QString::number(productId));
	setText(3, sellTime);
	setText(4, QString::number(sellAmount));
    setText(5, QString::number(totalPrice));
}

int ShoppingItem::id() const {

    return text(0).toInt();
}

int ShoppingItem::getCID() const {

    return text(1).toInt();
}

void ShoppingItem::setClient(int clinetId) {

    setText(1, QString::number(clinetId));
}

int ShoppingItem::getPID() const {

    return text(2).toInt();
}

void ShoppingItem::setProduct(int productId) {
	
	setText(2, QString::number(productId));
}

QString ShoppingItem::getTime() const {

    return text(3);
}

void ShoppingItem::setTime(QString sellTime) {

	setText(3, sellTime);
}

int ShoppingItem::getAmount() const {
	
    return text(4).toInt();
}

void ShoppingItem::setAmount(int sellAmount) {

	setText(4, QString::number(sellAmount));
}

int ShoppingItem::getTotalPrice() const {

    return text(5).toInt();
}

void ShoppingItem::setTotalPrice(int totalPrice)
{
    setText(5, QString::number(totalPrice));
}

bool ShoppingItem::operator==(const ShoppingItem& other) const {

	return (this->text(1) == other.text(1));
}
