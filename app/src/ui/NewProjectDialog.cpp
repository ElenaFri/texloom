#include "NewProjectDialog.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>

namespace texloom
{

    NewProjectDialog::NewProjectDialog(const QString &templatesPath, QWidget *parent)
        : QDialog(parent)
    {
        setWindowTitle(tr("New Project"));
        setMinimumWidth(450);
        setupUi();
        populateTemplates(templatesPath);
    }

    QString NewProjectDialog::projectName() const
    {
        return m_nameEdit->text().trimmed();
    }

    QString NewProjectDialog::projectLocation() const
    {
        return m_locationEdit->text().trimmed();
    }

    QString NewProjectDialog::templateName() const
    {
        return m_templateCombo->currentData().toString();
    }

    void NewProjectDialog::onBrowseLocation()
    {
        QString dir = QFileDialog::getExistingDirectory(
            this, tr("Select Project Location"), m_locationEdit->text());
        if (!dir.isEmpty())
        {
            m_locationEdit->setText(dir);
        }
    }

    void NewProjectDialog::validateInput()
    {
        bool valid = !m_nameEdit->text().trimmed().isEmpty() &&
                     !m_locationEdit->text().trimmed().isEmpty() &&
                     QDir(m_locationEdit->text().trimmed()).exists();
        m_createButton->setEnabled(valid);
    }

    void NewProjectDialog::setupUi()
    {
        // Name field
        m_nameEdit = new QLineEdit(this);
        m_nameEdit->setPlaceholderText(tr("My Project"));

        // Location field with browse button
        m_locationEdit = new QLineEdit(this);
        m_locationEdit->setPlaceholderText(tr("/home/user/projects"));
        m_browseButton = new QPushButton(tr("Browse..."), this);

        auto *locationLayout = new QHBoxLayout;
        locationLayout->addWidget(m_locationEdit);
        locationLayout->addWidget(m_browseButton);

        // Template selection
        m_templateCombo = new QComboBox(this);

        // Form layout
        auto *formLayout = new QFormLayout;
        formLayout->addRow(tr("Project &Name:"), m_nameEdit);
        formLayout->addRow(tr("&Location:"), locationLayout);
        formLayout->addRow(tr("&Template:"), m_templateCombo);

        // Buttons
        m_createButton = new QPushButton(tr("Create"), this);
        m_createButton->setDefault(true);
        m_createButton->setEnabled(false);
        m_cancelButton = new QPushButton(tr("Cancel"), this);

        auto *buttonLayout = new QHBoxLayout;
        buttonLayout->addStretch();
        buttonLayout->addWidget(m_createButton);
        buttonLayout->addWidget(m_cancelButton);

        // Main layout
        auto *mainLayout = new QVBoxLayout(this);
        mainLayout->addLayout(formLayout);
        mainLayout->addLayout(buttonLayout);

        // Connections
        connect(m_browseButton, &QPushButton::clicked, this, &NewProjectDialog::onBrowseLocation);
        connect(m_createButton, &QPushButton::clicked, this, &QDialog::accept);
        connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
        connect(m_nameEdit, &QLineEdit::textChanged, this, &NewProjectDialog::validateInput);
        connect(m_locationEdit, &QLineEdit::textChanged, this, &NewProjectDialog::validateInput);
    }

    void NewProjectDialog::populateTemplates(const QString &templatesPath)
    {
        QDir dir(templatesPath);
        QStringList filters;
        filters << "*.latex";
        QFileInfoList entries = dir.entryInfoList(filters, QDir::Files, QDir::Name);
        for (const QFileInfo &fi : entries)
        {
            QString baseName = fi.baseName();
            // Capitalize first letter for display
            QString displayName = baseName.at(0).toUpper() + baseName.mid(1);
            m_templateCombo->addItem(displayName, baseName);
        }
    }

} // namespace texloom
