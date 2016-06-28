#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QGraphicsScene>
#include <QImage>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QDebug>
#include <QTextEdit>
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QGraphicsAnchor>
#include <QGraphicsView>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_scene(nullptr),
    m_image(nullptr),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openButton_clicked()
{
    QString imagePath = QFileDialog::getOpenFileName(
            this, tr("Open File"), /*QDir::rootPath()*/ "/home/arda/Masaüstü",
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp)"));

    m_image = new QImage();
    m_image->load(imagePath);\

    m_pixmap = QPixmap::fromImage(*m_image);
    show_pixmap();

    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

}


void MainWindow::on_saveButton_clicked()
{
    QString imagePath = QFileDialog::getSaveFileName(
            this,tr("Save File"),/*QDir::rootPath()*/ "/home/arda/Masaüstü",
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp)"));

    *m_image = m_pixmap.toImage();
     m_image->save(imagePath);
}


void MainWindow::show_pixmap()
{
    if (! m_scene) {
        m_scene = new QGraphicsScene(this);
    }
    else {
        m_scene->clear();
    }

    m_scene->addPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());
}


void MainWindow::on_sld_quality_valueChanged(int value) {

}


void MainWindow::on_sld_scale_valueChanged(int value) {

    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    int w = m_image->width();
    int h = m_image->height();

    int new_w = (w * value)/100;
    int new_h = (h * value)/100;

    m_pixmap = QPixmap::fromImage(
                m_image->scaled(new_w, new_h, Qt::KeepAspectRatio, Qt::FastTransformation));


    if(value > 80)
    ui->graphicsView->scale(value/100.0,value/100.0);
    else if(value < 80)
    ui->graphicsView->scale(1/(value/100.0),1/(value/100.0));



}

