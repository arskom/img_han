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
    void on_sld_quality_valueChanged(int value);
    void on_sld_scale_valueChanged(int value);

    void on_btn_zoomin_clicked();
    void on_btn_zoomout_clicked();
    void on_sld_zoom_valueChanged(int value);

    void on_btn_rotate_right_clicked();
    void on_btn_rotate_left_clicked();

    // runs in gui thread
    void show_pixmap();

    // runs in concurrent thread
    void reprocess_image_impl(int scale, int quality);

    void on_actionOpen_triggered();
    void on_actionSave_As_triggered();
    void on_actionExit_triggered();

private:
    void reprocess_image(int scale, int quality);
    bool rescale_image(int);
    bool requality_image(int);

    void change_size();

    QString m_image_path;
    std::mutex m_mutex;
    QPixmap m_pixmap;

    qint16 m_current_scale;
    qint64 m_new_size;

    int m_new_w;
    int m_new_h;
    int m_sld_zoom_value;
    double m_ZoomFactor;

    QGraphicsScene *m_scene;
    QMovie *m_loading_animation;
    QImage m_orig_image;
    qint64 m_orig_size;
    bool m_processing;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
