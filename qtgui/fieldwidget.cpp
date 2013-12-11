#include "fieldwidget.h"

FieldWidget::FieldWidget(QWidget *parent) :
    QWidget(parent),center(0,0)
{
    show_move_numbers=true;

    bEmpty.load(":/Images/res/empty.bmp");
    bKrestik.load(":/Images/res/krestik.bmp");
    bNolik.load(":/Images/res/nolik.bmp");
    bKrestikLast.load(":/Images/res/krestik_last.bmp");
    bNolikLast.load(":/Images/res/nolik_last.bmp");
}

void FieldWidget::paintEvent(QPaintEvent *)
{
    if(!field)
        return;

    QPainter painter(this);


    QRect rc(0,0,width(),height());
    painter.fillRect(rc,Qt::white);

//    dc.SetTextColor(RGB(255,255,255));
//    dc.SetBkMode(TRANSPARENT);
//    CFont* def_font=dc.SelectObject(&m_font);

    const Gomoku::field_t& fl=*field;
    const Gomoku::steps_t& steps=fl.get_steps();

    for(int y=0;y<height();y+=box_size)
    for(int x=0;x<width();x+=box_size)
    {
        Gomoku::point pos=pix2world(QPoint(x,y));
        QPixmap* bmp;
        bool last=false;

        if(!fl.empty()&&pos==fl.back()||fl.size()>=2&&pos==steps[fl.size()-2])last=true;

        Gomoku::Step step=fl.at(pos);

        switch(step)
        {
        case Gomoku::st_krestik:bmp=last? &bKrestikLast:&bKrestik;break;
        case Gomoku::st_nolik:bmp=last? &bNolikLast:&bNolik;break;
        default:bmp=&bEmpty;
        }

        painter.drawPixmap(x,y,box_size,box_size,*bmp);

        if(show_move_numbers&&step!=Gomoku::st_empty)
        {
            Gomoku::steps_t::const_iterator it=std::find(steps.begin(),steps.end(),Gomoku::step_t(step,pos.x,pos.y));
            unsigned move_num=1+(it-steps.begin());

            QString smove_num=QString::number(move_num);
            painter.drawText(x,y,box_size,box_size,Qt::AlignCenter,smove_num);
        }
    }
//    on_after_paint(dc);
}

QPoint FieldWidget::world2pix(const Gomoku::point& pt) const
{
    QPoint ret((pt.x+center.x+width()/(box_size*2))*box_size,(pt.y+center.y+height()/(box_size*2))*box_size);
    return ret;
}

Gomoku::point FieldWidget::pix2world(const QPoint& pt) const
{
    Gomoku::point ret(pt.x()/box_size-center.x-width()/(box_size*2),pt.y()/box_size-center.y-height()/(box_size*2));
    return ret;
}
