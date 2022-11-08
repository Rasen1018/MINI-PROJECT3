#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ClientManagerForm;
class ProductManagerForm;
class ShoppingManagerForm;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionClient_triggered();
    void on_actionProduct_triggered();
    void on_actionShopping_triggered();

private:
    Ui::MainWindow *ui;
    ClientManagerForm *clientForm;
    ProductManagerForm *productForm;
    ShoppingManagerForm *shoppingForm;
};
#endif // MAINWINDOW_H
