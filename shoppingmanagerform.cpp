#include "shoppingmanagerform.h"
#include "ui_shoppingmanagerform.h"
#include "clientitem.h"
#include "productitem.h"
#include "shoppingitem.h"

#include <QFile>
#include <QMenu>
#include <QMessageBox>

ShoppingManagerForm::ShoppingManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShoppingManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 640 << 480;
    ui->splitter->setSizes(sizes);

    connect(ui->orderLineEdit, &QLineEdit::textChanged, this, [=](){
        QString order = ui->orderLineEdit->text();
#if 1
        if(ui->productTreeWidget->currentItem()==nullptr) {
            int price =
                    (ui->shopTreeWidget->currentItem()->text(5).toInt())/(ui->shopTreeWidget->currentItem()->text(4).toInt());
            int totalAmount = price * order.toInt();
            ui->totalLineEdit->setText(QString::number(totalAmount));
        }
        else {
            QString price = ui->productTreeWidget->currentItem()->text(2);
            int amount = price.toInt() * order.toInt();
            ui->totalLineEdit->setText(QString::number(amount));
        }
#else
        if(ui->productTreeWidget->currentItem()!=nullptr) {
            QString price = ui->productTreeWidget->currentItem()->text(2);
            int amount = price.toInt() * order.toInt();
            ui->totalLineEdit->setText(QString::number(amount));
        }
#endif
    });

    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeItem()));

    menu = new QMenu;
    menu->addAction(removeAction);
    ui->shopTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->shopTreeWidget->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->clientTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchTreeWidget->header()->
            setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(ui->shopTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
        this, SLOT(showContextMenu(QPoint)));

    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
        this, SLOT(on_searchPushButton_clicked()));

    QFile file("shoplist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");
        if (row.size()) {
            int id = row[0].toInt();
            int CID = row[1].toInt();
            int PID = row[2].toInt();
            int order = row[4].toInt();
            int totalPrice = row[5].toInt();
            ShoppingItem* s = new ShoppingItem(id, CID, PID, row[3], order, totalPrice);
            ui->shopTreeWidget->addTopLevelItem(s);
            shopList.insert(id, s);
        }
    }
    file.close( );
}

ShoppingManagerForm::~ShoppingManagerForm()
{
    delete ui;
    QFile file("shoplist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (const auto& v : shopList) {
        ShoppingItem* s = v;
        out << s->id() << ", " << s->getCID() << ", ";
        out << s->getPID() << ", ";
        out << s->getTime() << ", ";
        out << s->getAmount() << ", ";
        out << s->getTotalPrice() << "\n";
    }
    file.close( );
}

int ShoppingManagerForm::makeId()
{
    if (shopList.size()==0)
    {
        return 110001;
    }
    else {
        auto id = shopList.lastKey();
        return ++id;
    }
}

void ShoppingManagerForm::showContextMenu(const QPoint& pos) {

    QPoint globalPos = ui->shopTreeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void ShoppingManagerForm::removeItem() {

    QTreeWidgetItem* item = ui->shopTreeWidget->currentItem();
    if (item != nullptr) {
        shopList.remove(item->text(0).toInt());
        ui->shopTreeWidget->takeTopLevelItem(ui->shopTreeWidget->indexOfTopLevelItem(item));
        // 제거후 업데이트
        ui->shopTreeWidget->update();
    }
}

void ShoppingManagerForm::shopReceiveData(ProductItem *p)
{

    int id = p->id();
    QString name = p->getName();
    int price = p->getPrice();
    int stock = p->getStock();
    QString category = p->getCategory();
    ProductItem* item = new ProductItem(id, name, price, stock, category);
    ui->productTreeWidget->addTopLevelItem(item);

}

void ShoppingManagerForm::on_showLineEdit_returnPressed()
{
    int i = ui->showComboBox->currentIndex();

    if(i==0) {
        ui->clientTreeWidget->clear();
        QString name = ui->showLineEdit->text();
        emit clientDataSent(name);
    }

    if(i==1) {
        ui->productTreeWidget->clear();
    QString name = ui->showLineEdit->text();
        emit dataSent(name);
    }

    if(i==2) {
        ui->productTreeWidget->clear();
        QString name = ui->showLineEdit->text();
        emit categoryDataSent(name);
    }
}

void ShoppingManagerForm::on_clientTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->clientNameLineEdit->setText(item->text(0));
//    QString text0 = item->text(0);
//    QString text1 = item->text(1);
//    ui->clientNameLineEdit->setText(text0+"("+text1+")");
}


void ShoppingManagerForm::on_productTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->pdNameLlineEdit->setText(item->text(0));
    QString order = ui->orderLineEdit->text();
    if(order=="") {
    ui->totalLineEdit->setText(item->text(2));
    }
    else {
        int amount = order.toInt()*(item->text(2).toInt());
        ui->totalLineEdit->setText(QString::number(amount));
    }
}

void ShoppingManagerForm::on_shopTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    ui->clientTreeWidget->clear();
    ui->productTreeWidget->clear();
    Q_UNUSED(column);
    ui->idLineEdit->setText(item->text(0));
    ui->clientNameLineEdit->setText(item->text(1));
    ui->pdNameLlineEdit->setText(item->text(2));
    ui->timeLlineEdit->setText(item->text(3));
    ui->orderLineEdit->setText(item->text(4));
    ui->totalLineEdit->setText(item->text(5));
    emit clientIdSent(item->text(1).toInt());
    emit productIdSent(item->text(2).toInt());
}

