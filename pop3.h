#ifndef POP3_H
#define POP3_H
#include "Client.h"

#include <QMainWindow>

namespace Ui {
    class Pop3;
}

class Pop3 : public QMainWindow
{
    Q_OBJECT

public:
    explicit Pop3(QWidget *parent = 0);
    ~Pop3();
public slots:
    void MyEventHandler();
    void Authorize();
signals:
    void MySignal();
private:
    Ui::Pop3 *ui;
    Pop3Client *client;
};

#endif // POP3_H
