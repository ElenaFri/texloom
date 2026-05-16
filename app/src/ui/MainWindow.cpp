#include "MainWindow.h"
#include "NewProjectDialog.h"
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
#include <QInputDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QPushButton>
#include <QToolButton>
#include <QApplication>
#include <QStyleFactory>
#include <QMenu>
#include <QSet>
#include <QSettings>
#include <QProcess>

namespace texloom
{

    MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
    {
        m_systemStyleName = QApplication::style()->objectName();

        // Create core components
        m_projectModel = new ProjectModel(this);
        m_conversionEngine = new ConversionEngine(this);

        // Setup UI
        setupUi();
        createActions();
        createMenus();
        createToolbar();
        createStatusBar();
        applySavedThemeOrSystemDefault();

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

        // Left sidebar with project tree and status card
        auto *leftSidebar = new QWidget(this);
        auto *leftLayout = new QVBoxLayout(leftSidebar);
        leftLayout->setContentsMargins(0, 0, 0, 0);
        leftLayout->setSpacing(12);

        auto *projectCard = new QFrame(leftSidebar);
        projectCard->setObjectName("panelCard");
        auto *projectLayout = new QVBoxLayout(projectCard);
        projectLayout->setContentsMargins(12, 12, 12, 12);
        projectLayout->setSpacing(8);

        auto *projectTitle = new QLabel(tr("Project"), projectCard);
        projectTitle->setObjectName("panelTitle");
        projectLayout->addWidget(projectTitle);

        m_projectTree = new ProjectTreeWidget(this);
        m_projectTree->setContextMenuPolicy(Qt::ActionsContextMenu);
        projectLayout->addWidget(m_projectTree, 1);

        leftLayout->addWidget(projectCard, 1);

        auto *statusCard = new QFrame(leftSidebar);
        statusCard->setObjectName("panelCard");
        auto *statusLayout = new QVBoxLayout(statusCard);
        statusLayout->setContentsMargins(12, 12, 12, 12);
        statusLayout->setSpacing(10);

        auto *statusTitle = new QLabel(tr("Status"), statusCard);
        statusTitle->setObjectName("panelTitle");
        statusLayout->addWidget(statusTitle);

        auto *statusGrid = new QGridLayout();
        statusGrid->setContentsMargins(0, 0, 0, 0);
        statusGrid->setHorizontalSpacing(8);
        statusGrid->setVerticalSpacing(8);
        statusGrid->setColumnStretch(0, 1);
        statusGrid->setColumnStretch(1, 0);

        auto *engineKey = new QLabel(tr("LaTeX engine"), statusCard);
        auto *engineValue = new QLabel(tr("pdfLaTeX"), statusCard);
        engineValue->setObjectName("statusValue");
        engineValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        auto *compileKey = new QLabel(tr("Compilation"), statusCard);
        auto *compileValue = new QLabel(tr("No errors"), statusCard);
        compileValue->setObjectName("statusValue");
        compileValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        auto *lastCompileKey = new QLabel(tr("Last compilation"), statusCard);
        auto *lastCompileValue = new QLabel(tr("1 minute ago"), statusCard);
        lastCompileValue->setObjectName("statusValue");
        lastCompileValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        statusGrid->addWidget(engineKey, 0, 0);
        statusGrid->addWidget(engineValue, 0, 1);
        statusGrid->addWidget(compileKey, 1, 0);
        statusGrid->addWidget(compileValue, 1, 1);
        statusGrid->addWidget(lastCompileKey, 2, 0);
        statusGrid->addWidget(lastCompileValue, 2, 1);

        statusLayout->addLayout(statusGrid);

        auto *openPdfButton = new QPushButton(tr("Open PDF"), statusCard);
        openPdfButton->setObjectName("primaryButton");
        openPdfButton->setMinimumHeight(36);
        statusLayout->addWidget(openPdfButton);

        leftLayout->addWidget(statusCard, 0);

        m_mainSplitter->addWidget(leftSidebar);

        // Editor panel (center)
        auto *editorPanel = new QFrame(this);
        editorPanel->setObjectName("sectionCard");
        auto *editorLayout = new QVBoxLayout(editorPanel);
        editorLayout->setContentsMargins(8, 8, 8, 8);
        editorLayout->setSpacing(8);

        auto *editorControls = new QToolBar(editorPanel);
        editorControls->setMovable(false);
        editorControls->setFloatable(false);
        editorControls->setIconSize(QSize(16, 16));
        editorControls->addAction(tr("B"));
        editorControls->addAction(tr("I"));
        editorControls->addAction(tr("H"));
        editorControls->addSeparator();
        editorControls->addAction(tr("List"));
        editorControls->addAction(tr("Link"));
        editorControls->addAction(tr("Code"));
        editorControls->addAction(tr("Image"));
        editorLayout->addWidget(editorControls);

        m_editorTabs = new QTabWidget(editorPanel);
        m_editorTabs->setTabsClosable(true);
        m_editorTabs->setMovable(true);
        connect(m_editorTabs, &QTabWidget::tabCloseRequested,
                this, &MainWindow::onTabCloseRequested);
        editorLayout->addWidget(m_editorTabs, 1);

        m_mainSplitter->addWidget(editorPanel);

        // Preview panel (right)
        auto *previewPanel = new QFrame(this);
        previewPanel->setObjectName("sectionCard");
        auto *previewLayout = new QVBoxLayout(previewPanel);
        previewLayout->setContentsMargins(8, 8, 8, 8);
        previewLayout->setSpacing(8);

        auto *previewTitle = new QLabel(tr("PDF Preview"), previewPanel);
        previewTitle->setObjectName("panelTitle");
        previewLayout->addWidget(previewTitle);

        m_previewWidget = new PreviewWidget(previewPanel);
        previewLayout->addWidget(m_previewWidget, 1);

        m_mainSplitter->addWidget(previewPanel);

        // Set splitter proportions close to the mockup
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

        m_actionNewMarkdownFile = new QAction(tr("New Markdown File"), this);
        m_actionNewMarkdownFile->setShortcut(Qt::CTRL | Qt::ALT | Qt::Key_N);
        connect(m_actionNewMarkdownFile, &QAction::triggered, this, &MainWindow::onNewMarkdownFile);

        m_actionAddExistingFile = new QAction(tr("Add Existing File..."), this);
        connect(m_actionAddExistingFile, &QAction::triggered, this, &MainWindow::onAddExistingFile);

        m_actionRemoveFile = new QAction(tr("Remove File"), this);
        connect(m_actionRemoveFile, &QAction::triggered, this, &MainWindow::onRemoveFile);

        // Project tree context menu
        m_projectTree->addAction(m_actionNewMarkdownFile);
        m_projectTree->addAction(m_actionAddExistingFile);
        m_projectTree->addAction(m_actionRemoveFile);

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
        fileMenu->addAction(m_actionNewMarkdownFile);
        fileMenu->addAction(m_actionAddExistingFile);
        fileMenu->addAction(m_actionRemoveFile);
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
        toolbar->setMovable(false);
        toolbar->setFloatable(false);
        toolbar->setIconSize(QSize(16, 16));

        toolbar->addAction(m_actionNewProject);
        toolbar->addAction(m_actionOpenProject);
        toolbar->addAction(m_actionSaveProject);
        toolbar->addAction(m_actionNewMarkdownFile);
        toolbar->addSeparator();
        toolbar->addAction(m_actionConvertToLatex);
        toolbar->addAction(m_actionCompilePdf);
        toolbar->addAction(m_actionCompileAndPreview);

        auto *spacer = new QWidget(toolbar);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        toolbar->addWidget(spacer);

        auto *themeButton = new QToolButton(toolbar);
        themeButton->setText(tr("Theme"));
        themeButton->setPopupMode(QToolButton::InstantPopup);

        auto *themeMenu = new QMenu(themeButton);
        themeButton->setMenu(themeMenu);

        auto *systemTheme = themeMenu->addAction(tr("System Default"));
        connect(systemTheme, &QAction::triggered, this, [this]()
                {
            applyQtStyleTheme(m_systemStyleName);
                setStyleSheet(QString());
                refreshOpenEditorSyntaxThemes();
                saveThemePreference(QStringLiteral("system"), QString());
                statusBar()->showMessage(tr("Theme applied: System Default"), 3000); });

        themeMenu->addSeparator();

        auto *texloomTheme = themeMenu->addAction(tr("TexLoom Light"));
        connect(texloomTheme, &QAction::triggered, this, [this]()
                {
                    applyQtStyleTheme(QStringLiteral("Fusion"));
                    applyTexLoomAppearance();
                refreshOpenEditorSyntaxThemes();
                saveThemePreference(QStringLiteral("texloom"), QStringLiteral("light"));
                    statusBar()->showMessage(tr("Theme applied: TexLoom Light"), 3000); });

        themeMenu->addSeparator();

        QMenu *qtThemesMenu = themeMenu->addMenu(tr("Qt Styles"));
        const QStringList qtStyles = QStyleFactory::keys();
        for (const QString &styleName : qtStyles)
        {
            QAction *styleAction = qtThemesMenu->addAction(styleName);
            connect(styleAction, &QAction::triggered, this, [this, styleName]()
                    {
                        applyQtStyleTheme(styleName);
                        this->setStyleSheet(QString());
                        refreshOpenEditorSyntaxThemes();
                        saveThemePreference(QStringLiteral("qt"), styleName);
                        statusBar()->showMessage(tr("Qt style applied: %1").arg(styleName), 3000); });
        }

        QMenu *gtkThemesMenu = themeMenu->addMenu(tr("GTK Themes"));
        const QStringList gtkThemes = availableGtkThemes();
        if (gtkThemes.isEmpty())
        {
            QAction *noThemeAction = gtkThemesMenu->addAction(tr("No GTK theme found"));
            noThemeAction->setEnabled(false);
        }
        else
        {
            for (const QString &themeName : gtkThemes)
            {
                QAction *themeAction = gtkThemesMenu->addAction(themeName);
                connect(themeAction, &QAction::triggered, this, [this, themeName]()
                        {
                            saveThemePreference(QStringLiteral("gtk"), themeName);

                            const auto restart = QMessageBox::question(
                                this,
                                tr("Restart Required"),
                                tr("Applying a GTK theme requires restarting TexLoom.\nRestart now?"),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::Yes);

                            if (restart == QMessageBox::Yes)
                            {
                                QStringList args = QCoreApplication::arguments();
                                if (!args.isEmpty())
                                {
                                    args.removeFirst();
                                }

                                QProcess::startDetached(QCoreApplication::applicationFilePath(), args);
                                QCoreApplication::quit();
                                return;
                            }

                            statusBar()->showMessage(tr("GTK theme will be applied on next launch: %1").arg(themeName), 4000); });
            }
        }

        toolbar->addWidget(themeButton);

        auto *settingsButton = new QToolButton(toolbar);
        settingsButton->setText(tr("Settings"));
        connect(settingsButton, &QToolButton::clicked, this, &MainWindow::onBuildSettings);
        toolbar->addWidget(settingsButton);
    }

