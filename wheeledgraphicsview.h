#ifndef WHEELEDGRAPHICSVIEW_H
#define WHEELEDGRAPHICSVIEW_H

#include <QGraphicsView>

class WheeledGraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    static const double ZOOM_RATIO_PER_WHEEL_TICK;

    explicit WheeledGraphicsView(QWidget *parent = 0);

protected:
    virtual void wheelEvent(QWheelEvent* event) override;
};

#endif // WHEELEDGRAPHICSVIEW_H
