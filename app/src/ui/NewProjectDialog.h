#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

namespace texloom
{

    class NewProjectDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit NewProjectDialog(const QString &templatesPath, QWidget *parent = nullptr);

        QString projectName() const;
        QString projectLocation() const;
        QString templateName() const;

    private slots:
        void onBrowseLocation();
        void validateInput();

    private:
        void setupUi();
        void populateTemplates(const QString &templatesPath);

        QLineEdit *m_nameEdit = nullptr;
        QLineEdit *m_locationEdit = nullptr;
        QPushButton *m_browseButton = nullptr;
        QComboBox *m_templateCombo = nullptr;
        QPushButton *m_createButton = nullptr;
        QPushButton *m_cancelButton = nullptr;
        QLabel *m_errorLabel = nullptr;
    };

} // namespace texloom
