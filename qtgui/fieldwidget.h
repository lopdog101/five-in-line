#ifndef FIELDWIDGET_H
#define FIELDWIDGET_H

#include <QWidget>

class FieldWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FieldWidget(QWidget *parent = 0);
protected:
    void paintEvent(QPaintEvent *event);
signals:

public slots:

};

#endif // FIELDWIDGET_H
