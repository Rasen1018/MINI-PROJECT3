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

    // TreeView 헤더 스타일 설정
    setHeaderStyle();

    ui->tableView->setModel(q);     // Viewer에 Model 표시
}

ClientManagerForm::~ClientManagerForm()
{
    delete ui;
    delete searchQuery;
}

/***********************************************************************************************/
int ClientManagerForm::makeId( )    // 고객 ID 생성 함수
{
    if(q->rowCount()==0) {      // DB에 데이터가 없을 경우 ID 1001부터 시작
        return 1001;
    } else {                                            // 데이터가 있을 경우
        int i = q->rowCount();
        auto id = q->data(q->index(i-1,0)).toInt();     // DB 데이터의 마지막 ID를 가져와서 ++id
        return ++id;
    }
}


//___________고객 정보 삭제____________//
void ClientManagerForm::removeItem()
{
    int row = ui->tableView->currentIndex().row();
    int item = q->data(q->index(row, 0)).toInt();       // Query Model에서 삭제하려는 고객의 row를 통해 id를 가져옴
    /* delect query문 */
    q->setQuery(QString("delete from client where c_id = '%1'").arg(item));     // id를 argment로 받음
    q->setQuery("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                "from client order by c_id");
}


//___________채팅 서버에 고객 리스트 전달____________//
void ClientManagerForm::sendClientList() {
    emit updateList();
    for(int i=0; i< q->rowCount(); i++) {       // DB 끝까지 for문 반복
        QString id="";
        QString name="";
        id = q->data(q->index(i, 0)).toString();    // DB에서 순차적으로 데이터 가져옴
        name = q->data(q->index(i,1)).toString();
        QStringList list;
        list << id << name;         // id, 이름 전달
        emit getAllClient(list);
    }
}

//___________shoppingManagerForm에 StandardItem 전달____________//
void ClientManagerForm::receiveData(QString name) {

    QModelIndexList indexes =
            q->match(q->index(0, 1), Qt::EditRole, name, -1, Qt::MatchFlags(Qt::MatchContains));
          //query model에서 q->index(0, 1)의 데이터와 name을 비교해서 데이터의 내용이 name을 포함한다면[flags(Qt::MatchContains)]
          //해당하는 index를 전부 리스트에 저장

    foreach(auto idx, indexes) {        // DB의 모든 데이터 가져오기
        int id = q->data(idx.siblingAtColumn(0)).toInt();
        QString name = q->data(idx.siblingAtColumn(1)).toString();
        QString gender = q->data(idx.siblingAtColumn(2)).toString();
        int age = q->data(idx.siblingAtColumn(3)).toInt();
        QString phoneNum = q->data(idx.siblingAtColumn(4)).toString();
        QString address = q->data(idx.siblingAtColumn(5)).toString();

        QStringList strings;        // stringList에 저장
        strings << QString::number(id) << name << gender
                << QString::number(age) << phoneNum << address;

        QList<QStandardItem *> item;        // stringList->item으로 저장
        for (int i = 0; i < 6; ++i) {
            item.append(new QStandardItem(strings.at(i)));
        }
        emit clientItemSent(item);      // QList<QStandardItem *> item 전달
    }
}


//___________shoppingManagerForm에 StandardItem 전달____________//
void ClientManagerForm::receiveData(int id) {

    QModelIndexList indexes =
            q->match(q->index(0, 0), Qt::EditRole, id, 1, Qt::MatchFlags(Qt::MatchCaseSensitive));
          //query model에서 q->index(0, 0)의 데이터와 id를 비교해서 데이터의 내용이 id와 일치한다면[flags(Qt::MatchCaseSensitive)]
          //해당하는 index를 전부 리스트에 저장

    foreach(auto idx, indexes) {        // DB의 모든 데이터 가져오기
        int id = q->data(idx.siblingAtColumn(0)).toInt();
        QString name = q->data(idx.siblingAtColumn(1)).toString();
        QString gender = q->data(idx.siblingAtColumn(2)).toString();
        int age = q->data(idx.siblingAtColumn(3)).toInt();
        QString phoneNum = q->data(idx.siblingAtColumn(4)).toString();
        QString address = q->data(idx.siblingAtColumn(5)).toString();

        QStringList strings;        // stringList에 저장
        strings << QString::number(id) << name << gender
                << QString::number(age) << phoneNum << address;

        QList<QStandardItem *> item;        // stringList->item으로 저장
        for (int i = 0; i < 6; ++i) {
            item.append(new QStandardItem(strings.at(i)));
        }
        emit clientItemSent(item);      // QList<QStandardItem *> item 전달
    }
}


/***********************************************************************************************/
void ClientManagerForm::on_addPushButton_clicked()  // 추가 버튼 사용자 슬롯
{
    int id = makeId();
    QString name = ui->nameLineEdit->text();
    QString gender = ui->genderLineEdit->text();
    int age = ui->ageLineEdit->text().toInt();
    QString phoneNum = ui->phoneNumberLineEdit->text();
    QString adr = ui->addressLineEdit->text();

    // ORACLE SQL PROCEDURE 실행
    q->setQuery(QString("call add_client('%1', '%2', '%3', '%4', '%5', '%6');")
            .arg(id).arg(name).arg(gender).arg(age).arg(phoneNum).arg(adr));

    // ORDER BY로 정렬
    q->setQuery("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                "from client order by c_id");
}


//___________수정 버튼 사용자 슬롯____________//
void ClientManagerForm::on_modifyPushButton_clicked()
{
    int id = ui->idLineEdit->text().toInt();
    QString name = ui->nameLineEdit->text();
    QString gender = ui->genderLineEdit->text();
    int age = ui->ageLineEdit->text().toInt();
    QString phoneNum = ui->phoneNumberLineEdit->text();
    QString adr = ui->addressLineEdit->text();

    // UPDATE QUERY 문
    q->setQuery(QString("update client set c_name ='%1', c_gender = '%2', c_age = '%3',"
                        "c_phoneNum = '%4', c_address = '%5' where c_id = '%6';")
                         .arg(name).arg(gender).arg(age).arg(phoneNum).arg(adr).arg(id));
    // ORDER BY로 정렬
    q->setQuery("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                "from client order by c_id");
}



//___________검색 버튼 사용자 슬롯____________//
void ClientManagerForm::on_searchPushButton_clicked()
{
    if(ui->searchLineEdit->text() == nullptr) return;
    int category = ui->searchComboBox->currentIndex();      // comboBox 별로 항목을 다르게 검색

    // 검색을 위한 QUERYMODEL에 SELECT QUERY 문
    searchQuery->setQuery("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                 "from client order by c_id");
    // header 설정
    searchQuery->setHeaderData(0, Qt::Horizontal, tr("ID"));
    searchQuery->setHeaderData(1, Qt::Horizontal, tr("Name"));
    searchQuery->setHeaderData(2, Qt::Horizontal, tr("Gender"));
    searchQuery->setHeaderData(3, Qt::Horizontal, tr("Age"));
    searchQuery->setHeaderData(4, Qt::Horizontal, tr("Phone Number"));
    searchQuery->setHeaderData(5, Qt::Horizontal, tr("Address"));

    switch(category) {
    case 0: {   // 아이디 검색
        int id = ui->searchLineEdit->text().toInt();

        // 검색을 위한 WHERE 문
        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                      "from client where c_id = '%1'").arg(id));
        ui->searchTableView->setModel(searchQuery);
        break;}

    case 1: {   // 이름 검색
        QString name = ui->searchLineEdit->text();

        // 검색을 위한 WHERE 문
        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                      "from client where c_name like '%%1%'").arg(name));
        ui->searchTableView->setModel(searchQuery);
        break;}

    case 2: {   // 성별 검색
        QString gender= ui->searchLineEdit->text();

        // 검색을 위한 WHERE LIKE 문
        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                      "from client where c_gender like '%%1%'").arg(gender));
        ui->searchTableView->setModel(searchQuery);
        break;}

    case 3: {   // 나이 검색
        int age = ui->searchLineEdit->text().toInt();

        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                      "from client where c_age like '%%1%'").arg(age));
        ui->searchTableView->setModel(searchQuery);
        break;}

    case 4: {   // 전화번호 검색
        QString phone = ui->searchLineEdit->text();

        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                            "from client where c_phonenum like '%%1%'").arg(phone));
        ui->searchTableView->setModel(searchQuery);
        break;}

    case 5: {   // 주소 검색
        QString address = ui->searchLineEdit->text();

        searchQuery->setQuery(
                    QString("select c_id, C_NAME , C_GENDER , C_AGE , C_PHONENUM, C_ADDRESS "
                                      "from client where c_address like '%%1%'").arg(address));
        ui->searchTableView->setModel(searchQuery);
        break;}
    }
}


