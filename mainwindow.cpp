#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QGraphicsScene"
#include "QFileDialog"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
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
    QGraphicsScene *scene;
    QString imagePath = QFileDialog::getOpenFileName(
            this,
            tr("Open File"),
                "/",
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png)" )
    );

    imageObject = new QImage();
    imageObject->load(imagePath);

    image = QPixmap::fromImage(*imageObject);

    scene = new QGraphicsScene(this);
    scene->addPixmap(image);
    scene->setSceneRect(image.rect());
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
}

void MainWindow::on_sld_quality_valueChanged(int value) {

}

void MainWindow::on_sld_scale_valueChanged(int value) {

}
