#include "clientmanagerform.h"
#include "ui_clientmanagerform.h"

#include <QMenu>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QTreeWidgetItem>
#include <QStandardItemModel>

ClientManagerForm::ClientManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 640 << 480;
    ui->splitter->setSizes(sizes);

    /*우클릭으로 고객 정보 삭제*/
    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));
    menu = new QMenu;
    menu->addAction(removeAction);

    connect(ui->searchLineEdit, SIGNAL(returnPressed()),    // 검색할 때 엔터 누르면 검색 버튼 클릭
            this, SLOT(on_searchPushButton_clicked()));

    // 고객정보를 추가, 변경, 삭제하면 ChatServer로 고객 리스트 전달
    connect(ui->addPushButton, SIGNAL(clicked(bool)), this, SLOT(sendClientList()));
    connect(ui->modifyPushButton, SIGNAL(clicked(bool)), this, SLOT(sendClientList()));
    connect(removeAction, SIGNAL(triggered()), SLOT(sendClientList()));

    // Model 생성
    q = new QSqlQueryModel;
    searchQuery = new QSqlQueryModel;           // 검색을 위한 QueryModel
    q->setQuery("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                "from client order by c_id");   //  CLIENT 정보 SELECT문
    // QueryModel 헤더 이름 설정
    q->setHeaderData(0, Qt::Horizontal, tr("ID"));
    q->setHeaderData(1, Qt::Horizontal, tr("Name"));
    q->setHeaderData(2, Qt::Horizontal, tr("Gender"));
    q->setHeaderData(3, Qt::Horizontal, tr("Age"));
    q->setHeaderData(4, Qt::Horizontal, tr("Phone Number"));
    q->setHeaderData(5, Qt::Horizontal, tr("Address"));

    ui->tableView->setModel(q);     // Viewer에 Model 표시
}

ClientManagerForm::~ClientManagerForm()
{
    delete ui;
    delete searchQuery;
}

int ClientManagerForm::makeId( )    // 고객 ID 생성 함수
{
    if(q->rowCount()==0) {
        return 1001;
    } else {
        int i = q->rowCount();
        auto id = q->data(q->index(i-1,0)).toInt();
        return ++id;
    }
}

void ClientManagerForm::removeItem()    // TreeWidget에 있는 고객 리스트 삭제
{
    int row = ui->tableView->currentIndex().row();
    int item = q->data(q->index(row, 0)).toInt();
    q->setQuery(QString("delete from client where c_id = '%1'").arg(item));
    q->setQuery("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                "from client order by c_id");
}

void ClientManagerForm::sendClientList() {
    emit updateList();
    for(int i=0; i< q->rowCount(); i++) {
        QString id="";
        QString name="";
        id = q->data(q->index(i, 0)).toString();
        name = q->data(q->index(i,1)).toString();
        QStringList list;
        list << id << name;
        emit getAllClient(list);
    }
}

void ClientManagerForm::receiveData(QString name) {
#if 0
    QSqlQueryModel query;
    query.setQuery(QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS from client "
                   "where c_name like '%%1%' order by c_id").arg(name));

    for(int i=0;i<query.rowCount();i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        for(int j = 0; j<6; j++) {
            item->setText(j, query.data(query.index(i, j)).toString());
        }
        emit clientItemSent(item);
    }
#else
    QModelIndexList indexes =
            q->match(q->index(0, 1), Qt::EditRole, name, -1, Qt::MatchFlags(Qt::MatchContains));
    qDebug() << indexes;

//    foreach(auto ix, indexes) {
//        int id = clientModel->data(ix.siblingAtColumn(0)).toInt(); //c->id();
//        QString name = clientModel->data(ix.siblingAtColumn(1)).toString();
//        QString number = clientModel->data(ix.siblingAtColumn(2)).toString();
//        QString address = clientModel->data(ix.siblingAtColumn(3)).toString();
//        QStringList strings;
//        strings << QString::number(id) << name << number << address;

//        QList<QStandardItem *> items;
//        for (int i = 0; i < 4; ++i) {
//            items.append(new QStandardItem(strings.at(i)));
//        }

//        searchModel->appendRow(items);
//        ui->searchTableView->resizeColumnsToContents();
//    }
#endif
}

void ClientManagerForm::receiveData(int id) {

    QSqlQueryModel query;
    query.setQuery(QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS from client "
                   "where c_id = '%1'").arg(id));

    QTreeWidgetItem *item = new QTreeWidgetItem;
    for(int i=0;i<6;i++) {
        item->setText(i, query.data(query.index(0, i)).toString());
    }
    emit clientItemSent(item);
}

