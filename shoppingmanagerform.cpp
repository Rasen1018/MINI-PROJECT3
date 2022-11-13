#include "shoppingmanagerform.h"
#include "ui_shoppingmanagerform.h"

#include <QMenu>
#include <QMessageBox>
#include <QStandardItem>
#include <QSqlQueryModel>
#include <QStandardItemModel>

ShoppingManagerForm::ShoppingManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShoppingManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 640 << 480;            // 위젯 사이즈 설정
    ui->splitter->setSizes(sizes);

    // header 사이즈 설정
    setHeaderStyle();

    clientModel = new QStandardItemModel(0,6);     // 고객 StandardItemModel 생성, column 사이즈 설정
    ui->clientTreeView->setModel(clientModel);     // TreeView에 고객 모델 연결
    setClientHeader();                            // clientTreeView 헤더 스타일 설정

    productModel = new QStandardItemModel(0,5);    // 제품 StandardItemModel 생성, column 사이즈 설정
    ui->productTreeView->setModel(productModel);   // TreeView에 제품 모델 연결
    setProductHeader();                           // productTreeView 헤더 스타일 설정

    // Model 설정
    q = new QSqlQueryModel;
    q->setQuery("select * from orders order by o_id");  // ORDERS 정보 SELECT 문
    searchQuery = new QSqlQueryModel;                   // 검색을 위한 Query Model

    // shopTableView 헤더 설정
    q->setHeaderData(0, Qt::Horizontal, tr("ID"));
    q->setHeaderData(1, Qt::Horizontal, tr("clientName"));
    q->setHeaderData(2, Qt::Horizontal, tr("productName"));
    q->setHeaderData(3, Qt::Horizontal, tr("time"));
    q->setHeaderData(4, Qt::Horizontal, tr("order"));
    q->setHeaderData(5, Qt::Horizontal, tr("totalAmount"));

    // Viewer에 Model 연결
    ui->shopTableView->setModel(q);

    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeItem()));

    menu = new QMenu;
    menu->addAction(removeAction);

    // 엔터 입력시 검색 버튼 클릭
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
        this, SLOT(on_searchPushButton_clicked()));

/***************************lambda************************/
    // 주문 수량이 바뀔때마다 자동으로 총액을 구해서 입력해주는 lambda식
    connect(ui->orderLineEdit, &QLineEdit::textChanged, this, [=](){
        QString order = ui->orderLineEdit->text();

        // productModel에 제품 정보를 보여주는 데이터가 없을 경우, 주문 목록 TableView에서 가격과 주문 총액을 계산
        if(ui->productTreeView->currentIndex().constInternalPointer()==nullptr) {

            int row = ui->shopTableView->currentIndex().row();
            int price =
                    (q->data(q->index(row, 5)).toInt())/(q->data(q->index(row, 4)).toInt());
            /*  가격  =          주문금액               ÷             주문 수량            */
            int totalAmount = price * order.toInt();    //주문 총액
            ui->totalLineEdit->setText(QString::number(totalAmount));
        }
        // productModel에 데이터가 있는 경우
        else {
            int row = ui->productTreeView->currentIndex().row();
            auto price = (productModel->data(productModel->index(row, 2)));     // 가격 데이터 가져오기
            int amount = price.toInt() * order.toInt();
            ui->totalLineEdit->setText(QString::number(amount));    // 주문 총액 계산
        }
    });
}

ShoppingManagerForm::~ShoppingManagerForm()
{
    delete ui;
    delete q;
    delete searchQuery;
    delete clientModel;
    delete productModel;
}


/***********************************************************************************************/
int ShoppingManagerForm::makeId()       // key 생성
{
    if (q->rowCount()==0)      // DB에 데이터가 없을 경우 ID 110001부터 시작
        return 110001;
    else {                                               // 데이터가 있을 경우
        int i = q->rowCount();
        auto id = q->data(q->index(i-1, 0)).toInt();     // DB 데이터의 마지막 ID를 가져와서 ++id
        return ++id;
    }
}


//__________주문 목록 삭제____________//
void ShoppingManagerForm::removeItem() {

    int row = ui->shopTableView->currentIndex().row();
    int item = q->data(q->index(row, 0)).toInt();       // Query Model에서 삭제하려는 고객의 row를 통해 id를 가져옴
   /* DELETE QUERY 문 */
    q->setQuery(QString("delete from orders where o_id = '%1'").arg(item));     // id를 argument로 받음
    q->setQuery("select * from orders order by o_id");
}


