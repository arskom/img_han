#ifndef WHEELEDGRAPHICSVIEW_H
#define WHEELEDGRAPHICSVIEW_H

#include <QGraphicsView>

QT_FORWARD_DECLARE_CLASS(QSlider)

class WheeledGraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    explicit WheeledGraphicsView(QWidget *parent = 0);
    virtual void setSlider(QSlider *slider);

protected:
    virtual void wheelEvent(QWheelEvent* event) override;

private:
    QSlider *m_slider;
};

#endif // WHEELEDGRAPHICSVIEW_H
