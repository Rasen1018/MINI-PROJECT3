#include "productmanagerform.h"
#include "ui_productmanagerform.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QStandardItem>

ProductManagerForm::ProductManagerForm(QWidget* parent) :
    QWidget(parent), ui(new Ui::ProductManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 640 << 480;            // 위젯 사이즈 설정
    ui->splitter->setSizes(sizes);

    // 삭제 Action 연결
    QAction* removeAction = new QAction(tr("&Remove"));
    menu = new QMenu;
    menu->addAction(removeAction);
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeItem()));

    connect(ui->searchLineEdit, SIGNAL(returnPressed()),    // 엔터 누르면 버튼 클릭
        this, SLOT(on_searchPushButton_clicked()));

    // Model 생성
    q = new QSqlQueryModel;
    searchQuery = new QSqlQueryModel;       // 검색을 위한 Query Model

    // PRODUCT TABLE SELECT 문
    q->setQuery("select * from product order by p_id");

    setHeaderStyle();   // TreeView 헤더 스타일 설정
    ui->tableView->setModel(q);     // Viewer에 Model 연결
}

ProductManagerForm::~ProductManagerForm() {

    delete ui;
    delete q;
    delete searchQuery;
}



/***********************************************************************************************/
int ProductManagerForm::makeId() {    // 고객 ID 생성 함수
    if (q->rowCount()==0) {           // DB에 데이터가 없을 경우 ID 3001부터 시작
        return 3001;
    }
    else {                            // 데이터가 있을 경우
        int i = q->rowCount();
        auto id = q->data(q->index(i-1,0)).toInt();     // DB에 있는 마지막 행의 ID를 가져와서 ++id
        return ++id;
    }
}


//___________제품 정보 삭제____________//
void ProductManagerForm::removeItem() {

    int row = ui->tableView->currentIndex().row();
    int item = q->data(q->index(row, 0)).toInt();       // Query Model에서 삭제하려는 고객의 row에 접근해 id를 가져옴
    /* DELETE QUERY문 */
    q->setQuery(QString("delete from product where p_id = '%1'").arg(item));    // id를 argument로 받음
    q->setQuery("select * from product order by p_id");
}



//__________ShoppingManagerForm에 StandardItem 전달____________//
void ProductManagerForm::nameReceived(QString name) {

    QModelIndexList indexes =
            q->match(q->index(0, 1), Qt::EditRole, name, -1, Qt::MatchFlags(Qt::MatchContains));
        //query model에서 q->index(0, 1)의 데이터와 name을 비교해서 데이터의 내용이 name을 포함한다면[flags(Qt::MatchContains)]
        //해당하는 index를 전부 리스트에 저장

    foreach(auto idx, indexes) {        // DB의 모든 데이터 가져오기
        int id = q->data(idx.siblingAtColumn(0)).toInt();
        QString name = q->data(idx.siblingAtColumn(1)).toString();
        int price = q->data(idx.siblingAtColumn(2)).toInt();
        int stock = q->data(idx.siblingAtColumn(3)).toInt();
        QString category = q->data(idx.siblingAtColumn(4)).toString();

        QStringList strings;        // stringList에 저장
        strings << QString::number(id) << name << QString::number(price)
                << QString::number(stock) << category;

        QList<QStandardItem *> item;        // stringList->item으로 저장
        for (int i = 0; i < 5; ++i) {
            item.append(new QStandardItem(strings.at(i)));
        }
        emit productItemSent(item);       // QList<QStandardItem *> item 전달
    }
}



