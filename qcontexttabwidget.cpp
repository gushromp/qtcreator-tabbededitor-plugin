// Based on qmditabwidget by Diego Iastrubni
// https://code.google.com/p/qtedit4/source/browse/tools/qmdilib/src/qmditabwidget.cpp

#include "qcontexttabwidget.h"

#include <QTabBar>
#include <QEvent>
#include <QMouseEvent>

QContextTabWidget::QContextTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    tabBar()->installEventFilter(this);
}

QContextTabWidget::~QContextTabWidget()
{

}

void QContextTabWidget::handleContextMenuEvent(QContextMenuEvent *)
{

}

bool QContextTabWidget::eventFilter(QObject *obj, QEvent *event)
{
        if (obj != tabBar())
                return QObject::eventFilter(obj, event);

        if (event->type() != QEvent::MouseButtonPress)
                return QObject::eventFilter(obj, event);

        // compute the tab number

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QPoint position = mouseEvent->pos();
        int clickedItem = tabBar()->tabAt( position );

        // just in case
        if (clickedItem == -1)
                return QObject::eventFilter(obj, event);

        switch( mouseEvent->button() ) {
                case Qt::LeftButton:
                        return QObject::eventFilter(obj, event);
                        break;

                case Qt::RightButton:
                        emit tabRightClicked(clickedItem, position);
                        break;

                case Qt::MidButton:
                        emit tabMiddleClicked(clickedItem, position);
                        break;

                default:
                        return QObject::eventFilter(obj, event);
        }

        return true;
}