    void MainWindow::applyTexLoomAppearance()
    {
        const QPalette palette = QApplication::palette();
        const QString windowColor = palette.color(QPalette::Window).name();
        const QString barColor = palette.color(QPalette::AlternateBase).name();
        const QString borderColor = palette.color(QPalette::Mid).name();
        const QString textColor = palette.color(QPalette::WindowText).name();
        const QString mutedTextColor = palette.color(QPalette::Mid).name();
        const QString baseColor = palette.color(QPalette::Base).name();
        const QString accentColor = palette.color(QPalette::Highlight).name();

        setStyleSheet(
            QStringLiteral("QMainWindow { background: %1; }"
                           "QMenuBar { background: %2; border-bottom: 1px solid %3; }"
                           "QMenuBar::item { padding: 8px 10px; color: %4; }"
                           "QToolBar { background: %2; border: none; border-bottom: 1px solid %3; spacing: 6px; padding: 6px 8px; }"
                           "QToolButton { background: %5; border: 1px solid %3; border-radius: 6px; padding: 7px 12px; color: %4; }"
                           "QToolButton:hover { background: %2; }"
                           "QToolButton:pressed { background: %2; }"
                           "QToolButton:disabled { color: %6; background: %2; }"
                           "QPushButton#primaryButton { background: %7; color: %5; border: none; border-radius: 6px; font-weight: 600; padding: 8px 12px; }"
                           "QPushButton#primaryButton:hover { background: %7; }"
                           "QTabWidget::pane { border: 1px solid %3; background: %5; border-radius: 8px; }"
                           "QTabBar::tab { background: %2; border: 1px solid %3; border-bottom: none; border-top: 3px solid transparent; padding: 8px 14px; margin-right: 4px; border-top-left-radius: 6px; border-top-right-radius: 6px; }"
                           "QTabBar::tab:selected { background: %5; color: %7; font-weight: 600; border-top: 3px solid %7; }"
                           "QFrame#panelCard { background: %5; border: 1px solid %3; border-radius: 8px; }"
                           "QFrame#sectionCard { background: %5; border: 1px solid %3; border-radius: 8px; }"
                           "QLabel#panelTitle { color: %4; font-size: 22px; font-weight: 600; padding: 6px 2px; }"
                           "QLabel#statusValue { color: %4; font-weight: 600; }"
                           "QStatusBar { background: %2; border-top: 1px solid %3; color: %6; }"
                           "QTreeWidget { border: none; background: transparent; }"
                           "QTreeView::branch { border-image: none; image: none; }"
                           "QTreeWidget::item { height: 28px; border-radius: 5px; }"
                           "QTreeWidget::item:selected { background: %7; color: %5; border: 1px solid %3; padding-left: 0; margin-left: 0; background-clip: content-box; }")
                .arg(windowColor,
                     barColor,
                     borderColor,
                     textColor,
                     baseColor,
                     mutedTextColor,
                     accentColor));
    }

