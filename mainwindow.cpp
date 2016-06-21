
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
    QGraphicsScene *m_scene;
    QString imagePath = QFileDialog::getOpenFileName(
            this, tr("Open File"), QDir::rootPath(),
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png)"));

    m_image = new QImage();
    m_image->load(imagePath);\

    m_pixmap = QPixmap::fromImage(*m_image);

    if (! m_scene) {
        m_scene = new QGraphicsScene(this);
    }
    else {
        m_scene->clear();
    }

    m_scene->addPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());

    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
}

void MainWindow::on_sld_quality_valueChanged(int value) {

}

void MainWindow::on_sld_scale_valueChanged(int value) {

}
