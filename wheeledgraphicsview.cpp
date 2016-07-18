#include "wheeledgraphicsview.h"
#include "mainwindow.h"

#include <QGraphicsView>
#include <QWheelEvent>



WheeledGraphicsView::WheeledGraphicsView(QWidget *parent) : QGraphicsView(parent){
}


void WheeledGraphicsView::wheelEvent(QWheelEvent *event){

        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        double scaleFactor = 1.15;
        if(event->delta() > 0) {
            emit scale(scaleFactor, scaleFactor);
        }
        else {
            emit scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
}




