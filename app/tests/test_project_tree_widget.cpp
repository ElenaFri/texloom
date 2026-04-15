#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTreeWidgetItem>
#include <QTemporaryDir>
#include "../src/ui/ProjectTreeWidget.h"
#include "../src/core/ProjectModel.h"

using namespace texloom;

class TestProjectTreeWidget : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        m_tree = new ProjectTreeWidget();
    }

    void cleanup()
    {
        delete m_tree;
    }

    // ========== INITIAL STATE ==========

    void testInitialState()
    {
        QCOMPARE(m_tree->topLevelItemCount(), 0);
    }

    // ========== setProjectRoot ==========

    void testSetProjectRoot()
    {
        m_tree->setProjectRoot("MyProject", "/home/user/myproject");

        QCOMPARE(m_tree->topLevelItemCount(), 1);
        QCOMPARE(m_tree->topLevelItem(0)->text(0), QString("MyProject"));
    }

    void testSetProjectRootReplacesExisting()
    {
        m_tree->setProjectRoot("FirstProject", "/path/first");
        m_tree->setProjectRoot("SecondProject", "/path/second");

        QCOMPARE(m_tree->topLevelItemCount(), 1);
        QCOMPARE(m_tree->topLevelItem(0)->text(0), QString("SecondProject"));
    }

    // ========== addFile ==========

    void testAddFileAppearsUnderRoot()
    {
        m_tree->setProjectRoot("Project", "/path");
        m_tree->addFile("/path/chapter1.md");

        QTreeWidgetItem *root = m_tree->topLevelItem(0);
        QCOMPARE(root->childCount(), 1);
        QCOMPARE(root->child(0)->text(0), QString("chapter1.md"));
    }

    void testAddMultipleFiles()
    {
        m_tree->setProjectRoot("Project", "/path");
        m_tree->addFile("/path/chapter1.md");
        m_tree->addFile("/path/chapter2.md");
        m_tree->addFile("/path/appendix.md");

        QCOMPARE(m_tree->topLevelItem(0)->childCount(), 3);
    }

    void testAddFileStoresFullPath()
    {
        m_tree->setProjectRoot("Project", "/path");
        m_tree->addFile("/path/chapter1.md");

        QTreeWidgetItem *item = m_tree->topLevelItem(0)->child(0);
        QCOMPARE(item->data(0, Qt::UserRole).toString(), QString("/path/chapter1.md"));
    }

    void testAddFileWithoutProjectRootDoesNotCrash()
    {
        // Should silently do nothing — no root set
        m_tree->addFile("/path/file.md");
        QCOMPARE(m_tree->topLevelItemCount(), 0);
    }

    // ========== removeFile ==========

    void testRemoveExistingFile()
    {
        m_tree->setProjectRoot("Project", "/path");
        m_tree->addFile("/path/chapter1.md");
        m_tree->addFile("/path/chapter2.md");

        m_tree->removeFile("/path/chapter1.md");

        QTreeWidgetItem *root = m_tree->topLevelItem(0);
        QCOMPARE(root->childCount(), 1);
        QCOMPARE(root->child(0)->data(0, Qt::UserRole).toString(), QString("/path/chapter2.md"));
    }

    void testRemoveNonExistentFileDoesNotCrash()
    {
        m_tree->setProjectRoot("Project", "/path");
        m_tree->addFile("/path/chapter1.md");

        m_tree->removeFile("/path/does_not_exist.md");

        QCOMPARE(m_tree->topLevelItem(0)->childCount(), 1);
    }

    void testRemoveWithoutProjectRootDoesNotCrash()
    {
        m_tree->removeFile("/path/file.md");
    }

    // ========== clear ==========

    void testClearRemovesAllItems()
    {
        m_tree->setProjectRoot("Project", "/path");
        m_tree->addFile("/path/chapter1.md");
        m_tree->addFile("/path/chapter2.md");

        m_tree->clear();

        QCOMPARE(m_tree->topLevelItemCount(), 0);
    }

    void testClearThenAddWorksAgain()
    {
        m_tree->setProjectRoot("Project", "/path");
        m_tree->addFile("/path/chapter1.md");
        m_tree->clear();

        // After clear, addFile without a root should do nothing
        m_tree->addFile("/path/chapter2.md");
        QCOMPARE(m_tree->topLevelItemCount(), 0);

        m_tree->setProjectRoot("NewProject", "/new");
        m_tree->addFile("/new/intro.md");
        QCOMPARE(m_tree->topLevelItem(0)->childCount(), 1);
    }

    // ========== SIGNALS ==========

    void testFileSelectedSignalOnClick()
    {
        m_tree->setProjectRoot("Project", "/path");
        m_tree->addFile("/path/chapter1.md");

        QSignalSpy spy(m_tree, &ProjectTreeWidget::fileSelected);

        QTreeWidgetItem *item = m_tree->topLevelItem(0)->child(0);
        // Simulate click by calling the slot via the tree's itemClicked signal
        emit m_tree->itemClicked(item, 0);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().at(0).toString(), QString("/path/chapter1.md"));
    }

    void testFileDoubleClickedSignal()
    {
        m_tree->setProjectRoot("Project", "/path");
        m_tree->addFile("/path/chapter1.md");

        QSignalSpy spy(m_tree, &ProjectTreeWidget::fileDoubleClicked);

        QTreeWidgetItem *item = m_tree->topLevelItem(0)->child(0);
        emit m_tree->itemDoubleClicked(item, 0);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().at(0).toString(), QString("/path/chapter1.md"));
    }

    void testRootItemClickDoesNotEmitFileSelected()
    {
        m_tree->setProjectRoot("Project", "/path");

        QSignalSpy spy(m_tree, &ProjectTreeWidget::fileSelected);

        // Root item has no UserRole data → should not emit
        QTreeWidgetItem *root = m_tree->topLevelItem(0);
        emit m_tree->itemClicked(root, 0);

        QCOMPARE(spy.count(), 0);
    }

    // ========== SIGNAL WIRING (model → tree integration) ==========

    void testFileAddedSignalWiresToTree()
    {
        ProjectModel model;
        connect(&model, &ProjectModel::fileAdded,
                m_tree, &ProjectTreeWidget::addFile);

        m_tree->setProjectRoot("Project", "/path");
        model.createProject("Project", QDir::tempPath() + "/test_wire_add");

        model.addFile("/path/chapter1.md");

        QTreeWidgetItem *root = m_tree->topLevelItem(0);
        QCOMPARE(root->childCount(), 1);
        QCOMPARE(root->child(0)->data(0, Qt::UserRole).toString(), QString("/path/chapter1.md"));

        model.closeProject();
        QDir(QDir::tempPath() + "/test_wire_add").removeRecursively();
    }

    void testFileRemovedSignalWiresToTree()
    {
        ProjectModel model;
        connect(&model, &ProjectModel::fileRemoved,
                m_tree, &ProjectTreeWidget::removeFile);

        m_tree->setProjectRoot("Project", "/path");
        m_tree->addFile("/path/chapter1.md");
        m_tree->addFile("/path/chapter2.md");

        model.createProject("Project", QDir::tempPath() + "/test_wire_rm");
        model.addFile("/path/chapter1.md");
        model.addFile("/path/chapter2.md");

        model.removeFile("/path/chapter1.md");

        QTreeWidgetItem *root = m_tree->topLevelItem(0);
        QCOMPARE(root->childCount(), 1);
        QCOMPARE(root->child(0)->data(0, Qt::UserRole).toString(), QString("/path/chapter2.md"));

        model.closeProject();
        QDir(QDir::tempPath() + "/test_wire_rm").removeRecursively();
    }

    void testProjectOpenedPopulatesTree()
    {
        ProjectModel model;
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());

        // Create a project with files
        model.createProject("TestProj", tmpDir.path());
        model.addFile("/some/file1.md");
        model.addFile("/some/file2.md");
        model.saveProject();
        QString projFile = model.projectFilePath();
        model.closeProject();

        // Now wire signals and load
        connect(&model, &ProjectModel::projectOpened, m_tree, [&](const QString &path)
                {
            m_tree->setProjectRoot(model.projectName(), path);
            for (const QString &f : model.files())
            {
                m_tree->addFile(f);
            } });

        model.loadProject(projFile);

        QTreeWidgetItem *root = m_tree->topLevelItem(0);
        QVERIFY(root != nullptr);
        QCOMPARE(root->text(0), QString("TestProj"));
        QCOMPARE(root->childCount(), 2);
    }

    void testProjectClosedClearsTree()
    {
        ProjectModel model;
        connect(&model, &ProjectModel::projectClosed,
                m_tree, &ProjectTreeWidget::clear);

        m_tree->setProjectRoot("Project", "/path");
        m_tree->addFile("/path/file.md");

        QCOMPARE(m_tree->topLevelItemCount(), 1);

        model.createProject("P", QDir::tempPath() + "/test_wire_close");
        model.closeProject();

        QCOMPARE(m_tree->topLevelItemCount(), 0);

        QDir(QDir::tempPath() + "/test_wire_close").removeRecursively();
    }

private:
    ProjectTreeWidget *m_tree = nullptr;
};

QTEST_MAIN(TestProjectTreeWidget)
#include "test_project_tree_widget.moc"
