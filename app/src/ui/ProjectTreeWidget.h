#pragma once

#include <QTreeWidget>

namespace texloom
{

    /**
     * @brief Project file tree widget
     *
     * Displays the project's file hierarchy.
     * Allows selecting, adding, and removing files.
     */
    class ProjectTreeWidget : public QTreeWidget
    {
        Q_OBJECT

    public:
        explicit ProjectTreeWidget(QWidget *parent = nullptr);
        ~ProjectTreeWidget() override = default;

        // Tree operations
        void setProjectRoot(const QString &name, const QString &path);
        void addFile(const QString &filePath);
        void removeFile(const QString &filePath);
        void clear();

    signals:
        void fileSelected(const QString &filePath);
        void fileDoubleClicked(const QString &filePath);

    private slots:
        void onItemClicked(QTreeWidgetItem *item, int column);
        void onItemDoubleClicked(QTreeWidgetItem *item, int column);

    private:
        QTreeWidgetItem *m_rootItem = nullptr;
        QString m_projectPath;
    };

} // namespace texloom
