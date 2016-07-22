#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wheeledgraphicsview.h"

#include <QtConcurrent/QtConcurrentRun>

#include <math.h>

#include <mutex>
#include <string>
#include <iostream>

#include <QMovie>
#include <QEvent>
#include <QImage>
#include <QLabel>
#include <QDebug>
#include <QMatrix>
#include <QBuffer>
#include <QSlider>
#include <QScrollBar>
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
    m_orig_size(0),
    m_processing(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lyt_transform->setAlignment(ui->sld_zoom, Qt::AlignHCenter);

    ui->graphicsView->setSlider(ui->sld_zoom);

    connect(ui->btn_open, SIGNAL(clicked(bool)), ui->actionOpen, SIGNAL(triggered(bool)));
    connect(ui->btn_save, SIGNAL(clicked(bool)), ui->actionSave, SIGNAL(triggered(bool)));

    this->setWindowTitle("Arskom EasyCompress");
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::showEvent(QShowEvent *e) {
    QMainWindow::showEvent(e);

    const auto &desktop_rect = QApplication::desktop()->screen()->rect();

    // TODO: Store last window state and position
    {
        this->resize(desktop_rect.width() * .75, desktop_rect.height() * .75);
        this->move(desktop_rect.center() - this->rect().center());\

        auto toolbox_minw = ui->toolBox->minimumWidth();
        ui->splitter->setSizes(QList<int>() << toolbox_minw << width() - toolbox_minw);
    }
}

void MainWindow::on_actionOpen_triggered(){
    const auto &desktop_abs = QStandardPaths::standardLocations(
                QStandardPaths::DesktopLocation);

    m_image_path = QFileDialog::getOpenFileName(
            this, tr("Open File"), desktop_abs.first(),
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp);;WEBP (*.webp)"));

    if (m_image_path.isEmpty()) {
        qDebug() << "Empty string returned";
        return;
    }

    const auto &file_info = QFileInfo(m_image_path);

    if (! file_info.isReadable()) {
        qDebug() << "File not readable";
        QMessageBox::critical(this, tr("Critical Error"), tr("File is not readable"));
        return;
    }

    m_orig_image.load(m_image_path);

    m_orig_size = file_info.size();

    m_new_w = m_orig_image.width();
    m_new_h = m_orig_image.height();

    m_current_scale = 100;
    m_current_size = m_orig_size;

    m_pixmap = QPixmap::fromImage(m_orig_image);
    show_pixmap();

    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    ui->lbl_dimensions->setText(
                    QString("%1x%2").arg(m_orig_image.width())
                                    .arg(m_orig_image.height()));

    ui->statusBar->showMessage(tr("%1 was opened successfully (%2KB).")
                                            .arg(file_info.baseName()).arg(m_orig_size / 1024.0, 0, 'f', 1));

    reprocess_image(ui->sld_scale->value(), ui->sld_quality->value());
}

void MainWindow::on_actionSave_As_triggered() {
    const auto &desktop_abs = QStandardPaths::standardLocations(
                QStandardPaths::DesktopLocation);

    QString imagePath = QFileDialog::getOpenFileName(
            this, tr("Open File"), desktop_abs.first(),
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp);;WEBP (*.webp)"));

    if (imagePath.isEmpty()) {
        qDebug() << "Empty string returned";
        return;
    }

    m_orig_image = m_pixmap.toImage();
    m_orig_image.save(imagePath);
}

void MainWindow::on_actionExit_triggered(){
    close();
}

void MainWindow::show_pixmap() {
    if (! m_scene) {
        m_scene = new QGraphicsScene(this);
    }
    else {
        m_scene->clear();
    }

    m_pixmap = m_pixmap.scaled(m_orig_image.width(), m_orig_image.height());

    m_scene->addPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());

    ui->lbl_size_kb->setText(QStringLiteral("%1").arg(m_orig_size / 1024.0, 0, 'f', 1));
    ui->lbl_size_mp->setText(QStringLiteral("%1").arg(m_new_w * m_new_h / 1e6, 0, 'f', 1));
    ui->lbl_dimensions->setText(
                    QString("%1x%2").arg(m_new_w)
                                    .arg(m_new_h));

    ui->lbl_scale->setText(QString::number(m_current_scale));

    auto sld_value_quality = ui->sld_quality->value();
    double comp_p = 100.0 * m_current_size / m_orig_size;

    ui->lbl_quality->setText(QString::number(sld_value_quality));

    if(comp_p > 100) {
        ui->lbl_compression->setText(QString::number(comp_p));
        QLabel* m_label = ui->lbl_compression;
        m_label->setStyleSheet("QLabel { background-color : red; color : black; }");
    }
    else if(comp_p<=100) {
        ui->lbl_compression->setText(QString::number(comp_p));
        QLabel* m_label = ui->lbl_compression;
        m_label->setStyleSheet("QLabel { background-color : rgba(0,0,0,0); color : black; }");
    }

    double l_size_kb = m_current_size / 1024.00;
    ui->lbl_size_kb->setText(QString::number(l_size_kb));

    std::lock_guard<std::mutex> guard(m_mutex);
    m_processing = false;

    m_loading_animation = new QMovie(":/images/loading.gif");
    m_loading_animation->stop();
    ui->lbl_busy->setAttribute(Qt::WA_NoSystemBackground);
    ui->lbl_busy->setMovie(m_loading_animation);

}

