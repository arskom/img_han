
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QImage>
#include <QLabel>
#include <QDebug>
#include <QBuffer>
#include <QScrollBar>
#include <QFileDialog>
#include <QImageReader>
#include <QGraphicsView>
#include <QDesktopWidget>
#include <QGraphicsScene>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_scene(nullptr),
    m_image(nullptr),
    m_orig_size(0),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *e) {
    QMainWindow::showEvent(e);

    const auto &desktop_rect = QApplication::desktop()->screen()->rect();

    this->resize(desktop_rect.width() * .75, desktop_rect.height() * .75);
    this->move(desktop_rect.center() - this->rect().center());
}

void MainWindow::on_openButton_clicked() {

    QString imagePath = QFileDialog::getOpenFileName(
            this, tr("Open File"), /*QDir::rootPath()*/ "/home/arda/Masaüstü",
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp);;WEBP (*.webp)"));

    m_image = new QImage();
    m_image->load(imagePath);

    m_orig_size = QFileInfo(imagePath).size();

    ui->lbl_width->setText(QString::number(m_image->width()));
    ui->lbl_height->setText(QString::number(m_image->height()));

    m_pixmap = QPixmap::fromImage(*m_image);
    show_pixmap();

    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
}


void MainWindow::on_saveButton_clicked()
{
    QString imagePath = QFileDialog::getSaveFileName(
            this,tr("Save File"),/*QDir::rootPath()*/ "/home/arda/Masaüstü",
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp);;WEBP (*.webp)"));

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




void MainWindow::on_sld_quality_valueChanged(int value)
{

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    m_image->save(&buffer,"WEBP",value);

    auto l_size_b = buffer.size();
    double l_size_kb = buffer.size()/1024.00;
    ui->lbl_size->setText(QString::number(l_size_kb));

    QImage image;
    image.loadFromData(ba);
    m_pixmap = QPixmap::fromImage(image);

    show_pixmap();

    ui->lbl_quality->setText(QString::number(value));

    double comp_p = 100.0 * l_size_b / m_orig_size;

    if(comp_p>100)
    {
        ui->lbl_compression->setText(QString::number(comp_p));
        QLabel* m_label = ui->lbl_size;
        m_label->setStyleSheet("QLabel { background-color : red; color : black; }");
    }
    else if(comp_p<=100)
    {
        ui->lbl_compression->setText(QString::number(comp_p));
        QLabel* m_label = ui->lbl_size;
        m_label->setStyleSheet("QLabel { background-color : rgba(0,0,0,0%); color : black; }");

    }
}


void MainWindow::on_sld_scale_valueChanged(int value) {

    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    int w = m_image->width();
    int h = m_image->height();
    int new_w = (w * value)/100;
    int new_h = (h * value)/100;

    ui->lbl_width->setText(QString::number(new_w));
    ui->lbl_height->setText(QString::number(new_h));

    m_pixmap = QPixmap::fromImage(
                m_image->scaled(new_w, new_h, Qt::KeepAspectRatio, Qt::FastTransformation));

    show_pixmap();
    ui->lbl_scale->setText(QString::number(value));

 //   ui->graphicsView->scale(value/100.0,value/100.0);
}


