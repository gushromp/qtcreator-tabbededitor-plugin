#include "tabsforeditorswidget.h"
#include "contexttabwidget.h"

#include "tabbededitorconstants.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/idocument.h>

#include <QShortcut>
#include <QSignalMapper>

using namespace Core::Internal;

using namespace TabbedEditor::Internal;

TabsForEditorsWidget::TabsForEditorsWidget(QWidget *parent) :
    QWidget(parent),
    m_tabWidget(new ContextTabWidget(this))
{
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_tabWidget->sizePolicy().hasHeightForWidth());
    m_tabWidget->setSizePolicy(sizePolicy);
    m_tabWidget->setUsesScrollButtons(true);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    Core::EditorManager *em = Core::EditorManager::instance();

    foreach (Core::IEditor *editor, em->visibleEditors()) {
        QWidget *tab = new QWidget();
        m_tabWidget->addTab(tab, editor->document()->displayName());
        m_tabsEditors.insert(tab, editor);
    }

    connect(em, SIGNAL(editorOpened(Core::IEditor*)), SLOT(handleEditorOpened(Core::IEditor*)));
    connect(em, SIGNAL(currentEditorChanged(Core::IEditor*)), SLOT(updateCurrentTab(Core::IEditor*)));
    connect(em, SIGNAL(editorsClosed(QList<Core::IEditor*>)), SLOT(handlerEditorClosed(QList<Core::IEditor*>)));

    connect(m_tabWidget, SIGNAL(currentChanged(int)), SLOT(handleCurrentChanged(int)));
    connect(m_tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(handleTabCloseRequested(int)));

    connect(m_tabWidget, SIGNAL(tabRightClicked(int,QPoint&)), SLOT(handleTabRightButtonClick(int,QPoint&)));
    connect(m_tabWidget, SIGNAL(tabMiddleClicked(int,QPoint&)), SLOT(handleTabMiddleButtonClick(int,QPoint&)));

    QString shortCutSequence = QLatin1String("Ctrl+Alt+%1");
    for (int i = 1; i <= 10; ++i) {
        int key = i;
        if (key == 10)
            key = 0;
        QShortcut *shortcut = new QShortcut(shortCutSequence.arg(key), m_tabWidget);
        m_tabShortcuts.append(shortcut);
        connect(shortcut, SIGNAL(activated()), SLOT(selectTabAction()));
    }

    QAction *prevTabAction = new QAction(tr("Switch to previous tab"), this);
    Core::Command *prevTabCommand = Core::ActionManager::registerAction(prevTabAction,
                                      TabbedEditor::Constants::PREV_TAB_ID,
                                      Core::Context(Core::Constants::C_GLOBAL));
    prevTabCommand->setDefaultKeySequence(QKeySequence(tr("Ctrl+shift+j")));
    connect(prevTabAction, SIGNAL(triggered()), this, SLOT(prevTabAction()));

    QAction *nextTabAction = new QAction(tr("Switch to next tab"), this);
    Core::Command *nextTabCommand = Core::ActionManager::registerAction(nextTabAction,
                                      TabbedEditor::Constants::NEXT_TAB_ID,
                                      Core::Context(Core::Constants::C_GLOBAL));
    nextTabCommand->setDefaultKeySequence(QKeySequence(tr("Ctrl+shift+k")));
    connect(nextTabAction , SIGNAL(triggered()), this, SLOT(nextTabAction()));

    setupContextMenu();
}

QWidget *TabsForEditorsWidget::tabWidget() const
{
    return m_tabWidget;
}

void TabsForEditorsWidget::updateCurrentTab(Core::IEditor *editor)
{
    disconnect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(handleCurrentChanged(int))); //prevent update
    if (!editor)
        return;
    QWidget *currentTab = getTab(editor);
    if (!currentTab)
        return;
    m_tabWidget->setCurrentWidget(currentTab);
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(handleCurrentChanged(int))); //restore update
}

