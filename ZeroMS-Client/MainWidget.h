#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QMainWindow>

namespace Ui {
class MainWidget;
}

class MainWidget : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWidget();
    ~MainWidget();
    
private:
    Ui::MainWidget *ui;
};

#endif // MAINWIDGET_H