#include "wheeledgraphicsview.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSlider>
#include <QScrollBar>
#include <QWheelEvent>
#include <QGraphicsView>


WheeledGraphicsView::WheeledGraphicsView(QWidget *parent)
        : QGraphicsView(parent)
        , m_slider(nullptr) {
}

void WheeledGraphicsView::setSlider(QSlider *slider) {
    m_slider = slider;
}


void WheeledGraphicsView::wheelEvent(QWheelEvent *event){
    if(event->orientation() == Qt::Horizontal) {
        event->ignore();
    }
    else {
        if(event->delta() < 0){
            int val = m_slider->value();
            val = val - 3;
            m_slider->setValue(val);
        }
        else if(event->delta() > 0){
            int val = m_slider->value();
            val = val + 3;
            m_slider->setValue(val);
        }
    }
}
