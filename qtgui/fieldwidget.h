#ifndef FIELDWIDGET_H
#define FIELDWIDGET_H

#include <QWidget>
#include <QtCore>
#include <QtGui>

#ifndef Q_MOC_RUN
#  include "../algo/game.h"
#endif

class FieldWidget : public QWidget
{
    Q_OBJECT
public:
    static const int box_size=18;
    bool show_move_numbers;

#ifndef Q_MOC_RUN
    Gomoku::field_ptr field;
#endif

    explicit FieldWidget(QWidget *parent = 0);
private:
    QPixmap bEmpty;
    QPixmap bKrestik;
    QPixmap bNolik;
    QPixmap bKrestikLast;
    QPixmap bNolikLast;

#ifndef Q_MOC_RUN
    Gomoku::point center;
#endif

#ifndef Q_MOC_RUN
    QPoint world2pix(const Gomoku::point& pt) const;
    Gomoku::point pix2world(const QPoint& pt) const;
#endif

protected:
    void paintEvent(QPaintEvent *event);
Q_SIGNALS:

public Q_SLOTS:

};

#endif // FIELDWIDGET_H