//__________ShoppingManagerForm에 StandardItem 전달____________//
void ProductManagerForm::categoryReceived(QString category) {

    QModelIndexList indexes =
            q->match(q->index(0, 4), Qt::EditRole, category, -1, Qt::MatchFlags(Qt::MatchContains));
        //query model에서 q->index(0, 4)의 데이터와 category를 비교해서
        //데이터의 내용이 category을 포함한다면[flags(Qt::MatchContains)]
        //해당하는 index를 전부 리스트에 저장

    foreach(auto idx, indexes) {
        int id = q->data(idx.siblingAtColumn(0)).toInt();
        QString name = q->data(idx.siblingAtColumn(1)).toString();
        int price = q->data(idx.siblingAtColumn(2)).toInt();
        int stock = q->data(idx.siblingAtColumn(3)).toInt();
        QString category = q->data(idx.siblingAtColumn(4)).toString();

        QStringList strings;
        strings << QString::number(id) << name << QString::number(price)
                << QString::number(stock) << category;

        QList<QStandardItem *> item;
        for (int i = 0; i < 5; ++i) {
            item.append(new QStandardItem(strings.at(i)));
        }
        emit productItemSent(item);       // QList<QStandardItem *> item 전달
    }
}


//__________ShoppingManagerForm에 StandardItem 전달____________//
void ProductManagerForm::idReceived(int id) {

    QModelIndexList indexes =
            q->match(q->index(0, 0), Qt::EditRole, id, 1, Qt::MatchFlags(Qt::MatchCaseSensitive));
        //query model에서 q->index(0, 0)의 데이터와 id를 비교해서 데이터의 내용이 id와 일치한다면[flags(Qt::MatchCaseSensitive)]
        //해당하는 index를 전부 리스트에 저장

    foreach(auto idx, indexes) {
        int id = q->data(idx.siblingAtColumn(0)).toInt();
        QString name = q->data(idx.siblingAtColumn(1)).toString();
        int price = q->data(idx.siblingAtColumn(2)).toInt();
        int stock = q->data(idx.siblingAtColumn(3)).toInt();
        QString category = q->data(idx.siblingAtColumn(4)).toString();

        QStringList strings;
        strings << QString::number(id) << name << QString::number(price)
                << QString::number(stock) << category;

        QList<QStandardItem *> item;
        for (int i = 0; i < 5; ++i) {
            item.append(new QStandardItem(strings.at(i)));
        }
        emit productItemSent(item);       // QList<QStandardItem *> item 전달
    }
}


//__________ShoppingManagerForm에서 주문하는 경우 id, 주문량(order)을 가져옴____________//
void ProductManagerForm::inventoryChanged(int id, int order) {

    QSqlQueryModel query;
    query.setQuery(
      QString("select * from product where p_id = '%2'").arg(id));     // id에 해당하는 제품 정보 검색하는 WHERE문

    if (query.data(query.index(0,3)).toInt() > order) {    // 재고가 주문량보다 많을 때만
        query.setQuery(
          QString("update product set p_stock = p_stock - '%1' where p_id = '%2'")
          .arg(order).arg(id));     // 재고에 주문량 반영하는 UPDATE QUERY문
    }
    q->setQuery("select * from product order by p_id");
}



/***********************************************************************************************/
void ProductManagerForm::on_clearPushButton_clicked()   // 클리어 버튼 클릭시
{   // 입력창 내용 클리어
    ui->idLineEdit->clear();
    ui->productNameLineEdit->clear();
    ui->priceLlineEdit->clear();
    ui->stockLlineEdit->clear();
    ui->categoryLineEdit->clear();
}


//__________추가 버튼 사용자 슬롯____________//
void ProductManagerForm::on_addPushButton_clicked()
{
    int id = makeId();
    QString name = ui->productNameLineEdit->text();
    int price = ui->priceLlineEdit->text().toInt();
    int stock = ui->stockLlineEdit->text().toInt();
    QString category = ui->categoryLineEdit->text();

    // ORACLE SQL PROCEDURE 실행
    q->setQuery(QString("call add_product('%1', '%2', '%3', '%4', '%5')")
                .arg(id).arg(name).arg(price).arg(stock).arg(category));

    // ORDER BY로 정렬
    q->setQuery("select * from product order by p_id");
}


