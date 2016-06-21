
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QGraphicsScene>


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
            this, tr("Open File"), QDir::rootPath(),
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png)"));

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
            this,tr("Save File"),QDir::rootPath(),
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png)" ));

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

    int w = m_image->width();
    int h = m_image->height();

    int new_w = (w * value)/100;
    int new_h = (h * value)/100;

    m_pixmap = QPixmap::fromImage(
                m_image->scaled(new_w, new_h, Qt::KeepAspectRatio, Qt::FastTransformation));

    show_pixmap();
}