//__________ClientManagerForm에서 StandardItem 가져오는 슬롯____________//
void ShoppingManagerForm::receiveData(QList<QStandardItem *> c)
{
    clientModel->appendRow(c);  // 모델에 item 추가
}


//__________ProductManagerForm에서 StandardItem 가져오는 슬롯____________//
void ShoppingManagerForm::shopReceiveData(QList<QStandardItem *> p)
{
    productModel->appendRow(p); // 모델에 item 추가
}



/***********************************************************************************************/
void ShoppingManagerForm::on_showLineEdit_returnPressed()       // 고객, 제품 정보 검색
{
    if(ui->showLineEdit->text() == nullptr) return;

    int i = ui->showComboBox->currentIndex();

    if(i==0) {      // 고객 이름으로 검색할 경우
        clientModel->removeRows(0, clientModel->rowCount());
        QString name = ui->showLineEdit->text();
        emit clientDataSent(name);      // 고객 이름 전달해주는 시그널 발생
    }

    if(i==1) {      // 제품 이름으로 검색할 경우
        productModel->removeRows(0, productModel->rowCount());
        QString name = ui->showLineEdit->text();
        emit dataSent(name);            // 제품 이름 전달해주는 시그널 발생
    }

    if(i==2) {      // 제품 품목으로 검색할 경우
        productModel->removeRows(0, productModel->rowCount());
        QString name = ui->showLineEdit->text();
        emit categoryDataSent(name);    // 제품 품목 전달해주는 시그널 발생
    }
}


//__________고객 정보 TreeView 클릭 슬롯____________//
void ShoppingManagerForm::on_clientTreeView_clicked(const QModelIndex &index)
{
    int row = index.row();
    ui->clientNameLineEdit->        // 고객 ID 입력
            setText(clientModel->data(clientModel->index(row, 0)).toString());
}


//__________제품 정보 TreeView 클릭 슬롯____________//
void ShoppingManagerForm::on_productTreeView_clicked(const QModelIndex &index)
{
    int row = index.row();
    QString order = ui->orderLineEdit->text();

    ui->pdNameLlineEdit->       // 제품 ID 입력
            setText(productModel->data(productModel->index(row, 0)).toString());

    if(order=="") {             // 주문량이 없는 경우 총액 입력
    ui->totalLineEdit->
            setText(productModel->data(productModel->index(row, 2)).toString());
    }
    else {                      // 주문량이 있는 경우 총액 입력
        int amount = order.toInt()*(productModel->data(productModel->index(row, 2)).toInt());
        // 총액 계산
        ui->totalLineEdit->setText(QString::number(amount));
    }
}



/***********************************************************************************************/
void ShoppingManagerForm::on_addPushButton_clicked()    // 추가 버튼 클릭시
{
    QString time;
    int CID, PID, order, totalPrice;
    int id = makeId();
    CID = ui->clientNameLineEdit->text().toInt();
    PID = ui->pdNameLlineEdit->text().toInt();
    time = ui->timeLlineEdit->text();
    order = ui->orderLineEdit->text().toInt();
    totalPrice = ui->totalLineEdit->text().toInt();

    // 기존의 주문 목록을 이용하여 주문하는 경우(트리 위젯 X)
    if (ui->productTreeView->currentIndex().constInternalPointer() == nullptr) {
        int stock = productModel->data(productModel->index(0, 3)).toInt();
        qDebug() << stock;
        order = ui->orderLineEdit->text().toInt();

        if (stock > order) {     // 재고가 주문보다 많을 경우 정상 주문
            emit inventorySent(PID, order);     // 제품 ID와 주문량을 전달해주는 시그널 발생
            q->setQuery(QString("call add_order('%1', '%2', '%3', '%4', '%5', '%6');")
                        .arg(id).arg(CID).arg(PID).arg(time).arg(order).arg(totalPrice));

            q->setQuery("select * from orders order by o_id");
        }
        else if (stock < order) {       // 재고가 부족할 경우
            QMessageBox::warning(this, tr("ERROR"), tr("Inventory Overflow"));      // 오류 메세지 표시
            return;
        }
    }

    else {      // 제품 정보 보여주는 트리 위젯이 있을 경우
        int row = ui->productTreeView->currentIndex().row();
        int stock = productModel->data(productModel->index(row, 3)).toInt();
        order = ui->orderLineEdit->text().toInt();

        if (stock < order) {        // 재고가 부족할 경우
            QMessageBox::warning(this, tr("ERROR"), tr("Inventory Overflow"));      // 오류 메세지 표시
            return;
        }

        else if (stock > order) {         // 재고가 주문보다 많을 경우 정상 주문
            emit inventorySent(PID, order);    // 제품 ID와 주문량을 전달해주는 시그널 발생
            q->setQuery(QString("call add_order('%1', '%2', '%3', '%4', '%5', '%6');")
                        .arg(id).arg(CID).arg(PID).arg(time).arg(order).arg(totalPrice));

            q->setQuery("select * from orders order by o_id");
        }
    }
}

