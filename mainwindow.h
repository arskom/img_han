#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QMutex>

#include <string>
#include <mutex>

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void showEvent(QShowEvent *e) override;

private slots:
    void on_btn_open_clicked();
    void on_btn_save_clicked();

    void on_sld_quality_valueChanged(int value);
    void on_sld_scale_valueChanged(int value);

    void on_btn_zoomin_clicked();
    void on_btn_zoomout_clicked();

    void on_btn_rotate_right_clicked();
    void on_btn_rotate_left_clicked();

    // runs in gui thread
    void show_pixmap();

    // runs in concurrent thread
    void reprocess_image_impl(int scale, int quality);


private:
    void reprocess_image(int scale, int quality);
    void rescale_image(int);
    void requality_image(int);

    void change_size();

    bool m_processing;
    qint16 m_current_scale;
    qint64 m_current_size;

    int m_new_w;
    int m_new_h;

    QMovie *m_mv;
    std::mutex m_mutex;
    Ui::MainWindow *ui;
    QPixmap m_pixmap;
    QImage  *m_image;
    qint64 m_orig_size;
    QGraphicsScene *m_scene;
};

#endif // MAINWINDOW_H
