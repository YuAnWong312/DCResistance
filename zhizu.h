#ifndef ZHIZU_H
#define ZHIZU_H

#include <QDialog>

namespace Ui {
class Zhizu;
}

class Zhizu : public QDialog
{
    Q_OBJECT
    
public:
    explicit Zhizu(QWidget *parent = 0);
    ~Zhizu();
    
private:
    Ui::Zhizu *ui;
};

#endif // ZHIZU_H