void ClientManagerForm::on_addPushButton_clicked()  // 추가 버튼 사용자 슬롯
{
    int id = makeId();
    int age;
    QString name, gender, phoneNum, adr;
    name = ui->nameLineEdit->text();
    gender = ui->genderLineEdit->text();
    age = ui->ageLineEdit->text().toInt();
    phoneNum = ui->phoneNumberLineEdit->text();
    adr = ui->addressLineEdit->text();
    q->setQuery(QString("call add_client('%1', '%2', '%3', '%4', '%5', '%6');")
            .arg(id).arg(name).arg(gender).arg(age).arg(phoneNum).arg(adr));

    q->setQuery("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                "from client order by c_id");
}

void ClientManagerForm::on_modifyPushButton_clicked()   // 수정 버튼 사용자 슬롯
{
    int id = ui->idLineEdit->text().toInt();
    int age; QString name, gender, phoneNum, adr;

    name = ui->nameLineEdit->text();
    gender = ui->genderLineEdit->text();
    age = ui->ageLineEdit->text().toInt();
    phoneNum = ui->phoneNumberLineEdit->text();
    adr = ui->addressLineEdit->text();

    q->setQuery(QString("update client set c_name ='%1', c_gender = '%2', c_age = '%3',"
                        "c_phoneNum = '%4', c_address = '%5' where c_id = '%6';")
                         .arg(name).arg(gender).arg(age).arg(phoneNum).arg(adr).arg(id));

    q->setQuery("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                "from client order by c_id");
}

void ClientManagerForm::on_searchPushButton_clicked()   // 검색 버튼 사용자 슬롯
{
    if(ui->searchLineEdit->text() == nullptr) return;
    int category = ui->searchComboBox->currentIndex();
    searchQuery->setQuery("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                 "from client order by c_id");
    searchQuery->setHeaderData(0, Qt::Horizontal, tr("ID"));
    searchQuery->setHeaderData(1, Qt::Horizontal, tr("Name"));
    searchQuery->setHeaderData(2, Qt::Horizontal, tr("Gender"));
    searchQuery->setHeaderData(3, Qt::Horizontal, tr("Age"));
    searchQuery->setHeaderData(4, Qt::Horizontal, tr("Phone Number"));
    searchQuery->setHeaderData(5, Qt::Horizontal, tr("Address"));

    switch(category) {
    case 0: {
        int id = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                      "from client where c_id = '%1'").arg(id));
        ui->searchTableView->setModel(searchQuery);
        break;}
    case 1: {
        QString name = ui->searchLineEdit->text();
        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                      "from client where c_name like '%%1%'").arg(name));
        ui->searchTableView->setModel(searchQuery);
        break;}
    case 2: {
        QString gender= ui->searchLineEdit->text();
        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                      "from client where c_gender like '%%1%'").arg(gender));
        ui->searchTableView->setModel(searchQuery);
        break;}
    case 3: {
        int age = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                      "from client where c_age like '%%1%'").arg(age));
        ui->searchTableView->setModel(searchQuery);
        break;}
    case 4: {
        QString phone = ui->searchLineEdit->text();
        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                            "from client where c_phonenum like '%%1%'").arg(phone));
        ui->searchTableView->setModel(searchQuery);
        break;}
    case 5: {
        QString address = ui->searchLineEdit->text();
        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                      "from client where c_address like '%%1%'").arg(address));
        ui->searchTableView->setModel(searchQuery);
        break;}
    }
}

void ClientManagerForm::on_clearPushButton_clicked()    // 클리어 버튼 사용자 정의 슬롯
{
    // 입력창 내용 클리어
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->genderLineEdit->clear();
    ui->ageLineEdit->clear();
    ui->phoneNumberLineEdit->clear();
    ui->addressLineEdit->clear();
}

void ClientManagerForm::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
    if(ui->tableView->indexAt(pos).isValid())
            menu->exec(globalPos);
}

void ClientManagerForm::on_tableView_clicked(const QModelIndex &index)
{
    int row = index.row();
    ui->idLineEdit->setText(q->data(q->index(row, 0)).toString());
    ui->nameLineEdit->setText(q->data(q->index(row, 1)).toString());
    ui->genderLineEdit->setText(q->data(q->index(row, 2)).toString());
    ui->ageLineEdit->setText(q->data(q->index(row, 3)).toString());
    ui->phoneNumberLineEdit->setText(q->data(q->index(row, 4)).toString());
    ui->addressLineEdit->setText(q->data(q->index(row, 5)).toString());
}