void ShoppingManagerForm::on_addPushButton_clicked()
{
    QString time;
    int CID, PID, order, totalPrice;
    int id = makeId( );
    CID= ui->clientNameLineEdit->text().toInt();
    PID = ui->pdNameLlineEdit->text().toInt();
    time = ui->timeLlineEdit->text();
    totalPrice = ui->totalLineEdit->text().toInt();

    if (ui->productTreeWidget->currentItem() == nullptr) {
        int stock = ui->productTreeWidget->topLevelItem(0)->text(3).toInt();
        qDebug() << stock;
        order = ui->orderLineEdit->text().toInt();

        if (stock > order) {
            emit(inventorySent(PID, order));
            ShoppingItem* s = new ShoppingItem(id, CID, PID, time, order, totalPrice);
            shopList.insert(id, s);
            ui->shopTreeWidget->addTopLevelItem(s);
        }

        else if (stock < order) {
            QMessageBox::warning(this, tr("ERROR"), tr("Inventory Overflow"));
            return;
        }
    }

    else {
        int stock = ui->productTreeWidget->currentItem()->text(3).toInt();
        order = ui->orderLineEdit->text().toInt();

        if (stock < order) {
            QMessageBox::warning(this, tr("ERROR"), tr("Inventory Overflow"));
            return;
        }

        else if (stock > order) {
            emit(inventorySent(PID, order));
            ShoppingItem* s = new ShoppingItem(id, CID, PID, time, order, totalPrice);
            shopList.insert(id, s);
            ui->shopTreeWidget->addTopLevelItem(s);
        }
    }
}

void ShoppingManagerForm::on_modifyPushButton_clicked()
{
    QTreeWidgetItem* item = ui->shopTreeWidget->currentItem();
    if (item != nullptr) {
        int key = item->text(0).toInt();
        ShoppingItem* s = shopList[key];
        QString time;
        int CID, PID, order, totalPrice, prevOrder;
        CID= ui->clientNameLineEdit->text().toInt();
        PID = ui->pdNameLlineEdit->text().toInt();
        time = ui->timeLlineEdit->text();
        totalPrice = ui->totalLineEdit->text().toInt();
        prevOrder = item->text(4).toInt();


        if (ui->productTreeWidget->currentItem() == nullptr) {
            int stock = ui->productTreeWidget->topLevelItem(0)->text(3).toInt();
            qDebug() << stock;
            order = ui->orderLineEdit->text().toInt();

            if (stock > (order-prevOrder)) {
                emit(inventorySent(PID, (order-prevOrder)));
                s->setClient(CID);
                s->setProduct(PID);
                s->setTime(time);
                s->setAmount(order);
                s->setTotalPrice(totalPrice);
                shopList[key] = s;
            }

            else if (stock < (order-prevOrder)) {
                QMessageBox::warning(this, tr("ERROR"), tr("Inventory Overflow"));
                return;
            }
        }

        else {
            int stock = ui->productTreeWidget->currentItem()->text(3).toInt();
            order = ui->orderLineEdit->text().toInt();

            if (stock < (order-prevOrder)) {
                QMessageBox::warning(this, tr("ERROR"), tr("Inventory Overflow"));
                return;
            }

            else if (stock > (order-prevOrder)) {
                emit(inventorySent(PID, (order-prevOrder)));
                s->setClient(CID);
                s->setProduct(PID);
                s->setTime(time);
                s->setAmount(order);
                s->setTotalPrice(totalPrice);
                shopList[key] = s;
            }
        }
    }
}

void ShoppingManagerForm::on_clearPushButton_clicked()
{
    ui->idLineEdit->clear();
    ui->clientNameLineEdit->clear();
    ui->pdNameLlineEdit->clear();
    ui->timeLlineEdit->clear();
    ui->orderLineEdit->clear();
    ui->totalLineEdit->clear();
}


void ShoppingManagerForm::on_searchPushButton_clicked()
{
    ui->searchTreeWidget->clear();
    int i = ui->searchComboBox->currentIndex();

    if (i == 0 || i == 1 || i == 2) {
        auto items = ui->shopTreeWidget->findItems(ui->searchLineEdit->text(),
                                                   Qt::MatchCaseSensitive, i);

        foreach(auto i, items) {
            ShoppingItem* s = static_cast<ShoppingItem*>(i);
            int id = s->id();
            int clientId = s->getCID();
            int productId = s->getPID();
            QString sellTime = s->getTime();
            int sellAmount = s->getAmount();
            int totalPrice = s->getTotalPrice();
            ShoppingItem* item = new ShoppingItem(id, clientId, productId, sellTime, sellAmount, totalPrice);
            ui->searchTreeWidget->addTopLevelItem(item);
        }
    }

    else {
        auto items = ui->shopTreeWidget->findItems(ui->searchLineEdit->text(),
                                               Qt::MatchContains | Qt::MatchCaseSensitive, i);

        foreach(auto i, items) {
            ShoppingItem* s = static_cast<ShoppingItem*>(i);
            int id = s->id();
            int clientId = s->getCID();
            int productId = s->getPID();
            QString sellTime = s->getTime();
            int sellAmount = s->getAmount();
            int totalPrice = s->getTotalPrice();
            ShoppingItem* item = new ShoppingItem(id, clientId, productId, sellTime, sellAmount, totalPrice);
            ui->searchTreeWidget->addTopLevelItem(item);
        }
    }

}

