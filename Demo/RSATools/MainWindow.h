#ifndef ZEROMS_DEMO_RSATOOLS_MAINWINDOW_H
#define ZEROMS_DEMO_RSATOOLS_MAINWINDOW_H

#include <QtGui>
#include <QtWidgets>
#include "Base/Auth/RSA.h"

namespace ZeroMS {
namespace Demo {
namespace RSATools {

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_actionMakeKeyPair_triggered();
    void on_actionSavePriKeyToFile_triggered();
    void on_actionSavePubKeyToFile_triggered();
    void on_actionValiy_triggered();
    void on_actionReadPriKeyFromFile_triggered();
    void on_actionPriKeyInfo_triggered();
    void on_actionReadPubKeyFromFile_triggered();
    void on_actionPubKeyInfo_triggered();

    void on_actionGetSize_triggered();

    void on_actionPublicEncrypt_triggered();

    void on_actionPrivateDecrypt_triggered();

    void on_actionPrivateEncrypt_triggered();

    void on_actionPublicDecrypt_triggered();

private:
    Ui::MainWindow *ui;
    bool isHasKeyPair;

    ::ZeroMS::Base::Auth::RSAPrivateKey priKey;
    ::ZeroMS::Base::Auth::RSAPublicKey pubKey;
};

}}}   //namespace ZeroMS::Demo::RSATools

#endif // ZEROMS_DEMO_RSATOOLS_MAINWINDOW_H
