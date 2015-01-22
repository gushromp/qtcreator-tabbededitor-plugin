#ifndef TABSFOREDITORSWIDGET_H
#define TABSFOREDITORSWIDGET_H

#include <QMap>
#include <QWidget>
#include <QMenu>
#include <QSignalMapper>

QT_BEGIN_NAMESPACE
class QShortcut;
class QTabWidget;
QT_END_NAMESPACE

class ContextTabWidget;

namespace Core {
class IEditor;
}

namespace TabbedEditor {
namespace Internal {

class TabsForEditorsWidget : public QWidget
{
    Q_OBJECT

public:
    TabsForEditorsWidget(QWidget * parent = 0);

    QWidget *tabWidget() const;

private slots:
    void updateCurrentTab(Core::IEditor *getEditor);
    void handleCurrentChanged(int index);
    void handleEditorOpened(Core::IEditor *getEditor);
    void handlerEditorClosed(QList<Core::IEditor*> editors);
    void handleContextMenuSelected(int choice);
    void handleTabCloseRequested(int index);
    void handleTabsMultipleCloseRequested(QList<int>& indices);
    void handleTabCloseToRightRequested(int index);
    void handleTabCloseAllExceptOneRequested(int index);
    void handleTabCloseAllRequested();
    void selectTabAction();
    void updateTabText();
    void prevTabAction();
    void nextTabAction();
    void handleContextMenuEvent(QContextMenuEvent *) { }
    void handleTabRightButtonClick(int tabIndex, QPoint& position);
    void handleTabMiddleButtonClick(int tabIndex, QPoint& position);


private:
    void setupContextMenu();
    void closeTab(int index);

    Core::IEditor *getEditor(QWidget *getTab) const;
    QWidget *getTab(Core::IEditor *getEditor) const;
    bool isEditorWidget(QObject *obj) const;

    ContextTabWidget *m_tabWidget;
    QMap<QWidget *, Core::IEditor *> m_tabsEditors;
    QList<QShortcut *> m_tabShortcuts;

    QMenu* m_contextMenu;

    QSignalMapper* m_signalMapper;
    QList<QAction*> m_actionsList;
    QAction *m_closeTabAction, *m_closeAllTabsExceptThisAction, *m_closeTabsToRightAction
        , *m_closeAllTabsAction;

    int m_currentTabIndex;

};

} // namespace Internal
} // namespace TabbedEditor

#endif // TABSFOREDITORSWIDGET_H