//__________수정 버튼 사용자 슬롯____________//
void ProductManagerForm::on_modifyPushButton_clicked()
{
    int id = ui->idLineEdit->text().toInt();
    QString name = ui->productNameLineEdit->text();
    int price = ui->priceLlineEdit->text().toInt();
    int stock = ui->stockLlineEdit->text().toInt();
    QString category = ui->categoryLineEdit->text();

    // UPDATE QUERY 문
    q->setQuery(QString("update product set p_name = '%1', p_price = '%2',"
                        "p_stock = '%3', p_category = '%4' where p_id = '%5'")
                .arg(name).arg(price).arg(stock).arg(category).arg(id));

    // ORDER BY로 정렬
    q->setQuery("select * from product order by p_id");
}



//__________검색 버튼 사용자 슬롯____________//
void ProductManagerForm::on_searchPushButton_clicked()
{
    if(ui->searchLineEdit->text()==nullptr) return;
    int searchCase = ui->searchComboBox->currentIndex();     // comboBox 별로 항목을 다르게 검색

    // 검색을 위한 QUERY MODEL에 SELECT QUERY문
    searchQuery->setQuery("select * from product");
    // header 설정
    searchQuery->setHeaderData(0, Qt::Horizontal, tr("ID"));
    searchQuery->setHeaderData(1, Qt::Horizontal, tr("Name"));
    searchQuery->setHeaderData(2, Qt::Horizontal, tr("Price"));
    searchQuery->setHeaderData(3, Qt::Horizontal, tr("Stock"));
    searchQuery->setHeaderData(4, Qt::Horizontal, tr("Category"));

    switch(searchCase) {
    case 0: {   // 아이디 검색
        int id = ui->searchLineEdit->text().toInt();
        // 검색을 위한 WHERE 문
        searchQuery->setQuery(
                    QString("select * from product where p_id = '%1'").arg(id));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 1: {   // 제품명 검색
        QString name = ui->searchLineEdit->text();
        // 검색을 위한 WHERE LIKE 문
        searchQuery->setQuery(
                    QString("select * from product where p_name like '%%1%'").arg(name));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 2: {   // 가격 검색
        int price = ui->searchLineEdit->text().toInt();

        searchQuery->setQuery(
                    QString("select * from product where p_price like '%%1%'").arg(price));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 3: {   // 재고 검색
        int stock = ui->searchLineEdit->text().toInt();

        searchQuery->setQuery(
                    QString("select * from product where p_stock like '%%1%'").arg(stock));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 4: {   // 품목별 검색
        QString category = ui->searchLineEdit->text();

        searchQuery->setQuery(
                    QString("select * from product where p_category like '%%1%'").arg(category));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    }
}


/***********************************************************************************************/
void ProductManagerForm::setHeaderStyle() {
    q->setHeaderData(0, Qt::Horizontal, tr("ID"));
    q->setHeaderData(1, Qt::Horizontal, tr("Name"));
    q->setHeaderData(2, Qt::Horizontal, tr("Price"));
    q->setHeaderData(3, Qt::Horizontal, tr("Stock"));
    q->setHeaderData(4, Qt::Horizontal, tr("Category"));

    // 데이터의 크기에 따라 헤더 사이즈 설정
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // 헤더 폰트 설정
    ui->tableView->horizontalHeader()->setStyleSheet("QHeaderView {font-weight: bold; color : sandybrown}");
    ui->searchTableView->horizontalHeader()->setStyleSheet("QHeaderView {font-weight: bold; color : sandybrown}");
}


//__________tableView 클릭시 슬롯____________//
void ProductManagerForm::on_tableView_clicked(const QModelIndex &index)
{
    int row = index.row();
    ui->idLineEdit->setText(q->data(q->index(row, 0)).toString());
    ui->productNameLineEdit->setText(q->data(q->index(row, 1)).toString());
    ui->priceLlineEdit->setText(QString::number(q->data(q->index(row, 2)).toInt()));
    ui->stockLlineEdit->setText(q->data(q->index(row, 3)).toString());
    ui->categoryLineEdit->setText(q->data(q->index(row, 4)).toString());
}

//__________tableView 우클릭 슬롯____________//
void ProductManagerForm::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
    if(ui->tableView->indexAt(pos).isValid())
            menu->exec(globalPos);
}
