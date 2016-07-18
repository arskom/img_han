#ifndef WHEELEDGRAPHICSVIEW_H
#define WHEELEDGRAPHICSVIEW_H

#include <QGraphicsView>

class WheeledGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit WheeledGraphicsView(QWidget *parent = 0);

protected:
    virtual void wheelEvent(QWheelEvent* event) override;

signals:

public slots:
};

#endif // WHEELEDGRAPHICSVIEW_H
