#include "MainWindow.h"
#include "../core/ProjectModel.h"
#include "../core/ConversionEngine.h"
#include "ProjectTreeWidget.h"
#include "EditorWidget.h"
#include "PreviewWidget.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QCloseEvent>

namespace texloom
{

    MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
    {
        // Create core components
        m_projectModel = new ProjectModel(this);
        m_conversionEngine = new ConversionEngine(this);

        // Setup UI
        setupUi();
        createActions();
        createMenus();
        createToolbar();
        createStatusBar();

        // Connect signals
        connect(m_projectModel, &ProjectModel::projectOpened,
                this, &MainWindow::onProjectOpened);
        connect(m_projectModel, &ProjectModel::projectClosed,
                this, &MainWindow::onProjectClosed);
        connect(m_projectModel, &ProjectModel::projectModified,
                this, &MainWindow::onProjectModified);

        connect(m_projectModel, &ProjectModel::fileAdded,
                m_projectTree, &ProjectTreeWidget::addFile);
        connect(m_projectModel, &ProjectModel::fileRemoved,
                m_projectTree, &ProjectTreeWidget::removeFile);

        connect(m_projectTree, &ProjectTreeWidget::fileSelected,
                this, &MainWindow::onFileSelected);
        connect(m_projectTree, &ProjectTreeWidget::fileDoubleClicked,
                this, &MainWindow::onFileDoubleClicked);

        connect(m_conversionEngine, &ConversionEngine::conversionStarted,
                this, &MainWindow::onConversionStarted);
        connect(m_conversionEngine, &ConversionEngine::conversionProgress,
                this, &MainWindow::onConversionProgress);
        connect(m_conversionEngine, &ConversionEngine::conversionCompleted,
                this, &MainWindow::onConversionCompleted);
        connect(m_conversionEngine, &ConversionEngine::conversionFailed,
                this, &MainWindow::onConversionFailed);

        connect(m_conversionEngine, &ConversionEngine::pdfGenerated,
                m_previewWidget, &PreviewWidget::loadPdf);

        connect(m_previewWidget, &PreviewWidget::pdfLoaded, this, [this](const QString &path)
                { statusBar()->showMessage(tr("PDF loaded: %1").arg(QFileInfo(path).fileName()), 3000); });
        connect(m_previewWidget, &PreviewWidget::pdfLoadFailed, this, [this](const QString &error)
                { statusBar()->showMessage(tr("PDF error: %1").arg(error), 5000); });

        // Initial state
        updateWindowTitle();
        updateActions();

        resize(1400, 900);
    }

    MainWindow::~MainWindow() = default;

    void MainWindow::setupUi()
    {
        // Create main splitter
        m_mainSplitter = new QSplitter(Qt::Horizontal, this);

        // Project tree (left panel) - placeholder for now
        m_projectTree = new ProjectTreeWidget(this);
        m_mainSplitter->addWidget(m_projectTree);

        // Editor tabs (center)
        m_editorTabs = new QTabWidget(this);
        m_editorTabs->setTabsClosable(true);
        m_editorTabs->setMovable(true);
        connect(m_editorTabs, &QTabWidget::tabCloseRequested,
                this, &MainWindow::onTabCloseRequested);
        m_mainSplitter->addWidget(m_editorTabs);

        // Preview (right panel) - placeholder for now
        m_previewWidget = new PreviewWidget(this);
        m_mainSplitter->addWidget(m_previewWidget);

        // Set splitter proportions: 1:2:2 (tree:editor:preview)
        m_mainSplitter->setStretchFactor(0, 1);
        m_mainSplitter->setStretchFactor(1, 2);
        m_mainSplitter->setStretchFactor(2, 2);

        setCentralWidget(m_mainSplitter);

        // Log dock (bottom)
        m_logDock = new QDockWidget(tr("Build Log"), this);
        QTextEdit *logWidget = new QTextEdit(this);
        logWidget->setReadOnly(true);
        m_logDock->setWidget(logWidget);
        addDockWidget(Qt::BottomDockWidgetArea, m_logDock);
        m_logDock->hide(); // Hidden by default
    }