void TabsForEditorsWidget::handleCurrentChanged(int index)
{
    if (index == -1)
        return;

    QWidget *tab = m_tabWidget->widget(index);
    if (!tab)
        return;
    if (m_tabsEditors.contains(tab)) {
        Core::IEditor *editor = getEditor(tab);
        if (!editor)
            return;
        Core::EditorManager::instance()->activateEditor(editor);
    }
}

void TabsForEditorsWidget::handleEditorOpened(Core::IEditor *editor)
{
    QWidget *tab = new QWidget();
    m_tabWidget->addTab(tab, editor->document()->displayName());
    m_tabsEditors.insert(tab, editor);
    connect(editor->document(), SIGNAL(changed()), SLOT(updateTabText()));
}

void TabsForEditorsWidget::handlerEditorClosed(QList<Core::IEditor*> editors)
{
    foreach (Core::IEditor *editor, editors) {
        if (!editor)
            continue;
        QWidget *tab = getTab(editor);
        if (!tab)
            continue;

        m_tabsEditors.remove(tab);
        if (-1 < m_tabWidget->indexOf(tab))
            m_tabWidget->removeTab(m_tabWidget->indexOf(tab));
    }
}

void TabsForEditorsWidget::handleContextMenuSelected(int choice)
{
    switch (choice) {
    case Constants::CONTEXT_MENU_CLOSE_TAB:
        handleTabCloseRequested();
        break;
    case Constants::CONTEXT_MENU_CLOSE_ALL_BUT_THIS_TAB:
        handleTabCloseAllExceptOneRequested();
        break;
    case Constants::CONTEXT_MENU_CLOSE_TABS_TO_THE_RIGHT:
        handleTabCloseToRightRequested();
        break;
    case Constants::CONTEXT_MENU_CLOSE_ALL_TABS:
        handleTabCloseAllRequested();
        break;
    }
}

void TabsForEditorsWidget::handleTabCloseRequested(int index)
{
    QList<int> singleIndexList;
    singleIndexList.append(index);

    handleTabsMultipleCloseRequested(singleIndexList);
}

void TabsForEditorsWidget::handleTabsMultipleCloseRequested(QList<int>& indices)
{
    QList<Core::IEditor*> editorsToClose;
    editorsToClose.clear();

    foreach(int index, indices) {
        if (-1 < index) {
            QWidget *tab = m_tabWidget->widget(index);
            if (!tab)
                return;

            if (m_tabsEditors.contains(tab)) {
                Core::IEditor *editor = getEditor(tab);
                if (!editor)
                    continue;
                editorsToClose.append(editor);
            }
        }
    }

    Core::EditorManager::instance()->closeEditors(editorsToClose);
}

void TabsForEditorsWidget::handleTabCloseRequested()
{
    handleTabCloseRequested(m_currentTabIndex);
}

void TabsForEditorsWidget::handleTabCloseToRightRequested()
{
    QList<int> indices;

    for(int i = m_currentTabIndex + 1; i < m_tabsEditors.size(); i++)
        indices.append(i);

    handleTabsMultipleCloseRequested(indices);
}

void TabsForEditorsWidget::handleTabCloseAllExceptOneRequested()
{
    QList<int> indices;

    for(int i = 0; i < m_tabsEditors.size(); i++)
        if(i != m_currentTabIndex)
            indices.append(i);

    handleTabsMultipleCloseRequested(indices);
}

void TabsForEditorsWidget::handleTabCloseAllRequested()
{
    Core::EditorManager::instance()->closeAllEditors();
}

void TabsForEditorsWidget::selectTabAction()
{
    QShortcut *shortcut = qobject_cast<QShortcut*>(sender());
    if (!shortcut)
        return;
    int index = m_tabShortcuts.indexOf(shortcut);
    m_tabWidget->setCurrentIndex(index);
}

void TabsForEditorsWidget::prevTabAction()
{
  int currentIndex = m_tabWidget->currentIndex();
  if (currentIndex >= 1)
    m_tabWidget->setCurrentIndex(currentIndex - 1);
  else
    m_tabWidget->setCurrentIndex(m_tabWidget->count() - 1);
}

