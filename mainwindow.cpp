#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clientmanagerform.h"
#include "productmanagerform.h"
#include "shoppingmanagerform.h"
#include "chatserverform.h"
#include <QSqlError>
#include <QSqlDatabase>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    if(!createConnection()) return;

    clientForm = new ClientManagerForm(this);           // 고객 인포 생성
    clientForm->setWindowTitle(tr("Client Info"));

    productForm = new ProductManagerForm(this);         // 제품 인포 생성
    productForm->setWindowTitle(tr("Product Info"));

    shoppingForm = new ShoppingManagerForm(this);       // 쇼핑 인포 생성
    shoppingForm->setWindowTitle(tr("Shopping Info"));

    QMdiSubWindow *mdi = ui->mdiArea->addSubWindow(clientForm);     // 메인 윈도우에 위젯 추가
    ui->mdiArea->addSubWindow(productForm);
    ui->mdiArea->addSubWindow(shoppingForm);
    ui->mdiArea->setActiveSubWindow(mdi);
#if 0
    // ShoppingForm에서 고객 이름을 전달해주면 ClientForm에서 그 이름을 받아 item을 검색하는 슬롯 연결
    connect(shoppingForm, SIGNAL(clientDataSent(QString)), clientForm, SLOT(receiveData(QString)));
    // ShoppingForm에서 고객 ID를 전달해주면 ClientForm에서 그 ID를 받아 item을 검색하는 슬롯 연결
    connect(shoppingForm, SIGNAL(clientIdSent(int)), clientForm, SLOT(receiveData(int)));
    // ClientForm에서 위에서 전달받은 고객 ID, 이름을 검색해 일치하는 item을 ShoppingForm에 가져오는 슬롯 연결
    connect(clientForm, SIGNAL(clientDataSent(ClientItem*)), shoppingForm, SLOT(receiveData(ClientItem*)));

    // ShoppingForm에서 제품 이름을 전달해주면 ProductForm에서 그 이름을 받아 item을 검색하는 슬롯 연결
    connect(shoppingForm, SIGNAL(dataSent(QString)), productForm, SLOT(receiveName(QString)));
    // ShoppingForm에서 제품 ID를 전달해주면 ProductForm에서 그 ID를 받아 item을 검색하는 슬롯 연결
    connect(shoppingForm, SIGNAL(productIdSent(int)), productForm, SLOT(receiveName(int)));
    // ShoppingForm에서 제품 품목을 전달해주면 ProductForm에서 그 품목을 받아 item을 검색하는 슬롯 연결
    connect(shoppingForm, SIGNAL(categoryDataSent(QString)), productForm, SLOT(receiveCategory(QString)));
    // ProductForm에서 위에서 전달받은 제품 ID, 이름, 품목을 검색해 일치하는 item을 ShoppingForm에 가져오는 슬롯 연결
    connect(productForm, SIGNAL(productItemSent(QTreeWidgetItem*)), shoppingForm, SLOT(shopReceiveData(ProductItem*)));
#endif
    // ShoppingForm에서 고객 이름을 전달해주면 ClientForm에서 그 이름을 받아 item을 검색하는 슬롯 연결
    connect(shoppingForm, SIGNAL(clientDataSent(QString)), clientForm, SLOT(receiveData(QString)));
    // ShoppingForm에서 고객 ID를 전달해주면 ClientForm에서 그 ID를 받아 item을 검색하는 슬롯 연결
    connect(shoppingForm, SIGNAL(clientIdSent(int)), clientForm, SLOT(receiveData(int)));
    // ClientForm에서 위에서 전달받은 고객 ID, 이름을 검색해 일치하는 item을 ShoppingForm에 가져오는 슬롯 연결
    connect(clientForm, SIGNAL(clientItemSent(QTreeWidgetItem*)), shoppingForm, SLOT(receiveData(QTreeWidgetItem*)));

    // ShoppingForm에서 제품 이름을 전달해주면 ProductForm에서 그 이름을 받아 item을 검색하는 슬롯 연결
    connect(shoppingForm, SIGNAL(dataSent(QString)), productForm, SLOT(nameReceived(QString)));
    // ShoppingForm에서 제품 ID를 전달해주면 ProductForm에서 그 ID를 받아 item을 검색하는 슬롯 연결
    connect(shoppingForm, SIGNAL(productIdSent(int)), productForm, SLOT(idReceived(int)));
    // ShoppingForm에서 제품 품목을 전달해주면 ProductForm에서 그 품목을 받아 item을 검색하는 슬롯 연결
    connect(shoppingForm, SIGNAL(categoryDataSent(QString)), productForm, SLOT(categoryReceived(QString)));
    // ProductForm에서 위에서 전달받은 제품 ID, 이름, 품목을 검색해 일치하는 item을 ShoppingForm에 가져오는 슬롯 연결
    connect(productForm, SIGNAL(productItemSent(QTreeWidgetItem*)), shoppingForm, SLOT(shopReceiveData(QTreeWidgetItem*)));

    // ShoppingForm에서 주문하면 제품 ID와 주문량을 ProductForm에 전달해 재고를 변경시키는 슬롯 연결
    connect(shoppingForm, SIGNAL(inventorySent(int, int)), productForm, SLOT(inventoryChanged(int, int)));

    // ChatServerForm을 Singleton으로 생성
    ChatServerForm& chatServer = ChatServerForm::getIncetance();
    connect(ui->actionChat, &QAction::triggered, this, [&](){
        chatServer.show();
    });

    // ChatServerForm이 생성되면 ClientForm에 시그널을 주는 슬롯 연결
    connect(&chatServer, SIGNAL(callClientForm()), clientForm, SLOT(sendClientList()));
    // ClientForm에서 ChatServerForm으로 전체 리스트를 받아오는 슬롯 연결
    connect(clientForm, SIGNAL(getAllClient(QStringList)), &chatServer, SLOT(getAllClient(QStringList)));
    connect(clientForm, SIGNAL(updateList()), &chatServer, SLOT(widgetUpdate()));
    // signal callClientForm()을 부르는 함수
    chatServer.openWidget();

    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete clientForm;
    delete productForm;
    delete shoppingForm;
}

bool MainWindow::createConnection() {
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

void MainWindow::on_actionClient_triggered()
{
    if(clientForm != nullptr) {
        clientForm->setFocus();
    }
}

void MainWindow::on_actionProduct_triggered()
{
    if(productForm != nullptr) {
        productForm->setFocus();
    }
}

void MainWindow::on_actionShopping_triggered()
{
    if(shoppingForm != nullptr) {
        shoppingForm->setFocus();
    }
}