    void MainWindow::createActions()
    {
        // File menu
        m_actionNewProject = new QAction(tr("&New Project..."), this);
        m_actionNewProject->setShortcut(QKeySequence::New);
        m_actionNewProject->setStatusTip(tr("Create a new TexLoom project"));
        connect(m_actionNewProject, &QAction::triggered, this, &MainWindow::onNewProject);

        m_actionOpenProject = new QAction(tr("&Open Project..."), this);
        m_actionOpenProject->setShortcut(QKeySequence::Open);
        m_actionOpenProject->setStatusTip(tr("Open an existing project"));
        connect(m_actionOpenProject, &QAction::triggered, this, &MainWindow::onOpenProject);

        m_actionSaveProject = new QAction(tr("&Save Project"), this);
        m_actionSaveProject->setShortcut(QKeySequence::Save);
        m_actionSaveProject->setStatusTip(tr("Save the current project"));
        connect(m_actionSaveProject, &QAction::triggered, this, &MainWindow::onSaveProject);

        m_actionSaveProjectAs = new QAction(tr("Save Project &As..."), this);
        m_actionSaveProjectAs->setShortcut(QKeySequence::SaveAs);
        connect(m_actionSaveProjectAs, &QAction::triggered, this, &MainWindow::onSaveProjectAs);

        m_actionCloseProject = new QAction(tr("&Close Project"), this);
        m_actionCloseProject->setShortcut(QKeySequence::Close);
        connect(m_actionCloseProject, &QAction::triggered, this, &MainWindow::onCloseProject);

        m_actionQuit = new QAction(tr("&Quit"), this);
        m_actionQuit->setShortcut(QKeySequence::Quit);
        connect(m_actionQuit, &QAction::triggered, this, &MainWindow::onQuit);

        // Edit menu
        m_actionUndo = new QAction(tr("&Undo"), this);
        m_actionUndo->setShortcut(QKeySequence::Undo);
        connect(m_actionUndo, &QAction::triggered, this, &MainWindow::onUndo);

        m_actionRedo = new QAction(tr("&Redo"), this);
        m_actionRedo->setShortcut(QKeySequence::Redo);
        connect(m_actionRedo, &QAction::triggered, this, &MainWindow::onRedo);

        m_actionFind = new QAction(tr("&Find..."), this);
        m_actionFind->setShortcut(QKeySequence::Find);
        connect(m_actionFind, &QAction::triggered, this, &MainWindow::onFind);

        // View menu
        m_actionToggleProjectTree = new QAction(tr("Project Tree"), this);
        m_actionToggleProjectTree->setShortcut(Qt::CTRL | Qt::Key_1);
        m_actionToggleProjectTree->setCheckable(true);
        m_actionToggleProjectTree->setChecked(true);
        connect(m_actionToggleProjectTree, &QAction::triggered, this, &MainWindow::onToggleProjectTree);

        m_actionTogglePreview = new QAction(tr("Preview Panel"), this);
        m_actionTogglePreview->setShortcut(Qt::CTRL | Qt::Key_2);
        m_actionTogglePreview->setCheckable(true);
        m_actionTogglePreview->setChecked(true);
        connect(m_actionTogglePreview, &QAction::triggered, this, &MainWindow::onTogglePreview);

        m_actionToggleLog = new QAction(tr("Build Log"), this);
        m_actionToggleLog->setShortcut(Qt::CTRL | Qt::Key_3);
        m_actionToggleLog->setCheckable(true);
        connect(m_actionToggleLog, &QAction::triggered, this, &MainWindow::onToggleLog);

        m_actionEditorModeCode = new QAction(tr("Editor Mode: Code"), this);
        m_actionEditorModeCode->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_M);
        connect(m_actionEditorModeCode, &QAction::triggered, this, &MainWindow::onEditorModeCode);

