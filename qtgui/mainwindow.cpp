#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include "../db/solution_tree_utils.h"


//#include <boost/filesystem/operations.hpp>
//#include <boost/filesystem/convenience.hpp>
//namespace fs=boost::filesystem;

#include "../algo/check_player.h"
#include "../algo/game_xml.h"
#include "../algo/env_variables.h"

BOOST_CLASS_EXPORT(Gomoku::check_player_t)
BOOST_CLASS_EXPORT(Gomoku::WsPlayer::wsplayer_t)
//BOOST_CLASS_EXPORT(Gomoku::ThreadPlayer)
//BOOST_CLASS_EXPORT(Gomoku::mfcPlayer)
//BOOST_CLASS_EXPORT(Gomoku::NullPlayer)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    field=Gomoku::field_ptr(new Gomoku::field_t);
    game.fieldp=field;

    ui->widget->field=field;
}

MainWindow::~MainWindow()
{
    delete ui;
}
