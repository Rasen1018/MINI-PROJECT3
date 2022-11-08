#include "clientitem.h"

#include <QLabel>
#include <iostream>

using namespace std;

ClientItem::ClientItem(int id, QString name, QString gender,
                       int age, QString phoneNumber, QString address)
{
    setText(0, QString::number(id));
    setText(1, name);
    setText(2, gender);
    setText(3, QString::number(age));
    setText(4, phoneNumber);
    setText(5, address);
}

QString ClientItem::getName() const
{
    return text(1);
}

void ClientItem::setName(QString& name)
{
    setText(1, name);
}

QString ClientItem::getGender() const
{
    return text(2);
}

void ClientItem::setGender(QString& gender)
{
    setText(2, gender);
}

int ClientItem::getAge() const
{
    return text(3).toInt();
}

void ClientItem::setAge(int age)
{
    setText(3, QString::number(age));
}

QString ClientItem::getPhoneNumber() const
{
    return text(4);
}

void ClientItem::setPhoneNumber(QString& phoneNumber)
{
    setText(4, phoneNumber);    // c_str() --> const char*
}

QString ClientItem::getAddress() const
{
    return text(5);
}

void ClientItem::setAddress(QString& address)
{
    setText(5, address);
}

int ClientItem::id() const
{
    return text(0).toInt();
}

// Define copy assignment operator.
bool ClientItem::operator==(const ClientItem &other) const {
    return (this->text(1) == other.text(1));
}