void MainWindow::reprocess_image(int scale, int quality) {
    if (m_image_path.isEmpty()) {
        ui->sld_scale->setValue(100);
        ui->sld_quality->setValue(50);
        return;
    }

    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_processing) {
        return;
    }

    m_loading_animation = new QMovie(":/images/loading.gif");
    m_loading_animation->start();
    ui->lbl_busy->setAttribute(Qt::WA_NoSystemBackground);
    ui->lbl_busy->setMovie(m_loading_animation);

    QtConcurrent::run(this, &MainWindow::reprocess_image_impl, scale, quality);
}

void MainWindow::reprocess_image_impl(int scale, int quality) {

    std::lock_guard<std::mutex> guard(m_mutex);

    if (! rescale_image(scale)) {
        return;
    }

    if (! requality_image(quality)) {
        return;
    }

    QMetaObject::invokeMethod(this, "show_pixmap");
}

bool MainWindow::rescale_image(int scale) {
    int w = m_orig_image.width();
    int h = m_orig_image.height();

    m_new_w = (w * scale)/100;
    m_new_h = (h * scale)/100;

    m_current_scale = scale;

    m_pixmap = QPixmap::fromImage(
                m_orig_image.scaled(m_new_w, m_new_h, Qt::KeepAspectRatio, Qt::FastTransformation));

    return true;
}

bool MainWindow::requality_image(int quality) {
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    m_pixmap.save(&buffer, "WEBP", quality);

    m_current_size = buffer.size();

    qDebug() << "image size(b) = " << buffer.size();

    QImage image;
    image.loadFromData(ba);
    m_pixmap = QPixmap::fromImage(image);
    if (m_pixmap.isNull()) {
        QMessageBox::critical(this, tr("Critical Error"), tr("Image recompression error!"));
        return false;
    }

    return true;
}

void MainWindow::on_sld_quality_valueChanged(int value) {
    reprocess_image(ui->sld_scale->value(), value);
}

void MainWindow::on_sld_scale_valueChanged(int scale) {
    reprocess_image(scale, ui->sld_quality->value());
}

void MainWindow::on_btn_zoomin_clicked(){
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    int val = ui->sld_zoom->value();
    val = val + 3;
    ui->sld_zoom->setValue(val);
}

void MainWindow::on_btn_zoomout_clicked(){
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    int val = ui->sld_zoom->value();
    val = val - 3;
    ui->sld_zoom->setValue(val);
}

void MainWindow::on_btn_rotate_right_clicked(){
    ui->graphicsView->rotate(90);
}

void MainWindow::on_btn_rotate_left_clicked(){
    ui->graphicsView->rotate(-90);
}

void MainWindow::on_sld_zoom_valueChanged(int value){
    if (m_image_path.isEmpty()) {
        ui->sld_zoom->setValue(100);
        return;
    }

    m_sld_zoom_value = value;
    m_ZoomFactor = pow(10,((value-100) / 100.0));
    qDebug() << "zoom factor = " << m_ZoomFactor << "// zoom slider value = " << value;

    QMatrix matrix;
    matrix.scale(m_ZoomFactor, m_ZoomFactor);

    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setMatrix(matrix);
}
