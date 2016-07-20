#include "wheeledgraphicsview.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScrollBar>
#include <QWheelEvent>
#include <QGraphicsView>

const double WheeledGraphicsView::ZOOM_RATIO_PER_WHEEL_TICK = 1.15;


WheeledGraphicsView::WheeledGraphicsView(QWidget *parent) : QGraphicsView(parent){

}

void WheeledGraphicsView::wheelEvent(QWheelEvent *event){
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    if(event->orientation() == Qt::Horizontal) {
        event->ignore();
    }
    else {
        if(event->delta() > 0) {
            scale(ZOOM_RATIO_PER_WHEEL_TICK, ZOOM_RATIO_PER_WHEEL_TICK);
        }
        else {
            scale(1.0 / ZOOM_RATIO_PER_WHEEL_TICK, 1.0 / ZOOM_RATIO_PER_WHEEL_TICK);
        }
    }
}
