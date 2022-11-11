#include "shoppingmanagerform.h"
#include "ui_shoppingmanagerform.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlQueryModel>

ShoppingManagerForm::ShoppingManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShoppingManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 640 << 480;            // 위젯 사이즈 설정
    ui->splitter->setSizes(sizes);

    // header 사이즈 설정
    ui->clientTreeWidget->header()->
            setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->productTreeWidget->header()->
            setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->shopTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    q = new QSqlQueryModel;
    q->setQuery("select * from orders order by o_id");
    searchQuery = new QSqlQueryModel;
    q->setHeaderData(0, Qt::Horizontal, tr("ID"));
    q->setHeaderData(1, Qt::Horizontal, tr("clientName"));
    q->setHeaderData(2, Qt::Horizontal, tr("productName"));
    q->setHeaderData(3, Qt::Horizontal, tr("time"));
    q->setHeaderData(4, Qt::Horizontal, tr("order"));
    q->setHeaderData(5, Qt::Horizontal, tr("totalAmount"));

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

        // 제품 정보를 보여주는 TreeWidget이 없을 경우 주문 목록 TreeWidget에서 가격과 주문 총액을 계산
        if(ui->productTreeWidget->currentItem()==nullptr) {
            int row = ui->shopTableView->currentIndex().row();
            int price =  // 가격 = 주문 수량/주문 총액
                    (q->data(q->index(row, 5)).toInt())/(q->data(q->index(row, 4)).toInt());

            int totalAmount = price * order.toInt();    //주문 총액
            ui->totalLineEdit->setText(QString::number(totalAmount));
        }
        // 제품 정보를 보여주는 TreeWidget이 있을 경우
        else {
            QString price = ui->productTreeWidget->currentItem()->text(2);  // 가격 가져옴
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
}

int ShoppingManagerForm::makeId()       // key 생성
{
    if (q->rowCount()==0)
        return 110001;
    else {
        int i = q->rowCount();
        auto id = q->data(q->index(i-1, 0)).toInt();
        return ++id;
    }
}

void ShoppingManagerForm::removeItem() {    // 아이템 삭제 함수

    int row = ui->shopTableView->currentIndex().row();
    int item = q->data(q->index(row, 0)).toInt();
    q->setQuery(QString("delete from orders where o_id = '%1'").arg(item));
    q->setQuery("select * from orders order by o_id");
}

void ShoppingManagerForm::receiveData(QTreeWidgetItem *c)
    // ClientManagerForm에서 id, 고객 이름으로 검색된 Item을 가져오는 슬롯
{
    ui->clientTreeWidget->addTopLevelItem(c);     // 고객 정보 TreeWidget에 추가
}

void ShoppingManagerForm::shopReceiveData(QTreeWidgetItem *p)
    // ProductManagerForm에서 id, 제품 이름, 품목으로 검색된 Item을 가져오는 슬롯
{
    ui->productTreeWidget->addTopLevelItem(p);     // 제품 정보 TreeWidget에 추가
}

// ShoppingManagerForm에서 고객 리스트, 제품 리스트를 검색해서 띄워주기
void ShoppingManagerForm::on_showLineEdit_returnPressed()
{
    if(ui->showLineEdit->text() == nullptr) return;

    int i = ui->showComboBox->currentIndex();

    if(i==0) {      // 고객 이름으로 검색할 경우
        ui->clientTreeWidget->clear();
        QString name = ui->showLineEdit->text();
        emit clientDataSent(name);      // 고객 이름 전달해주는 시그널 발생
    }

    if(i==1) {      // 제품 이름으로 검색할 경우
        ui->productTreeWidget->clear();
    QString name = ui->showLineEdit->text();
        emit dataSent(name);            // 제품 이름 전달해주는 시그널 발생
    }

    if(i==2) {      // 제품 품목으로 검색할 경우
        ui->productTreeWidget->clear();
        QString name = ui->showLineEdit->text();
        emit categoryDataSent(name);    // 제품 품목 전달해주는 시그널 발생
    }
}

void ShoppingManagerForm::on_clientTreeWidget_itemClicked   // 고객 정보 TreeWidget 클릭시
(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->clientNameLineEdit->setText(item->text(0));     // 고객 이름 입력
}


void ShoppingManagerForm::on_productTreeWidget_itemClicked   // 제품 정보 TreeWidget 클릭시
(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->pdNameLlineEdit->setText(item->text(0));    // 제품 이름 입력
    QString order = ui->orderLineEdit->text();
    // 주문량이 없다면 주문 총액에 제품 가격 입력
    if(order=="") {
    ui->totalLineEdit->setText(item->text(2));
    }
    // 주문량이 있다면 총액 입력
    else {
        int amount = order.toInt()*(item->text(2).toInt());     // 총액 계산
        ui->totalLineEdit->setText(QString::number(amount));
    }
}

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
    if (ui->productTreeWidget->currentItem() == nullptr) {
        int stock = ui->productTreeWidget->topLevelItem(0)->text(3).toInt();
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
        int stock = ui->productTreeWidget->currentItem()->text(3).toInt();
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

void ShoppingManagerForm::on_modifyPushButton_clicked()     // 수정버튼 클릭시
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
    if (ui->productTreeWidget->currentItem() == nullptr) {
        int stock = ui->productTreeWidget->topLevelItem(0)->text(3).toInt();
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
        int stock = ui->productTreeWidget->currentItem()->text(3).toInt();
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

void ShoppingManagerForm::on_searchPushButton_clicked()     // 검색 버튼 클릭시
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
                    QString("select * from orders where o_id = '%1'").arg(id));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 1: {
        int CID = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(
                    QString("select * from orders where o_c_id = '%1'").arg(CID));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 2: {
        int PID = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(
                    QString("select * from orders where o_p_id = '%1'").arg(PID));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 3: {
        QString date = ui->searchLineEdit->text();
        searchQuery->setQuery(
                    QString("select * from orders where o_date like '%%1%'").arg(date));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    }
}

void ShoppingManagerForm::on_clearPushButton_clicked()      // 클리어 버튼 클릭시
{
    ui->idLineEdit->clear();
    ui->clientNameLineEdit->clear();
    ui->pdNameLlineEdit->clear();
    ui->timeLlineEdit->clear();
    ui->orderLineEdit->clear();
    ui->totalLineEdit->clear();
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

    ui->clientTreeWidget->clear();
    ui->productTreeWidget->clear();
    emit clientIdSent(q->data(q->index(row, 1)).toInt());   // 고객 ID 전달
    emit productIdSent(q->data(q->index(row, 2)).toInt());  // 제품 ID 전달
}