//____________________수정 버튼 사용자 슬롯______________________//
void ShoppingManagerForm::on_modifyPushButton_clicked()
{
    int id = ui->idLineEdit->text().toInt();
    int row = ui->shopTableView->currentIndex().row();
    int CID, PID, order, totalPrice, prevOrder; QString date;

    CID = ui->clientNameLineEdit->text().toInt();
    PID = ui->pdNameLlineEdit->text().toInt();
    date = ui->timeLlineEdit->text();
    order = ui->orderLineEdit->text().toInt();
    totalPrice = ui->totalLineEdit->text().toInt();
    prevOrder = q->data(q->index(row, 4)).toInt();      // 기존의 주문량 저장

    // 기존의 주문 목록을 이용하여 주문하는 경우(트리 위젯 X)
    if (ui->productTreeView->currentIndex().constInternalPointer() == nullptr) {
        int stock = productModel->data(productModel->index(0, 3)).toInt();
        qDebug() << stock;
        order = ui->orderLineEdit->text().toInt();

        if (stock > (order-prevOrder)) {    // 현재 주문량이 재고보다 적을 경우 정상 주문
            emit inventorySent(PID, (order-prevOrder));
            q->setQuery(QString("update orders set o_c_id = '%1', o_p_id = '%2',"
                                "o_date = '%3', o_amount = '%4', o_total = '%5' where o_id = '%6'")
                        .arg(CID).arg(PID).arg(date).arg(order).arg(totalPrice).arg(id));

            q->setQuery("select * from orders order by o_id");
        }

        else if (stock < (order-prevOrder)) {     // 현재 주문량이 재고보다 많을 경우
            QMessageBox::warning(this, tr("ERROR"), tr("Inventory Overflow"));      // 에러메세지 표시
            return;
        }
    }

    else {      // 제품 정보 보여주는 트리 위젯이 있을 경우
        int row = ui->productTreeView->currentIndex().row();
        int stock = productModel->data(productModel->index(row, 3)).toInt();
        order = ui->orderLineEdit->text().toInt();

        if (stock < (order-prevOrder)) {     // 재고가 부족하다면
            QMessageBox::warning(this, tr("ERROR"), tr("Inventory Overflow"));      // 에러메세지 표시
            return;
        }

        else if (stock > (order-prevOrder)) {      // 현재 주문량이 재고보다 적다면
            emit inventorySent(PID, (order-prevOrder));    // 현재 주문량을 전달하는 시그널 발생
            q->setQuery(QString("update orders set o_c_id = '%1', o_p_id = '%2',"
                                "o_date = '%3', o_amount = '%4', o_total = '%5' where o_id = '%6'")
                        .arg(CID).arg(PID).arg(date).arg(order).arg(totalPrice).arg(id));

            q->setQuery("select * from orders order by o_id");
        }
    }
}


