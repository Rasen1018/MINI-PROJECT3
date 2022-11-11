#include "productmanagerform.h"
#include "ui_productmanagerform.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QTreeWidgetItem>

ProductManagerForm::ProductManagerForm(QWidget* parent) :
    QWidget(parent), ui(new Ui::ProductManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 640 << 480;            // 위젯 사이즈 설정
    ui->splitter->setSizes(sizes);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 삭제 Action 연결
    QAction* removeAction = new QAction(tr("&Remove"));
    menu = new QMenu;
    menu->addAction(removeAction);
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeItem()));

    connect(ui->searchLineEdit, SIGNAL(returnPressed()),    // 엔터 누르면 버튼 클릭
        this, SLOT(on_searchPushButton_clicked()));

    q = new QSqlQueryModel;
    searchQuery = new QSqlQueryModel;
    q->setQuery("select * from product order by p_id");
    q->setHeaderData(0, Qt::Horizontal, tr("ID"));
    q->setHeaderData(1, Qt::Horizontal, tr("Name"));
    q->setHeaderData(2, Qt::Horizontal, tr("Price"));
    q->setHeaderData(3, Qt::Horizontal, tr("Stock"));
    q->setHeaderData(4, Qt::Horizontal, tr("Category"));

    ui->tableView->setModel(q);
}

ProductManagerForm::~ProductManagerForm() {

    delete ui;
    delete q;
    delete searchQuery;
}

int ProductManagerForm::makeId() {    // 고객 ID 생성 함수
    if (q->rowCount()==0) {
        return 3001;
    }
    else {
        int i = q->rowCount();
        auto id = q->data(q->index(i-1,0)).toInt();
        return ++id;
    }
}

void ProductManagerForm::removeItem() {     // TreeWidget에 있는 고객 리스트 삭제

    int row = ui->tableView->currentIndex().row();
    int item = q->data(q->index(row, 0)).toInt();
    q->setQuery(QString("delete from product where p_id = '%1'").arg(item));
    q->setQuery("select * from product order by p_id");
}

void ProductManagerForm::nameReceived(QString name) {
    QSqlQueryModel query;
    query.setQuery(
                QString("select * from product where p_name like '%%1%' order by p_id")
                .arg(name));

    for(int i=0;i<query.rowCount();i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        for(int j = 0; j<5; j++) {
            item->setText(j, query.data(query.index(i, j)).toString());
        }
        emit productItemSent(item);
    }
}

void ProductManagerForm::categoryReceived(QString category) {
    QSqlQueryModel query;
    query.setQuery(
                QString("select * from product where p_category like '%%1%' order by p_id")
                .arg(category));

    for(int i=0;i<query.rowCount();i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        for(int j = 0; j<5; j++) {
            item->setText(j, query.data(query.index(i, j)).toString());
        }
        emit productItemSent(item);
    }
}

void ProductManagerForm::idReceived(int id) {

    QSqlQueryModel query;
    query.setQuery(QString("select * from product where p_id = '%1'").arg(id));

    QTreeWidgetItem *item = new QTreeWidgetItem;
    for(int i=0;i<5;i++) {
        item->setText(i, query.data(query.index(0, i)).toString());
    }
    emit productItemSent(item);
}

void ProductManagerForm::inventoryChanged(int id, int order) {
    // ShoppingManagerForm에서 주문하는 경우 id, 주문량(order)을 가져옴
    QSqlQueryModel query;
    query.setQuery(
      QString("select * from product where p_id = '%2'").arg(id));

    if (query.data(query.index(0,3)).toInt() > order) {    // 재고가 주문량보다 많을 때만
        query.setQuery(
          QString("update product set p_stock = p_stock - '%1' where p_id = '%2'")
          .arg(order).arg(id));
    }
    q->setQuery("select * from product order by p_id");
}

void ProductManagerForm::on_clearPushButton_clicked()   // 클리어 버튼 클릭시
{   // 입력창 내용 클리어
    ui->idLineEdit->clear();
    ui->productNameLineEdit->clear();
    ui->priceLlineEdit->clear();
    ui->stockLlineEdit->clear();
    ui->categoryLineEdit->clear();
}

void ProductManagerForm::on_addPushButton_clicked()     // 추가 버튼 클릭시
{
    QString name, category;
    int price, stock;
    int id = makeId();
    name = ui->productNameLineEdit->text();
    price = ui->priceLlineEdit->text().toInt();
    stock = ui->stockLlineEdit->text().toInt();
    category = ui->categoryLineEdit->text();
    q->setQuery(QString("call add_product('%1', '%2', '%3', '%4', '%5')")
                .arg(id).arg(name).arg(price).arg(stock).arg(category));

    q->setQuery("select * from product order by p_id");
}

void ProductManagerForm::on_modifyPushButton_clicked()    // 수정 버튼 클릭시
{
    int id = ui->idLineEdit->text().toInt();
    int price, stock; QString name, category;

    name = ui->productNameLineEdit->text();
    price = ui->priceLlineEdit->text().toInt();
    stock = ui->stockLlineEdit->text().toInt();
    category = ui->categoryLineEdit->text();
    q->setQuery(QString("update product set p_name = '%1', p_price = '%2',"
                        "p_stock = '%3', p_category = '%4' where p_id = '%5'")
                .arg(name).arg(price).arg(stock).arg(category).arg(id));

    q->setQuery("select * from product order by p_id");
}

void ProductManagerForm::on_searchPushButton_clicked()    // 검색 버튼 클릭시
{
    if(ui->searchLineEdit->text()==nullptr) return;

    int searchCase = ui->searchComboBox->currentIndex();
    searchQuery->setQuery("select * from product");
    searchQuery->setHeaderData(0, Qt::Horizontal, tr("ID"));
    searchQuery->setHeaderData(1, Qt::Horizontal, tr("Name"));
    searchQuery->setHeaderData(2, Qt::Horizontal, tr("Price"));
    searchQuery->setHeaderData(3, Qt::Horizontal, tr("Stock"));
    searchQuery->setHeaderData(4, Qt::Horizontal, tr("Category"));

    switch(searchCase) {
    case 0: {
        int id = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(
                    QString("select * from product where p_id = '%1'").arg(id));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 1: {
        QString name = ui->searchLineEdit->text();
        searchQuery->setQuery(
                    QString("select * from product where p_name like '%%1%'").arg(name));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 2: {
        int price = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(
                    QString("select * from product where p_price like '%%1%'").arg(price));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 3: {
        int stock = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(
                    QString("select * from product where p_stock like '%%1%'").arg(stock));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 4: {
        QString category = ui->searchLineEdit->text();
        searchQuery->setQuery(
                    QString("select * from product where p_category like '%%1%'").arg(category));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    }
}

void ProductManagerForm::on_tableView_clicked(const QModelIndex &index)
{
    int row = index.row();
    ui->idLineEdit->setText(q->data(q->index(row, 0)).toString());
    ui->productNameLineEdit->setText(q->data(q->index(row, 1)).toString());
    ui->priceLlineEdit->setText(QString::number(q->data(q->index(row, 2)).toInt()));
    ui->stockLlineEdit->setText(q->data(q->index(row, 3)).toString());
    ui->categoryLineEdit->setText(q->data(q->index(row, 4)).toString());
}

void ProductManagerForm::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
    if(ui->tableView->indexAt(pos).isValid())
            menu->exec(globalPos);
}
