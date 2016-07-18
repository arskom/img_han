
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wheeledgraphicsview.h"

#include <QtConcurrent/QtConcurrentRun>

#include <mutex>
#include <string>
#include <iostream>

#include <QEvent>
#include <QImage>
#include <QLabel>
#include <QDebug>
#include <QMatrix>
#include <QBuffer>
#include <QShortcut>
#include <QScrollBar>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QImageReader>
#include <QGraphicsView>
#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QStandardPaths>


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    m_scene(nullptr),
    m_image(nullptr),
    m_orig_size(0),
    m_processing(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    new QShortcut(Qt::CTRL | Qt::Key_Q, this, SLOT(close()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::showEvent(QShowEvent *e) {
    QMainWindow::showEvent(e);

    const auto &desktop_rect = QApplication::desktop()->screen()->rect();

    this->resize(desktop_rect.width() * .75, desktop_rect.height() * .75);
    this->move(desktop_rect.center() - this->rect().center());
}

void MainWindow::on_btn_open_clicked() {
    const auto &desktop_abs = QStandardPaths::standardLocations(
                QStandardPaths::DesktopLocation);

    QString imagePath = QFileDialog::getOpenFileName(
            this, tr("Open File"), desktop_abs.first(),
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp);;WEBP (*.webp)"));

    if (imagePath.isEmpty()) {
        qDebug() << "Empty string returned";
        return;
    }

    if (! QFileInfo(imagePath).isReadable()) {
        qDebug() << "File not readable";
        QMessageBox::critical(this, tr("Critical Error"), tr("File is not readable"));
        return;
    }

    m_image = new QImage();
    m_image->load(imagePath);

    m_orig_size = QFileInfo(imagePath).size();

    ui->lbl_size->setText(QString::number(m_orig_size/1024.00));

    ui->lbl_width->setText(QString::number(m_image->width()));
    ui->lbl_height->setText(QString::number(m_image->height()));

    ui->lbl_running->setStyleSheet("QLabel { background-color : green; color : black; }");

    m_pixmap = QPixmap::fromImage(*m_image);
    show_pixmap();

    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

}

void MainWindow::on_btn_save_clicked() {
    QString imagePath = QFileDialog::getSaveFileName(
            this,tr("Save File"),/*QDir::rootPath()*/ "/home/arda/Masaüstü",
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp);;WEBP (*.webp)"));

    *m_image = m_pixmap.toImage();
     m_image->save(imagePath);
}

void MainWindow::show_pixmap() {
    if (! m_scene) {
        m_scene = new QGraphicsScene(this);
    }
    else {
        m_scene->clear();
    }

    m_pixmap = m_pixmap.scaled(m_image->width(), m_image->height());

    m_scene->addPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());

    ui->lbl_running->setStyleSheet("QLabel { background-color : green; color : black; }");

    m_processing = false;
}

void MainWindow::reprocess_image(int scale, int quality) {

    if (m_processing) {
        return;
    }

    ui->lbl_running->setStyleSheet("QLabel { background-color : red; color : black; }");

    QtConcurrent::run(this, &MainWindow::reprocess_image_impl, scale, quality);
}

void MainWindow::reprocess_image_impl(int scale, int quality) {

    rescale_image(scale);
    requality_image(quality);

    QMetaObject::invokeMethod(this, "show_pixmap");
}

void MainWindow::rescale_image(int scale) {
    int w = m_image->width();
    int h = m_image->height();
    int new_w = (w * scale)/100;
    int new_h = (h * scale)/100;

    ui->lbl_width->setText(QString::number(new_w));
    ui->lbl_height->setText(QString::number(new_h));

    m_pixmap = QPixmap::fromImage(
                m_image->scaled(new_w, new_h, Qt::KeepAspectRatio, Qt::FastTransformation));

    ui->lbl_scale->setText(QString::number(scale));
}

void MainWindow::requality_image(int quality) {
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    m_pixmap.save(&buffer, "WEBP", quality);

    auto l_size_b = buffer.size();

    qDebug() << "x = " << buffer.size();

    double l_size_kb = buffer.size() / 1024.00;
    ui->lbl_size->setText(QString::number(l_size_kb));

    QImage image;
    image.loadFromData(ba);
    m_pixmap = QPixmap::fromImage(image);

    auto sld_value_quality = ui->sld_quality->value();
    ui->lbl_quality->setText(QString::number(sld_value_quality));

    double comp_p = 100.0 * l_size_b / m_orig_size;

    if(comp_p>100) {
        ui->lbl_compression->setText(QString::number(comp_p));
        QLabel* m_label = ui->lbl_size;
        m_label->setStyleSheet("QLabel { background-color : red; color : black; }");
    }
    else if(comp_p<=100) {
        ui->lbl_compression->setText(QString::number(comp_p));
        QLabel* m_label = ui->lbl_size;
        m_label->setStyleSheet("QLabel { background-color : rgba(0,0,0,0); color : black; }");
    }
}

void MainWindow::on_sld_quality_valueChanged(int value) {
    reprocess_image(ui->sld_scale->value(), value);
}

void MainWindow::on_sld_scale_valueChanged(int scale) {
    reprocess_image(scale, ui->sld_quality->value());
}



void MainWindow::on_btn_zoomin_clicked(){
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    double scaleFactor = 1.15;
    ui->graphicsView-> scale(scaleFactor, scaleFactor);
}

void MainWindow::on_btn_zoomout_clicked(){
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    double scaleFactor = 1.15;
    ui->graphicsView->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
}

void MainWindow::on_btn_rotate_right_clicked(){
    ui->graphicsView->rotate(90);
}

void MainWindow::on_btn_rotate_left_clicked(){
    ui->graphicsView->rotate(-90);
}
