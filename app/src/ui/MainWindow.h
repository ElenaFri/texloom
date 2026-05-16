#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include <QDockWidget>
#include <QMap>

namespace texloom
{

    class ProjectModel;
    class ConversionEngine;
    class ProjectTreeWidget;
    class EditorWidget;
    class PreviewWidget;

    /**
     * @brief Main application window
     *
     * Manages the overall UI layout with:
     * - Project tree (left panel)
     * - Editor tabs (center)
     * - PDF preview (right panel)
     * - Build log (bottom dock)
     * - Menu bar and toolbar
     */
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow() override;

        // Non-copyable and non-movable
        MainWindow(const MainWindow &) = delete;
        MainWindow &operator=(const MainWindow &) = delete;
        MainWindow(MainWindow &&) = delete;
        MainWindow &operator=(MainWindow &&) = delete;

    protected:
        void closeEvent(QCloseEvent *event) override;

    private slots:
        // File menu
        void onNewProject();
        void onOpenProject();
        void onSaveProject();
        void onSaveProjectAs();
        void onCloseProject();
        void onQuit();
        void onNewMarkdownFile();
        void onAddExistingFile();
        void onRemoveFile();

        // Edit menu
        void onUndo();
        void onRedo();
        void onFind();

        // View menu
        void onToggleProjectTree();
        void onTogglePreview();
        void onToggleLog();
        void onEditorModeCode();
        void onEditorModeWysiwyg();

        // Build menu
        void onConvertToLatex();
        void onCompilePdf();
        void onCompileAndPreview();
        void onBuildSettings();

        // Project model signals
        void onProjectOpened(const QString &path);
        void onProjectClosed();
        void onProjectModified();

        // Project tree signals
        void onFileSelected(const QString &filePath);
        void onFileDoubleClicked(const QString &filePath);

        // Editor tab signals
        void onEditorModified(bool modified);
        void onTabCloseRequested(int index);

        // Conversion engine signals
        void onConversionStarted();
        void onConversionProgress(const QString &message);
        void onConversionCompleted(const QString &output);
        void onConversionFailed(const QString &error);

        // Internal helpers exposed as slots for deterministic tests
        void onAddFileToProject(const QString &filePath);
        void onCreateMarkdownFileAtPath(const QString &filePath);

    private:
        void setupUi();
        void createActions();
        void createMenus();
        void createToolbar();
        void createStatusBar();
        void updateWindowTitle();
        void updateActions();
        void applyTexLoomAppearance();
        QStringList availableGtkThemes() const;
        void applyQtStyleTheme(const QString &styleName);
        void applyGtkTheme(const QString &themeName);
        void saveThemePreference(const QString &themeType, const QString &themeValue);
        void applySavedThemeOrSystemDefault();
        void refreshOpenEditorSyntaxThemes();

        bool maybeSave();

        // Apply the project's selected template to the conversion engine.
        // Resolves the .latex file relative to the application binary and sets
        // it on the engine; clears the template path when none is configured or
        // the file cannot be found.
        void applyProjectTemplate();

        // Core components
        ProjectModel *m_projectModel = nullptr;
        ConversionEngine *m_conversionEngine = nullptr;

        // UI components
        ProjectTreeWidget *m_projectTree = nullptr;
        QTabWidget *m_editorTabs = nullptr;
        PreviewWidget *m_previewWidget = nullptr;
        QDockWidget *m_logDock = nullptr;
        QMap<QString, EditorWidget *> m_openEditors;

        // Layout
        QSplitter *m_mainSplitter = nullptr;
        QString m_systemStyleName;

        // Actions - File menu
        QAction *m_actionNewProject = nullptr;
        QAction *m_actionOpenProject = nullptr;
        QAction *m_actionSaveProject = nullptr;
        QAction *m_actionSaveProjectAs = nullptr;
        QAction *m_actionCloseProject = nullptr;
        QAction *m_actionQuit = nullptr;
        QAction *m_actionNewMarkdownFile = nullptr;
        QAction *m_actionAddExistingFile = nullptr;
        QAction *m_actionRemoveFile = nullptr;

        // Actions - Edit menu
        QAction *m_actionUndo = nullptr;
        QAction *m_actionRedo = nullptr;
        QAction *m_actionFind = nullptr;

        // Actions - View menu
        QAction *m_actionToggleProjectTree = nullptr;
        QAction *m_actionTogglePreview = nullptr;
        QAction *m_actionToggleLog = nullptr;
        QAction *m_actionEditorModeCode = nullptr;
        QAction *m_actionEditorModeWysiwyg = nullptr;

        // Actions - Build menu
        QAction *m_actionConvertToLatex = nullptr;
        QAction *m_actionCompilePdf = nullptr;
        QAction *m_actionCompileAndPreview = nullptr;
        QAction *m_actionBuildSettings = nullptr;
    };

} // namespace texloom
