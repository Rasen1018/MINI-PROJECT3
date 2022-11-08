#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clientmanagerform.h"
#include "productmanagerform.h"
#include "shoppingmanagerform.h"
#include "chatserverform.h"
#include "chatclientform.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    ClientManagerForm *clientForm1 = new ClientManagerForm(0);
//    clientForm1->show();
    clientForm = new ClientManagerForm(this);
    clientForm->setWindowTitle(tr("Client Info"));
    connect(clientForm, SIGNAL(destroyed()),
            clientForm, SLOT(deleteLater()));
//    ui->tabWidget->addTab(clientForm, "&Client Info");

    productForm = new ProductManagerForm(this);
    productForm->setWindowTitle(tr("Product Info"));

    shoppingForm = new ShoppingManagerForm(this);
    shoppingForm->setWindowTitle(tr("Shopping Info"));

    connect(shoppingForm, SIGNAL(clientDataSent(QString)), clientForm, SLOT(receiveData(QString)));
    connect(shoppingForm, SIGNAL(clientIdSent(int)), clientForm, SLOT(receiveData(int)));

    connect(shoppingForm, SIGNAL(dataSent(QString)), productForm, SLOT(receiveName(QString)));
    connect(shoppingForm, SIGNAL(productIdSent(int)), productForm, SLOT(receiveName(int)));
    connect(shoppingForm, SIGNAL(categoryDataSent(QString)), productForm, SLOT(receiveCategory(QString)));
    connect(productForm, SIGNAL(producdataSent(ProductItem*)), shoppingForm, SLOT(shopReceiveData(ProductItem*)));

    connect(shoppingForm, SIGNAL(inventorySent(int, int)), productForm, SLOT(inventoryChanged(int, int)));

    ChatServerForm& chatServer = ChatServerForm::getIncetance();
    connect(ui->actionChat, &QAction::triggered, this, [&](){
        chatServer.show();
    });

    connect(&chatServer, SIGNAL(callClientForm()), clientForm, SLOT(receiveAllClient()));
    connect(clientForm, SIGNAL(getAllClient(QStringList)), &chatServer, SLOT(getAllClient(QStringList)));
    chatServer.openWidget();

    ChatClientForm* chatClient = new ChatClientForm(0);
    chatClient->show();
    connect(chatClient, SIGNAL(destroyed()),
            chatClient, SLOT(deleteLater()));

    QMdiSubWindow *cw = ui->mdiArea->addSubWindow(clientForm);
    ui->mdiArea->addSubWindow(productForm);
    ui->mdiArea->addSubWindow(shoppingForm);
    ui->mdiArea->setActiveSubWindow(cw);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete clientForm;
    delete productForm;
    delete shoppingForm;
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