    QStringList MainWindow::availableGtkThemes() const
    {
        const QStringList themeRoots = {
            QDir::homePath() + QStringLiteral("/.themes"),
            QStringLiteral("/usr/share/themes")};

        QSet<QString> themes;
        for (const QString &rootPath : themeRoots)
        {
            const QDir rootDir(rootPath);
            const QFileInfoList entries = rootDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QFileInfo &entry : entries)
            {
                const QString gtk3Path = entry.absoluteFilePath() + QStringLiteral("/gtk-3.0");
                const QString gtk4Path = entry.absoluteFilePath() + QStringLiteral("/gtk-4.0");
                if (QFileInfo::exists(gtk3Path) || QFileInfo::exists(gtk4Path))
                {
                    themes.insert(entry.fileName());
                }
            }
        }

        QStringList result = themes.values();
        result.sort(Qt::CaseInsensitive);
        return result;
    }

    void MainWindow::applyQtStyleTheme(const QString &styleName)
    {
        if (!QStyleFactory::keys().contains(styleName, Qt::CaseInsensitive))
        {
            return;
        }

        if (QStyle *style = QStyleFactory::create(styleName))
        {
            QApplication::setStyle(style);
        }
    }

    void MainWindow::applyGtkTheme(const QString &themeName)
    {
        qputenv("GTK_THEME", themeName.toUtf8());
    }

    void MainWindow::saveThemePreference(const QString &themeType, const QString &themeValue)
    {
        QSettings settings;
        settings.beginGroup(QStringLiteral("appearance"));
        settings.setValue(QStringLiteral("themeType"), themeType);
        settings.setValue(QStringLiteral("themeValue"), themeValue);
        settings.endGroup();
    }

    void MainWindow::applySavedThemeOrSystemDefault()
    {
        QSettings settings;
        settings.beginGroup(QStringLiteral("appearance"));
        const QString themeType = settings.value(QStringLiteral("themeType")).toString();
        const QString themeValue = settings.value(QStringLiteral("themeValue")).toString();
        settings.endGroup();

        // Default when user has never selected a theme: native system style.
        if (themeType.isEmpty() || themeType == QStringLiteral("system"))
        {
            applyQtStyleTheme(m_systemStyleName);
            setStyleSheet(QString());
            refreshOpenEditorSyntaxThemes();
            return;
        }

        if (themeType == QStringLiteral("texloom"))
        {
            applyQtStyleTheme(QStringLiteral("Fusion"));
            applyTexLoomAppearance();
            refreshOpenEditorSyntaxThemes();
            return;
        }

        if (themeType == QStringLiteral("qt"))
        {
            applyQtStyleTheme(themeValue);
            setStyleSheet(QString());
            refreshOpenEditorSyntaxThemes();
            return;
        }

        if (themeType == QStringLiteral("gtk"))
        {
            applyGtkTheme(themeValue);
            setStyleSheet(QString());
            refreshOpenEditorSyntaxThemes();
            return;
        }

        applyQtStyleTheme(m_systemStyleName);
        setStyleSheet(QString());
        refreshOpenEditorSyntaxThemes();
    }

    void MainWindow::refreshOpenEditorSyntaxThemes()
    {
        for (auto it = m_openEditors.begin(); it != m_openEditors.end(); ++it)
        {
            if (it.value())
            {
                it.value()->refreshSyntaxTheme();
            }
        }
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
        m_actionNewMarkdownFile->setEnabled(projectOpen);
        m_actionAddExistingFile->setEnabled(projectOpen);
        m_actionRemoveFile->setEnabled(projectOpen);

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

        QString templatesPath = QCoreApplication::applicationDirPath() + "/../../resources/templates";
        NewProjectDialog dialog(templatesPath, this);
        if (dialog.exec() != QDialog::Accepted)
            return;

        if (m_projectModel->isOpen())
            m_projectModel->closeProject();

        if (m_projectModel->createProject(dialog.projectName(), dialog.projectLocation()))
        {
            m_projectModel->setTemplateName(dialog.templateName());
            statusBar()->showMessage(tr("Project created: %1").arg(dialog.projectName()), 3000);
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

    void MainWindow::onNewMarkdownFile()
    {
        if (!m_projectModel->isOpen())
        {
            statusBar()->showMessage(tr("No project open"), 3000);
            return;
        }

        QString defaultPath = QDir(m_projectModel->projectPath()).filePath("new_file.md");
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("New Markdown File"),
                                                        defaultPath,
                                                        tr("Markdown Files (*.md)"));
        if (fileName.isEmpty())
            return;

        if (!fileName.endsWith(".md", Qt::CaseInsensitive))
            fileName += ".md";

        onCreateMarkdownFileAtPath(fileName);
    }

    void MainWindow::onAddExistingFile()
    {
        if (!m_projectModel->isOpen())
        {
            statusBar()->showMessage(tr("No project open"), 3000);
            return;
        }

        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Add Existing File"),
                                                        m_projectModel->projectPath(),
                                                        tr("Markdown Files (*.md)"));
        if (fileName.isEmpty())
            return;

        onAddFileToProject(fileName);
    }

    void MainWindow::onRemoveFile()
    {
        if (!m_projectModel->isOpen())
        {
            statusBar()->showMessage(tr("No project open"), 3000);
            return;
        }

        auto *item = m_projectTree->currentItem();
        if (!item)
        {
            statusBar()->showMessage(tr("No file selected"), 3000);
            return;
        }

        QString filePath = item->data(0, Qt::UserRole).toString();
        if (filePath.isEmpty())
        {
            statusBar()->showMessage(tr("Select a file to remove"), 3000);
            return;
        }

        if (m_openEditors.contains(filePath))
        {
            auto *editor = m_openEditors.value(filePath);
            int idx = m_editorTabs->indexOf(editor);
            if (idx >= 0)
                m_editorTabs->removeTab(idx);
            m_openEditors.remove(filePath);
            delete editor;
        }

        m_projectModel->removeFile(filePath);
        statusBar()->showMessage(tr("Removed file: %1").arg(QFileInfo(filePath).fileName()), 3000);
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

    void MainWindow::applyProjectTemplate()
    {
        const QString templateName = m_projectModel->templateName();
        if (templateName.isEmpty())
        {
            m_conversionEngine->setTemplatePath(QString{});
            return;
        }

        const QString templatePath = QDir(
                                         QCoreApplication::applicationDirPath() + "/../../resources/templates")
                                         .filePath(templateName + ".latex");

        m_conversionEngine->setTemplatePath(
            QFileInfo::exists(templatePath) ? templatePath : QString{});
    }

    void MainWindow::onConvertToLatex()
    {
        auto *editor = qobject_cast<EditorWidget *>(m_editorTabs->currentWidget());
        if (!editor || editor->currentFile().isEmpty())
        {
            statusBar()->showMessage(tr("No file open to convert"), 3000);
            return;
        }

        if (editor->isModified() && !editor->saveFile())
        {
            statusBar()->showMessage(tr("Cannot save file before converting"), 5000);
            return;
        }

        applyProjectTemplate();

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

        if (editor->isModified() && !editor->saveFile())
        {
            statusBar()->showMessage(tr("Cannot save file before compiling"), 5000);
            return;
        }

        applyProjectTemplate();

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
        m_openEditors.clear();
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
        // Check if already open via map
        if (m_openEditors.contains(filePath))
        {
            m_editorTabs->setCurrentWidget(m_openEditors.value(filePath));
            return;
        }

        // Open in new tab
        auto *editor = new EditorWidget(this);
        if (editor->loadFile(filePath))
        {
            QFileInfo fi(filePath);
            int idx = m_editorTabs->addTab(editor, fi.fileName());
            m_editorTabs->setCurrentIndex(idx);
            m_openEditors.insert(filePath, editor);
            statusBar()->showMessage(tr("Opened: %1").arg(fi.fileName()), 3000);

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
        m_actionToggleLog->setChecked(true);
        statusBar()->showMessage(tr("Converting..."));
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

        m_openEditors.remove(editor->currentFile());
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
        statusBar()->showMessage(message);
    }

    void MainWindow::onConversionCompleted(const QString &output)
    {
        updateActions();
        m_actionToggleLog->setChecked(false);
        m_logDock->hide();
        statusBar()->showMessage(tr("Conversion completed: %1").arg(output), 5000);
    }

    void MainWindow::onConversionFailed(const QString &error)
    {
        updateActions();
        statusBar()->showMessage(tr("Conversion failed: %1").arg(error), 5000);
        QMessageBox::warning(this, tr("Conversion Error"), error);
    }

    void MainWindow::onAddFileToProject(const QString &filePath)
    {
        if (!m_projectModel->isOpen())
        {
            statusBar()->showMessage(tr("No project open"), 3000);
            return;
        }

        QFileInfo fi(filePath);
        if (!fi.exists())
        {
            statusBar()->showMessage(tr("File does not exist: %1").arg(filePath), 5000);
            return;
        }

        m_projectModel->addFile(filePath);
        statusBar()->showMessage(tr("Added file: %1").arg(fi.fileName()), 3000);
    }

    void MainWindow::onCreateMarkdownFileAtPath(const QString &filePath)
    {
        if (!m_projectModel->isOpen())
        {
            statusBar()->showMessage(tr("No project open"), 3000);
            return;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            statusBar()->showMessage(tr("Cannot create file: %1").arg(filePath), 5000);
            return;
        }

        QTextStream out(&file);
        QString title = QFileInfo(filePath).completeBaseName();
        out << "# " << title << "\n\n";
        file.close();

        onAddFileToProject(filePath);
    }

} // namespace texloom
