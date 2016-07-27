/*
 * This file is part of Arskom EasyCompress Image Compression Tool
 * source code package.
 *
 * Copyright (C) 2016 Arskom Ltd. (http://arskom.com.tr)
 * Please contact Burak ARSLAN <burak.arslan@arskom.com.tr>
 *                Arda OZDEMIR <arda.ozdemir@arskom.com.tr>
 * for more information.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

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
#include <QTimer>
#include <QDebug>
#include <QMatrix>
#include <QBuffer>
#include <QSlider>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QImageReader>
#include <QGraphicsView>
#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QStandardPaths>

static QString LBL_NEW_SIZE_TEXT = QStringLiteral(
            "<html><head/><body><p><span style=\" font-weight:600;\">%1</span> KB, "
            "<span style=\"font-weight:600; color:%2;\">%3%4</span>"
            " %5</p></body></html>");


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    m_current_scale(0),
    m_current_size(0),
    m_fast(false),
    m_orig_size(0),
    m_processing(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lyt_transform->setAlignment(ui->sld_zoom, Qt::AlignHCenter);

    ui->graphicsView->setSlider(ui->sld_zoom);

    connect(ui->btn_open, SIGNAL(clicked(bool)), ui->action_open, SIGNAL(triggered(bool)));
    connect(ui->btn_save, SIGNAL(clicked(bool)), ui->action_save_as, SIGNAL(triggered(bool)));

    m_scene = new QGraphicsScene(this);

    ui->sld_quality->setDisabled(true);
    ui->sld_scale->setDisabled(true);
    ui->sld_zoom->setDisabled(true);
    ui->btn_save->setDisabled(true);
    ui->btn_rotate_left->setDisabled(true);
    ui->btn_rotate_right->setDisabled(true);

    this->setWindowTitle("Arskom EasyCompress");

    ui->lbl_new_size->clear();
    ui->lbl_new_dimensions->clear();

    ui->lbl_orig_size->clear();
    ui->lbl_orig_dimensions->clear();

    m_scene = new QGraphicsScene(this);
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

    auto formats = QImageReader::supportedImageFormats();
    if (! formats.contains("webp")) {
        auto formats_str = QLatin1String(formats.join(", "));

        QMessageBox::critical(this, tr("WebP Not Supported"),
                              tr("Please install WebP Support for Qt before running this program.\n\n"
                                 "Supported formats are: (%1)").arg(formats_str));

        // must call quit() after exec() in main
        QTimer::singleShot(0, qApp, SLOT(quit()));
    }
}

void MainWindow::on_action_open_triggered(){
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
    m_new_size = m_orig_size;

    m_pixmap = QPixmap::fromImage(m_orig_image);
    show_pixmap();

    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    ui->statusBar->showMessage(tr("%1 was opened successfully (%2KB).")
                                 .arg(file_info.baseName()).arg(m_orig_size / 1024.0, 0, 'f', 1));

    ui->sld_quality->setDisabled(false);
    ui->sld_scale->setDisabled(false);
    ui->sld_zoom->setDisabled(false);
    ui->btn_save->setDisabled(false);
    ui->btn_rotate_left->setDisabled(false);
    ui->btn_rotate_right->setDisabled(false);

    reprocess_image(ui->sld_scale->value(), ui->sld_quality->value());
}

void MainWindow::on_action_save_as_triggered() {
    if (m_image_path.isEmpty()) {
        qDebug() << "Empty string returned";
        return;
    }

    const auto &desktop_abs = QStandardPaths::standardLocations(
                QStandardPaths::DesktopLocation);

    QString imagePath = QFileDialog::getSaveFileName(
            this, tr("Save File"), desktop_abs.first(),
            tr("WEBP (*.webp)"));

    m_orig_image = m_pixmap.toImage();
    m_orig_image.save(imagePath);
}

void MainWindow::on_actionExit_triggered(){
    close();
}

void MainWindow::show_pixmap_fast() {
    m_scene->clear();
    m_scene->addPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());
}

void MainWindow::show_pixmap() {
    std::lock_guard<std::mutex> guard(m_mutex);

    m_scene->clear();

    auto orig_w = m_orig_image.width();
    auto orig_h = m_orig_image.height();

    m_scene->addPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());

    /* set original image information */
    ui->lbl_orig_size->setTextFormat(Qt::RichText);
    ui->lbl_orig_size->setText(
            QStringLiteral("<b>%1</b> KB")
                .arg(m_orig_size / 1024.0, 0, 'f', 1));


    ui->lbl_orig_dimensions->setTextFormat(Qt::RichText);
    ui->lbl_orig_dimensions->setText(
            QStringLiteral("%1x%2 (<b>%3</b> megapixels)")
                .arg(m_orig_image.width())
                .arg(m_orig_image.height())
                .arg(orig_w * orig_h / 1e6, 0, 'f', 1));

    /* set slider information */
    ui->lbl_scale->setText(QStringLiteral("%1").arg(m_current_scale));
    ui->lbl_quality->setText(QString::number(ui->sld_quality->value()));

    /* set slider information */
    ui->lbl_new_dimensions->setTextFormat(Qt::RichText);
    ui->lbl_new_dimensions->setText(
            QStringLiteral("%1x%2 (<b>%3</b> megapixels)")
                .arg(m_new_w)
                .arg(m_new_h)
                .arg(m_new_w * m_new_h / 1e6, 0, 'f', 1));


    /* set compressed image information */
    int comp_ratio = 100.0 * m_new_size / m_orig_size;
    int weight_loss = 100 - comp_ratio;

    ui->lbl_new_size->setTextFormat(Qt::RichText);
    QString lbl_new_size_text = LBL_NEW_SIZE_TEXT
            .arg(m_new_size / 1024.0, 0, 'f', 1);

    if (comp_ratio == 100) {
        ui->lbl_new_size->setText(lbl_new_size_text.arg("black")
                                  .arg("No").arg("").arg(tr("compression")));
    }
    else if(comp_ratio > 100) {
        ui->lbl_new_size->setText(lbl_new_size_text.arg("red")
                                  .arg(weight_loss).arg("%").arg(tr("increase")));
    }
    else if(comp_ratio < 100) {
        ui->lbl_new_size->setText(lbl_new_size_text.arg("green")
                                  .arg(weight_loss).arg("%").arg(tr("compression")));
    }

    m_processing = false;

    m_loading_animation = new QMovie(":/images/loading.gif");
    m_loading_animation->stop();
    ui->lbl_busy->setAttribute(Qt::WA_NoSystemBackground);
    ui->lbl_busy->setMovie(m_loading_animation);

}

