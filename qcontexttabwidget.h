// Based on qmditabwidget by Diego Iastrubni
// https://code.google.com/p/qtedit4/source/browse/tools/qmdilib/src/qmditabwidget.cpp

#ifndef QCONTEXTTABWIDGET_H
#define QCONTEXTTABWIDGET_H

#include <QObject>
#include <QTabWidget>

class QContextTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    QContextTabWidget(QWidget* parent = 0);
    ~QContextTabWidget();

signals:
    void tabRightClicked(int tabIndex, QPoint& position);
    void tabMiddleClicked(int tabIndex, QPoint& position);

protected slots:
    void handleContextMenuEvent(QContextMenuEvent * evt);

private:
    bool eventFilter(QObject *obj, QEvent *event) override;

};

#endif // QCONTEXTTABWIDGET_H