void TabsForEditorsWidget::nextTabAction()
{
  int currentIndex = m_tabWidget->currentIndex();
  if (currentIndex < m_tabWidget->count() - 1)
    m_tabWidget->setCurrentIndex(currentIndex + 1);
  else
    m_tabWidget->setCurrentIndex(0);
}

void TabsForEditorsWidget::handleTabRightButtonClick(int tabIndex, QPoint &position)
{
    m_currentTabIndex = tabIndex;

    m_contextMenu->exec(mapToGlobal(position));
}

void TabsForEditorsWidget::handleTabMiddleButtonClick(int tabIndex, QPoint &)
{
    handleTabCloseRequested(tabIndex);
}

void TabsForEditorsWidget::setupContextMenu()
{
    m_contextMenu = new QMenu(this);

    m_closeTabAction = new QAction(tr("Close Tab"), this);
    m_closeAllTabsExceptThisAction = new QAction(tr("Close All BUT This Tab"), this);
    m_closeTabsToRightAction = new QAction(tr("Close Tabs to the Right"), this);
    m_closeAllTabsAction = new QAction(tr("Close All Tabs"), this);

    connect(m_closeTabAction, SIGNAL(triggered()), this, SLOT(handleTabCloseAllRequested()));
    connect(m_closeAllTabsExceptThisAction, SIGNAL(triggered()), this, SLOT(handleTabCloseAllExceptOneRequested()));
    connect(m_closeTabsToRightAction, SIGNAL(triggered()), this, SLOT(handleTabCloseToRightRequested()));
    connect(m_closeAllTabsAction, SIGNAL(triggered()), this, SLOT(handleTabCloseAllRequested()));

    m_actionsList.append(m_closeTabAction);
    m_actionsList.append(m_closeAllTabsExceptThisAction);
    m_actionsList.append(m_closeTabsToRightAction);
    m_actionsList.append(m_closeAllTabsAction);

    m_contextMenu->addActions(m_actionsList);
}

void TabsForEditorsWidget::closeTab(int index)
{
    QWidget *tab = m_tabWidget->widget(index);

    if (m_tabsEditors.contains(tab))
        m_tabsEditors.remove(tab);
    if (-1 < index)
    {
        m_tabWidget->removeTab(0);
    }
}

void TabsForEditorsWidget::updateTabText()
{
    Core::IDocument *document = qobject_cast<Core::IDocument*>(QObject::sender());
    if (!document)
        return;

    QString tabTitle = document->displayName();
    if (document->isModified())
        tabTitle += QString::fromUtf8("*");

    Core::EditorManager *em = Core::EditorManager::instance();
    Core::IEditor *editor = em->activateEditorForDocument(document);
    QWidget *tabToUpdate = this->getTab(editor);
    int tabToUpdateIndex = m_tabWidget->indexOf( tabToUpdate );
    m_tabWidget->setTabText(tabToUpdateIndex , tabTitle);
    /*
    QList<Core::IEditor*> editors
            = Core::EditorManager::instance()->documentModel()->editorsForDocument(document);
    foreach (Core::IEditor *editor, editors) {
        QWidget *tabToUpdate = getTab(editor);
        int tabToUpdateIndex = m_tabWidget->indexOf(tabToUpdate);
        m_tabWidget->setTabText(tabToUpdateIndex, tabTitle);
    }
    */ 
}

Core::IEditor *TabsForEditorsWidget::getEditor(QWidget *tab) const
{
    return m_tabsEditors.value(tab);
}

QWidget *TabsForEditorsWidget::getTab(Core::IEditor *editor) const
{
    return m_tabsEditors.key(editor);
}

bool TabsForEditorsWidget::isEditorWidget(QObject *obj) const
{
    if (!obj)
        return false;
    QMapIterator<QWidget*, Core::IEditor*> i(m_tabsEditors);
    while (i.hasNext()) {
        i.next();
        if (obj == i.value()->widget())
            return true;
    }
    return false;
}
