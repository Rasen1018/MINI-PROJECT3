#include "productmanagerform.h"
#include "ui_productmanagerform.h"
#include "productitem.h"

#include <QFile>
#include <QMenu>
#include <QMessageBox>

ProductManagerForm::ProductManagerForm(QWidget* parent) :
    QWidget(parent), ui(new Ui::ProductManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 640 << 480;
    ui->splitter->setSizes(sizes);

    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeItem()));

    menu = new QMenu;
    menu->addAction(removeAction);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeWidget->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->searchTreeWidget->header()->
            setSectionResizeMode(QHeaderView::Stretch);
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
        this, SLOT(showContextMenu(QPoint)));

    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
        this, SLOT(on_searchPushButton_clicked()));

    //connect(ui->, this, SIGNAL(dataSent(ProductItem*))

    QFile file("productlist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");
        if (row.size()) {
            int id = row[0].toInt();
            int price = row[2].toInt();
            int stock = row[3].toInt();
            ProductItem* p = new ProductItem(id, row[1], price, stock, row[4]);
            ui->treeWidget->addTopLevelItem(p);
            productList.insert(id, p);
        }
    }
    file.close( );
}

ProductManagerForm::~ProductManagerForm() {

    delete ui;
    QFile file("productlist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (const auto& v : productList) {
        ProductItem* p = v;
        out << p->id() << ", " << p->getName() << ", ";
        out << p->getPrice() << ", ";
        out << p->getStock() << ", ";
        out << p->getCategory() << "\n";
    }
    file.close( );
}

int ProductManagerForm::makeId() {
    if (productList.size() == 0) {
        return 3001;
    }
    else {
        auto id = productList.lastKey();
        return ++id;
    }
}

void ProductManagerForm::showContextMenu(const QPoint& pos) {

    QPoint globalPos = ui->treeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void ProductManagerForm::removeItem() {

    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (item != nullptr) {
        productList.remove(item->text(0).toInt());
        ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(item));
        // 제거후 업데이트
        ui->treeWidget->update();
    }
}

void ProductManagerForm::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->idLineEdit->setText(item->text(0));
    ui->productNameLineEdit->setText(item->text(1));
    ui->priceLlineEdit->setText(item->text(2));
    ui->stockLlineEdit->setText(item->text(3));
    ui->categoryLineEdit->setText(item->text(4));
}

void ProductManagerForm::on_clearPushButton_clicked()
{
    ui->idLineEdit->clear();
    ui->productNameLineEdit->clear();
    ui->priceLlineEdit->clear();
    ui->stockLlineEdit->clear();
    ui->categoryLineEdit->clear();
}

void ProductManagerForm::on_addPushButton_clicked()
{
    QString name, category;
    int price, stock;
    int id = makeId( );
    name= ui->productNameLineEdit->text();
    price = ui->priceLlineEdit->text().toInt();
    stock = ui->stockLlineEdit->text().toInt();
    category = ui->categoryLineEdit->text();

    if (name.length()) {
        ProductItem* p = new ProductItem(id, name, price, stock, category);
        productList.insert(id, p);
        ui->treeWidget->addTopLevelItem(p);
    }
}

void ProductManagerForm::on_modifyPushButton_clicked()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (item != nullptr) {
        int key = item->text(0).toInt();
        ProductItem* p = productList[key];
        int price, stock;
        QString name, category;
        name = ui->productNameLineEdit->text();
        price = ui->priceLlineEdit->text().toInt();
        stock = ui->stockLlineEdit->text().toInt();
        category = ui->categoryLineEdit->text();
        p->setName(name);
        p->setPrice(price);
        p->setStock(stock);
        p->setCategory(category);
        productList[key] = p;
    }
}

void ProductManagerForm::on_searchPushButton_clicked()
{
    ui->searchTreeWidget->clear();
    int i = ui->searchComboBox->currentIndex();
    auto flag = (i) ? Qt::MatchCaseSensitive | Qt::MatchContains
        : Qt::MatchCaseSensitive;
    {
        auto items = ui->treeWidget->findItems(ui->searchLineEdit->text(), flag, i);

        foreach(auto i, items) {
            ProductItem* p = static_cast<ProductItem*>(i);
            int id = p->id();
            QString name = p->getName();
            int price = p->getPrice();
            int stock = p->getStock();
            QString category = p->getCategory();
            ProductItem* item = new ProductItem(id, name, price, stock, category);
            ui->searchTreeWidget->addTopLevelItem(item);
        }
    }
}

void ProductManagerForm::receiveName(QString string) {

        auto items = ui->treeWidget->findItems(string, Qt::MatchContains, 1);

        if (string=="") return;
        foreach(auto i, items) {

            ProductItem* p = static_cast<ProductItem*>(i);
            int id = p->id();
            QString name = p->getName();
            int price = p->getPrice();
            int stock = p->getStock();
            QString category = p->getCategory();
            ProductItem* item = new ProductItem(id, name, price, stock, category);
            emit producdataSent(item);
        }
}

void ProductManagerForm::receiveName(int id) {

        auto items = ui->treeWidget->findItems(QString::number(id), Qt::MatchCaseSensitive|Qt::MatchContains, 0);

        //if (string=="") return;
        foreach(auto i, items) {

            ProductItem* p = static_cast<ProductItem*>(i);
            int id = p->id();
            QString name = p->getName();
            int price = p->getPrice();
            int stock = p->getStock();
            QString category = p->getCategory();
            ProductItem* item = new ProductItem(id, name, price, stock, category);
            emit producdataSent(item);

        }
}

void ProductManagerForm::receiveCategory(QString string)
{
        auto items = ui->treeWidget->findItems(string, Qt::MatchContains, 4);

        if (string=="") return;
        foreach(auto i, items) {

            ProductItem* p = static_cast<ProductItem*>(i);
            int id = p->id();
            QString name = p->getName();
            int price = p->getPrice();
            int stock = p->getStock();
            QString category = p->getCategory();
            ProductItem* item = new ProductItem(id, name, price, stock, category);
            emit producdataSent(item);
        }
}

void ProductManagerForm::inventoryChanged(int id, int order) {

    ProductItem* p = productList[id];
    if (p->getStock() > order) {
        int stock = p->getStock() - order;
        p->setStock(stock);
        productList[id] = p;
    }
}