//___________클리어 버튼 사용자 정의 슬롯____________//
void ClientManagerForm::on_clearPushButton_clicked()
{
    // 입력창 내용 클리어
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->genderLineEdit->clear();
    ui->ageLineEdit->clear();
    ui->phoneNumberLineEdit->clear();
    ui->addressLineEdit->clear();
}


/***********************************************************************************************/
void ClientManagerForm::setHeaderStyle() {
    q->setHeaderData(0, Qt::Horizontal, tr("ID"));
    q->setHeaderData(1, Qt::Horizontal, tr("Name"));
    q->setHeaderData(2, Qt::Horizontal, tr("Gender"));
    q->setHeaderData(3, Qt::Horizontal, tr("Age"));
    q->setHeaderData(4, Qt::Horizontal, tr("Phone Number"));
    q->setHeaderData(5, Qt::Horizontal, tr("Address"));

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);         // 데이터의 크기에 따라 헤더 사이즈 설정
    ui->searchTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setStyleSheet("QHeaderView {font-weight: bold; color : sandybrown}");        // 헤더 폰트 설정
    ui->searchTableView->horizontalHeader()->setStyleSheet("QHeaderView {font-weight: bold; color : sandybrown}");
}


//___________tableView 우클릭 슬롯____________//
void ClientManagerForm::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
    if(ui->tableView->indexAt(pos).isValid())           // viewer에 데이터가 있을 때만 메뉴 표시
            menu->exec(globalPos);
}


//___________tableView 클릭시 슬롯____________//
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