        m_actionEditorModeWysiwyg = new QAction(tr("Editor Mode: WYSIWYG"), this);
        m_actionEditorModeWysiwyg->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_W);
        connect(m_actionEditorModeWysiwyg, &QAction::triggered, this, &MainWindow::onEditorModeWysiwyg);

        // Build menu
        m_actionConvertToLatex = new QAction(tr("Convert to LaTeX"), this);
        m_actionConvertToLatex->setShortcut(Qt::CTRL | Qt::Key_B);
        connect(m_actionConvertToLatex, &QAction::triggered, this, &MainWindow::onConvertToLatex);

        m_actionCompilePdf = new QAction(tr("Compile PDF"), this);
        m_actionCompilePdf->setShortcut(Qt::Key_F5);
        connect(m_actionCompilePdf, &QAction::triggered, this, &MainWindow::onCompilePdf);

        m_actionCompileAndPreview = new QAction(tr("Compile and Preview"), this);
        m_actionCompileAndPreview->setShortcut(Qt::CTRL | Qt::Key_F5);
        connect(m_actionCompileAndPreview, &QAction::triggered, this, &MainWindow::onCompileAndPreview);

        m_actionBuildSettings = new QAction(tr("Build Settings..."), this);
        connect(m_actionBuildSettings, &QAction::triggered, this, &MainWindow::onBuildSettings);
    }

    void MainWindow::createMenus()
    {
        // File menu
        QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
        fileMenu->addAction(m_actionNewProject);
        fileMenu->addAction(m_actionOpenProject);
        fileMenu->addSeparator();
        fileMenu->addAction(m_actionSaveProject);
        fileMenu->addAction(m_actionSaveProjectAs);
        fileMenu->addAction(m_actionCloseProject);
        fileMenu->addSeparator();
        fileMenu->addAction(m_actionQuit);

        // Edit menu
        QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
        editMenu->addAction(m_actionUndo);
        editMenu->addAction(m_actionRedo);
        editMenu->addSeparator();
        editMenu->addAction(m_actionFind);

        // View menu
        QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
        viewMenu->addAction(m_actionToggleProjectTree);
        viewMenu->addAction(m_actionTogglePreview);
        viewMenu->addAction(m_actionToggleLog);
        viewMenu->addSeparator();
        viewMenu->addAction(m_actionEditorModeCode);
        viewMenu->addAction(m_actionEditorModeWysiwyg);

        // Build menu
        QMenu *buildMenu = menuBar()->addMenu(tr("&Build"));
        buildMenu->addAction(m_actionConvertToLatex);
        buildMenu->addAction(m_actionCompilePdf);
        buildMenu->addAction(m_actionCompileAndPreview);
        buildMenu->addSeparator();
        buildMenu->addAction(m_actionBuildSettings);

        // Help menu
        QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
        helpMenu->addAction(tr("&About TexLoom"), this, [this]()
                            { QMessageBox::about(this, tr("About TexLoom"),
                                                 tr("TexLoom - Markdown to LaTeX Converter\nVersion 0.1.0")); });
    }

    void MainWindow::createToolbar()
    {
        QToolBar *toolbar = addToolBar(tr("Main Toolbar"));
        toolbar->addAction(m_actionNewProject);
        toolbar->addAction(m_actionOpenProject);
        toolbar->addAction(m_actionSaveProject);
        toolbar->addSeparator();
        toolbar->addAction(m_actionConvertToLatex);
        toolbar->addAction(m_actionCompilePdf);
        toolbar->addAction(m_actionCompileAndPreview);
    }

    void MainWindow::createStatusBar()
    {
        statusBar()->showMessage(tr("Ready"));
    }

    void MainWindow::updateWindowTitle()
    {
        QString title = "TexLoom";

        if (m_projectModel->isOpen())
        {
            title = m_projectModel->projectName();
            if (m_projectModel->isModified())
            {
                title += " *";
            }
            title += " - TexLoom";
        }

        setWindowTitle(title);
    }

    void MainWindow::updateActions()
    {
        bool projectOpen = m_projectModel->isOpen();
        bool projectModified = m_projectModel->isModified();
        bool conversionBusy = m_conversionEngine->isBusy();

        m_actionSaveProject->setEnabled(projectOpen && projectModified);
        m_actionSaveProjectAs->setEnabled(projectOpen);
        m_actionCloseProject->setEnabled(projectOpen);

        m_actionConvertToLatex->setEnabled(projectOpen && !conversionBusy);
        m_actionCompilePdf->setEnabled(projectOpen && !conversionBusy);
        m_actionCompileAndPreview->setEnabled(projectOpen && !conversionBusy);
    }

    bool MainWindow::maybeSave()
    {
        if (!m_projectModel->isModified())
        {
            return true;
        }

        QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("TexLoom"),
                                                               tr("The project has been modified.\nDo you want to save your changes?"),
                                                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (ret == QMessageBox::Save)
        {
            onSaveProject();
            return true;
        }
        else if (ret == QMessageBox::Cancel)
        {
            return false;
        }

        return true;
    }

    void MainWindow::closeEvent(QCloseEvent *event)
    {
        if (maybeSave())
        {
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }

    // Slots implementation

    void MainWindow::onNewProject()
    {
        if (!maybeSave())
            return;

        QString dir = QFileDialog::getExistingDirectory(this, tr("Select Project Directory"));
        if (dir.isEmpty())
            return;

        QString name = QFileInfo(dir).fileName();
        if (m_projectModel->isOpen())
            m_projectModel->closeProject();

        if (m_projectModel->createProject(name, dir))
        {
            statusBar()->showMessage(tr("Project created: %1").arg(name), 3000);
        }
    }

    void MainWindow::onOpenProject()
    {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open Project"), QString(), tr("TexLoom Projects (*.texloom)"));

        if (!fileName.isEmpty())
        {
            if (m_projectModel->loadProject(fileName))
            {
                statusBar()->showMessage(tr("Project opened: %1").arg(fileName), 3000);
            }
        }
    }

    void MainWindow::onSaveProject()
    {
        if (m_projectModel->saveProject())
        {
            updateWindowTitle();
            updateActions();
            statusBar()->showMessage(tr("Project saved"), 3000);
        }
    }

    void MainWindow::onSaveProjectAs()
    {
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save Project As"), QString(), tr("TexLoom Projects (*.texloom)"));

        if (!fileName.isEmpty())
        {
            if (m_projectModel->saveProjectAs(fileName))
            {
                statusBar()->showMessage(tr("Project saved as: %1").arg(fileName), 3000);
            }
        }
    }

    void MainWindow::onCloseProject()
    {
        if (maybeSave())
        {
            m_projectModel->closeProject();
        }
    }

    void MainWindow::onQuit()
    {
        close();
    }

    void MainWindow::onUndo()
    {
        auto *editor = qobject_cast<EditorWidget *>(m_editorTabs->currentWidget());
        if (editor)
            editor->undo();
    }

    void MainWindow::onRedo()
    {
        auto *editor = qobject_cast<EditorWidget *>(m_editorTabs->currentWidget());
        if (editor)
            editor->redo();
    }

    void MainWindow::onFind()
    {
        statusBar()->showMessage(tr("Find - not yet implemented"), 3000);
    }

    void MainWindow::onToggleProjectTree()
    {
        m_projectTree->setVisible(m_actionToggleProjectTree->isChecked());
    }

    void MainWindow::onTogglePreview()
    {
        m_previewWidget->setVisible(m_actionTogglePreview->isChecked());
    }

    void MainWindow::onToggleLog()
    {
        if (m_actionToggleLog->isChecked())
        {
            m_logDock->show();
        }
        else
        {
            m_logDock->hide();
        }
    }

    void MainWindow::onEditorModeCode()
    {
        auto *editor = qobject_cast<EditorWidget *>(m_editorTabs->currentWidget());
        if (editor)
            editor->setEditorMode(EditorWidget::Mode::Code);
    }

    void MainWindow::onEditorModeWysiwyg()
    {
        auto *editor = qobject_cast<EditorWidget *>(m_editorTabs->currentWidget());
        if (editor)
            editor->setEditorMode(EditorWidget::Mode::Wysiwyg);
    }

    void MainWindow::onConvertToLatex()
    {
        auto *editor = qobject_cast<EditorWidget *>(m_editorTabs->currentWidget());
        if (!editor || editor->currentFile().isEmpty())
        {
            statusBar()->showMessage(tr("No file open to convert"), 3000);
            return;
        }

        QString mdFile = editor->currentFile();
        QString latexFile = QFileInfo(mdFile).absolutePath() + "/" +
                            QFileInfo(mdFile).baseName() + ".tex";
        m_conversionEngine->convertToLatex(mdFile, latexFile);
    }

    void MainWindow::onCompilePdf()
    {
        auto *editor = qobject_cast<EditorWidget *>(m_editorTabs->currentWidget());
        if (!editor || editor->currentFile().isEmpty())
        {
            statusBar()->showMessage(tr("No file open to compile"), 3000);
            return;
        }

        QString mdFile = editor->currentFile();
        QString pdfFile = QFileInfo(mdFile).absolutePath() + "/" +
                          QFileInfo(mdFile).baseName() + ".pdf";
        m_conversionEngine->convertAll(mdFile, pdfFile);
    }

    void MainWindow::onCompileAndPreview()
    {
        onCompilePdf();
    }

    void MainWindow::onBuildSettings()
    {
        statusBar()->showMessage(tr("Build Settings - not yet implemented"), 3000);
    }

    void MainWindow::onProjectOpened(const QString &path)
    {
        m_projectTree->setProjectRoot(m_projectModel->projectName(), path);
        for (const QString &file : m_projectModel->files())
        {
            m_projectTree->addFile(file);
        }

        updateWindowTitle();
        updateActions();
        statusBar()->showMessage(tr("Project opened: %1").arg(path), 3000);
    }

    void MainWindow::onProjectClosed()
    {
        // Close all editor tabs
        while (m_editorTabs->count() > 0)
        {
            QWidget *w = m_editorTabs->widget(0);
            m_editorTabs->removeTab(0);
            delete w;
        }

        m_projectTree->clear();
        m_previewWidget->clear();

        updateWindowTitle();
        updateActions();
        statusBar()->showMessage(tr("Project closed"), 3000);
    }

    void MainWindow::onProjectModified()
    {
        updateWindowTitle();
        updateActions();
    }

    void MainWindow::onFileSelected(const QString &filePath)
    {
        statusBar()->showMessage(filePath, 3000);
    }

    void MainWindow::onFileDoubleClicked(const QString &filePath)
    {
        // Check if already open in a tab
        for (int i = 0; i < m_editorTabs->count(); ++i)
        {
            auto *editor = qobject_cast<EditorWidget *>(m_editorTabs->widget(i));
            if (editor && editor->currentFile() == filePath)
            {
                m_editorTabs->setCurrentIndex(i);
                return;
            }
        }

        // Open in new tab
        auto *editor = new EditorWidget(this);
        if (editor->loadFile(filePath))
        {
            QFileInfo fi(filePath);
            int idx = m_editorTabs->addTab(editor, fi.fileName());
            m_editorTabs->setCurrentIndex(idx);

            connect(editor, &EditorWidget::fileModified, this, &MainWindow::onEditorModified);
            connect(editor, &EditorWidget::modeChanged, this, [this](EditorWidget::Mode mode)
                    {
                QString modeName = (mode == EditorWidget::Mode::Code) ? tr("Code") : tr("WYSIWYG");
                statusBar()->showMessage(tr("Editor mode: %1").arg(modeName), 3000); });
        }
        else
        {
            delete editor;
            statusBar()->showMessage(tr("Cannot open file: %1").arg(filePath), 5000);
        }
    }

    void MainWindow::onConversionStarted()
    {
        updateActions();
        m_logDock->show();
    }

    void MainWindow::onEditorModified(bool modified)
    {
        auto *editor = qobject_cast<EditorWidget *>(sender());
        if (!editor)
            return;

        int idx = m_editorTabs->indexOf(editor);
        if (idx < 0)
            return;

        QString title = QFileInfo(editor->currentFile()).fileName();
        if (modified)
            title += " *";
        m_editorTabs->setTabText(idx, title);
    }

    void MainWindow::onTabCloseRequested(int index)
    {
        auto *editor = qobject_cast<EditorWidget *>(m_editorTabs->widget(index));
        if (!editor)
            return;

        if (editor->isModified())
        {
            QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("TexLoom"),
                                                                   tr("The file has been modified.\nDo you want to save your changes?"),
                                                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

            if (ret == QMessageBox::Save)
                editor->saveFile();
            else if (ret == QMessageBox::Cancel)
                return;
        }

        m_editorTabs->removeTab(index);
        delete editor;
    }

    void MainWindow::onConversionProgress(const QString &message)
    {
        QTextEdit *log = qobject_cast<QTextEdit *>(m_logDock->widget());
        if (log)
        {
            log->append(message);
        }
    }

    void MainWindow::onConversionCompleted(const QString &output)
    {
        updateActions();
        statusBar()->showMessage(tr("Conversion completed: %1").arg(output), 5000);
    }

    void MainWindow::onConversionFailed(const QString &error)
    {
        updateActions();
        statusBar()->showMessage(tr("Conversion failed: %1").arg(error), 5000);
        QMessageBox::warning(this, tr("Conversion Error"), error);
    }

} // namespace texloom