//____________________검색 버튼 사용자 슬롯______________________//
void ShoppingManagerForm::on_searchPushButton_clicked()
{
    if(ui->searchLineEdit->text()==nullptr) return;

    int searchCase = ui->searchComboBox->currentIndex();
    searchQuery->setQuery("select * from orders");
    searchQuery->setHeaderData(0, Qt::Horizontal, tr("ID"));
    searchQuery->setHeaderData(1, Qt::Horizontal, tr("clientName"));
    searchQuery->setHeaderData(2, Qt::Horizontal, tr("productName"));
    searchQuery->setHeaderData(3, Qt::Horizontal, tr("time"));
    searchQuery->setHeaderData(4, Qt::Horizontal, tr("order"));
    searchQuery->setHeaderData(5, Qt::Horizontal, tr("totalAmount"));

    switch(searchCase) {
    case 0: {
        int id = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(
                    QString("select * from orders where o_id = '%1' order by o_id").arg(id));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 1: {
        int CID = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(
                    QString("select * from orders where o_c_id = '%1' order by o_id").arg(CID));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 2: {
        int PID = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(
                    QString("select * from orders where o_p_id = '%1' order by o_id").arg(PID));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 3: {
        QString date = ui->searchLineEdit->text();
        searchQuery->setQuery(
                    QString("select * from orders where o_date like '%%1%' order by o_id").arg(date));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    }
}


//____________________클리어 버튼 클릭시 슬롯______________________//
void ShoppingManagerForm::on_clearPushButton_clicked()
{
    ui->idLineEdit->clear();
    ui->clientNameLineEdit->clear();
    ui->pdNameLlineEdit->clear();
    ui->timeLlineEdit->clear();
    ui->orderLineEdit->clear();
    ui->totalLineEdit->clear();
}



/***********************************************************************************************/
void ShoppingManagerForm::setHeaderStyle() {
    ui->clientTreeView->header()->
            setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->productTreeView->header()->
            setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->shopTableView->horizontalHeader()->
            setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->clientTreeView->header()->setStyleSheet("QHeaderView {font-weight: bold}");
    ui->productTreeView->header()->setStyleSheet("QHeaderView {font-weight: bold}");
    ui->shopTableView->horizontalHeader()->setStyleSheet("QHeaderView {font-weight: bold; color : sandybrown}");
    ui->searchTableView->horizontalHeader()->setStyleSheet("QHeaderView {font-weight: bold; color : sandybrown}");
}


void ShoppingManagerForm::setClientHeader() {
    clientModel->setHeaderData(0, Qt::Horizontal, tr("CID"));
    clientModel->setHeaderData(1, Qt::Horizontal, tr("cName"));
    clientModel->setHeaderData(2, Qt::Horizontal, tr("Gender"));
    clientModel->setHeaderData(3, Qt::Horizontal, tr("Age"));
    clientModel->setHeaderData(4, Qt::Horizontal, tr("Phone Number"));
    clientModel->setHeaderData(5, Qt::Horizontal, tr("Address"));
}



void ShoppingManagerForm::setProductHeader() {
    productModel->setHeaderData(0, Qt::Horizontal, tr("PID"));
    productModel->setHeaderData(1, Qt::Horizontal, tr("pName"));
    productModel->setHeaderData(2, Qt::Horizontal, tr("Price"));
    productModel->setHeaderData(3, Qt::Horizontal, tr("Stock"));
    productModel->setHeaderData(4, Qt::Horizontal, tr("Category"));
}



void ShoppingManagerForm::on_shopTableView_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->shopTableView->mapToGlobal(pos);
    if(ui->shopTableView->indexAt(pos).isValid())
            menu->exec(globalPos);
}



void ShoppingManagerForm::on_shopTableView_clicked(const QModelIndex &index)
{
    int row = index.row();
    ui->idLineEdit->setText(q->data(q->index(row, 0)).toString());
    ui->clientNameLineEdit->setText(q->data(q->index(row, 1)).toString());
    ui->pdNameLlineEdit->setText((q->data(q->index(row, 2)).toString()));
    ui->timeLlineEdit->setText(q->data(q->index(row, 3)).toString());
    ui->orderLineEdit->setText(q->data(q->index(row, 4)).toString());
    ui->totalLineEdit->setText(q->data(q->index(row, 5)).toString());

    clientModel->removeRows(0, clientModel->rowCount());
    productModel->removeRows(0, productModel->rowCount());
    emit clientIdSent(q->data(q->index(row, 1)).toInt());   // 고객 ID 전달
    emit productIdSent(q->data(q->index(row, 2)).toInt());  // 제품 ID 전달
}


void ShoppingManagerForm::on_searchTableView_clicked(const QModelIndex &index)
{
    int row = index.row();
    clientModel->removeRows(0, clientModel->rowCount());
    productModel->removeRows(0, productModel->rowCount());
    emit clientIdSent(q->data(q->index(row, 1)).toInt());   // 고객 ID 전달
    emit productIdSent(q->data(q->index(row, 2)).toInt());  // 제품 ID 전달
}

