#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsPixmapItem>

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)

class QGraphicsScene;



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_openButton_clicked();
    void on_sld_quality_valueChanged(int value);
    void on_sld_scale_valueChanged(int value);
    void on_saveButton_clicked();

private:
    void show_pixmap();
    Ui::MainWindow *ui;
    QPixmap m_pixmap;
    QImage  *m_image;
    qint64 m_orig_size;
    QGraphicsScene *m_scene;
};

#endif // MAINWINDOW_H
