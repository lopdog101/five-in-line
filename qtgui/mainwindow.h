#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#ifndef Q_MOC_RUN
#  include "../algo/game.h"
#  include "../algo/wsplayer.h"
#  include "../extern/object_progress.hpp"
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

#ifndef Q_MOC_RUN
    Gomoku::field_ptr m_field;
    Gomoku::game_t game;
    Gomoku::steps_t redo_steps;
//    boost::signals::scoped_connection hld_step;

    ObjectProgress::logout_debug log_dbg;
    ObjectProgress::logout_file log_file;
#endif
};

#endif // MAINWINDOW_H