void MainWindow::reprocess_image_fast(int scale, int) {
    m_new_w = (m_orig_image.width() * scale) / 100;
    m_new_h = (m_orig_image.height() * scale)/100;

    m_pixmap = QPixmap::fromImage(
                m_orig_image.scaled(m_new_w, m_new_h, Qt::KeepAspectRatio, Qt::FastTransformation));

    show_pixmap_fast();
}

void MainWindow::reprocess_image(int scale, int quality) {
    if (m_fast) {
        reprocess_image_fast(scale, quality);
    }
    else {
        reprocess_image_smooth(scale, quality);
    }
}

void MainWindow::reprocess_image_smooth(int scale, int quality) {
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
    double w = m_orig_image.width();
    double h = m_orig_image.height();

    m_new_w = static_cast<int>(std::round((w * scale) / 100));
    m_new_h = static_cast<int>(std::round((h * scale) / 100));

    m_current_scale = scale;

    m_pixmap = QPixmap::fromImage(m_orig_image.scaled(m_new_w, m_new_h,
                                                    Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // for showing to the user
    m_pixmap = m_pixmap.scaled(m_orig_image.width(), m_orig_image.height(),
                                                    Qt::KeepAspectRatio, Qt::FastTransformation);

    return true;
}

bool MainWindow::requality_image(int quality) {
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    m_pixmap.save(&buffer, "WEBP", quality);

    m_new_size = buffer.size();

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

void MainWindow::on_btn_zoomin_clicked() {
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    int val = ui->sld_zoom->value();
    val = val + 3;
    ui->sld_zoom->setValue(val);
}

void MainWindow::on_btn_zoomout_clicked() {
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
    m_sld_zoom_value = value;

    m_ZoomFactor = pow(10,((value-100) / 100.0));
    qDebug() << "zoom factor = " << m_ZoomFactor << "// zoom slider value = " << value;

    QMatrix matrix;
    matrix.scale(m_ZoomFactor, m_ZoomFactor);

    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setMatrix(matrix);
}
