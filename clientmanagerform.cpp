#include "clientmanagerform.h"
#include "ui_clientmanagerform.h"
#include "clientitem.h"

#include <QFile>
#include <QMenu>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQueryModel>

ClientManagerForm::ClientManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 640 << 480;
    ui->splitter->setSizes(sizes);
    ui->tableView->horizontalHeader()->resizeContentsPrecision();

    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    menu->addAction(removeAction);

    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));

    connect(ui->addPushButton, SIGNAL(clicked(bool)), this, SLOT(receiveAllClient()));

    if(!createConnection()) return;

    q = new QSqlQueryModel;
    searchQuery = new QSqlQueryModel;
    q->setQuery("select * from client order by c_id");
    q->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    q->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
    q->setHeaderData(2, Qt::Horizontal, QObject::tr("Gender"));
    q->setHeaderData(3, Qt::Horizontal, QObject::tr("Age"));
    q->setHeaderData(4, Qt::Horizontal, QObject::tr("Phone Number"));
    q->setHeaderData(5, Qt::Horizontal, QObject::tr("Address"));

    ui->tableView->setModel(q);
}

ClientManagerForm::~ClientManagerForm()
{
    delete ui;
    delete searchQuery;
}

bool ClientManagerForm::createConnection() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("Oracle11gx64");
    db.setUserName("th");
    db.setPassword("asdasd123");
    if (!db.open()) {
        qDebug() << db.lastError().text();
    }
    else {
        qDebug("success");
    }
    return true;
}

int ClientManagerForm::makeId()
{
    if(q->rowCount()==0) {
        return 1001;
    } else {
        int i = q->rowCount();
        auto id = q->data(q->index(i-1,0)).toInt();
        return ++id;
    }
}

void ClientManagerForm::removeItem()
{
    int row = ui->tableView->currentIndex().row();
    int item = q->data(q->index(row, 0)).toInt();
    q->setQuery(QString("delete from client where c_id = '%1'").arg(item));
    q->setQuery("select * from client order by c_id");
}

void ClientManagerForm::on_searchPushButton_clicked()
{
    int category = ui->searchComboBox->currentIndex();
    searchQuery->setQuery("select * from client");

    switch(category) {
    case 0:
    {
        int id = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(QString("select * from client where c_id = '%1'").arg(id));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 1:
    {
        QString name = ui->searchLineEdit->text();
        searchQuery->setQuery(QString("select * from client where c_name like '%%1%'").arg(name));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 2:
    {
        QString gender= ui->searchLineEdit->text();
        searchQuery->setQuery(QString("select * from client where c_gender like '%%1%'").arg(gender));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 3:
    {
        int age = ui->searchLineEdit->text().toInt();
        searchQuery->setQuery(QString("select * from client where c_age like '%%1%'").arg(age));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 4:
    {
        QString phone = ui->searchLineEdit->text();
        searchQuery->setQuery(QString("select * from client where c_phonenum like '%%1%'").arg(phone));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    case 5:
    {
        QString address = ui->searchLineEdit->text();
        searchQuery->setQuery(QString("select * from client where c_address like '%%1%'").arg(address));
        ui->searchTableView->setModel(searchQuery);
        break;
    }
    }
}

void ClientManagerForm::on_modifyPushButton_clicked()
{
    int row = ui->tableView->currentIndex().row();
    int id = q->data(q->index(row, 0)).toInt();
    int age; QString name, gender, phoneNum, adr;

    name = ui->nameLineEdit->text();
    gender = ui->genderLineEdit->text();
    age = ui->ageLineEdit->text().toInt();
    phoneNum = ui->phoneNumberLineEdit->text();
    adr = ui->addressLineEdit->text();

    q->setQuery(QString("update client set c_name ='%1', c_gender = '%2', c_age = '%3',"
                        "c_phoneNum = '%4', c_address = '%5' where c_id = '%6';")
                         .arg(name).arg(gender).arg(age).arg(phoneNum).arg(adr).arg(id));

    q->setQuery("select * from client order by c_id");
}

void ClientManagerForm::on_addPushButton_clicked()
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

    q->setQuery("select * from client order by c_id");
}
void ClientManagerForm::on_clearPushButton_clicked()
{
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

