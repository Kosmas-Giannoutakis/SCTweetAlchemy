#include "tweeteditdialog.h"
#include <QtWidgets> // Includes QLineEdit, QTextEdit, QDialogButtonBox, QVBoxLayout, QFormLayout, QMessageBox

TweetEditDialog::TweetEditDialog(Mode mode, QWidget *parent)
    : QDialog(parent), m_mode(mode)
{
    setupUi();

    if (m_mode == Mode::Add) {
        setWindowTitle("Add New SCTweet");
    } else {
        setWindowTitle("Edit SCTweet");
        m_idLineEdit->setReadOnly(true); // Typically, ID is not changed during edit
    }
    setMinimumSize(500, 400); // Set a reasonable minimum size
}

void TweetEditDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    m_idLineEdit = new QLineEdit();
    m_codeTextEdit = new QTextEdit();
    m_codeTextEdit->setAcceptRichText(false);
    m_codeTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)); // Good for code
    m_authorLineEdit = new QLineEdit();
    m_sourceUrlLineEdit = new QLineEdit();
    m_descriptionTextEdit = new QTextEdit(); // Changed to QTextEdit
    m_descriptionTextEdit->setAcceptRichText(false);
    m_publicationDateLineEdit = new QLineEdit();
    m_sonicTagsLineEdit = new QLineEdit();
    m_sonicTagsLineEdit->setPlaceholderText("Comma-separated, e.g., bass, glitch");
    m_techniqueTagsLineEdit = new QLineEdit();
    m_techniqueTagsLineEdit->setPlaceholderText("Comma-separated, e.g., feedback, filter");
    m_genericTagsLineEdit = new QLineEdit();
    m_genericTagsLineEdit->setPlaceholderText("Comma-separated");


    formLayout->addRow("ID:", m_idLineEdit);
    formLayout->addRow("Author:", m_authorLineEdit);
    formLayout->addRow("Source URL:", m_sourceUrlLineEdit);
    formLayout->addRow("Publication Date:", m_publicationDateLineEdit);
    formLayout->addRow("Description:", m_descriptionTextEdit);
    formLayout->addRow("Sonic Tags:", m_sonicTagsLineEdit);
    formLayout->addRow("Technique Tags:", m_techniqueTagsLineEdit);
    formLayout->addRow("Generic Tags:", m_genericTagsLineEdit);
    // Code is larger, add it separately or make form layout span columns for it
    
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(new QLabel("SuperCollider Code:"));
    mainLayout->addWidget(m_codeTextEdit, 1); // Give code editor stretch factor

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &TweetEditDialog::onAccept); // Connect to custom slot
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(m_buttonBox);

    setLayout(mainLayout);
}

void TweetEditDialog::setTweetData(const TweetData& data)
{
    m_originalTweetIdForEdit = data.id; // Store original ID for edit mode
    m_idLineEdit->setText(data.id);
    m_codeTextEdit->setPlainText(data.originalCode);
    m_authorLineEdit->setText(data.author);
    m_sourceUrlLineEdit->setText(data.sourceUrl);
    m_descriptionTextEdit->setPlainText(data.description);
    m_publicationDateLineEdit->setText(data.publicationDate);
    m_sonicTagsLineEdit->setText(data.sonicTags.join(", "));
    m_techniqueTagsLineEdit->setText(data.techniqueTags.join(", "));
    m_genericTagsLineEdit->setText(data.genericTags.join(", "));
}

TweetData TweetEditDialog::getTweetData() const
{
    TweetData data;
    data.id = m_idLineEdit->text().trimmed();
    data.originalCode = m_codeTextEdit->toPlainText();
    data.author = m_authorLineEdit->text().trimmed();
    data.sourceUrl = m_sourceUrlLineEdit->text().trimmed();
    data.description = m_descriptionTextEdit->toPlainText().trimmed();
    data.publicationDate = m_publicationDateLineEdit->text().trimmed();
    
    auto splitTags = [](const QString& s) {
        QStringList list = s.split(',', Qt::SkipEmptyParts);
        for(QString& tag : list) tag = tag.trimmed();
        return list;
    };

    data.sonicTags = splitTags(m_sonicTagsLineEdit->text());
    data.techniqueTags = splitTags(m_techniqueTagsLineEdit->text());
    data.genericTags = splitTags(m_genericTagsLineEdit->text());
    // UGens will be extracted by TweetRepository when adding/updating
    return data;
}

void TweetEditDialog::setExistingTweetIds(const QSet<QString>& ids)
{
    m_existingTweetIds = ids;
}

bool TweetEditDialog::validateInput()
{
    QString id = m_idLineEdit->text().trimmed();
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Tweet ID cannot be empty.");
        m_idLineEdit->setFocus();
        return false;
    }

    if (m_codeTextEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "SuperCollider code cannot be empty.");
        m_codeTextEdit->setFocus();
        return false;
    }

    if (m_mode == Mode::Add) {
        if (m_existingTweetIds.contains(id)) {
            QMessageBox::warning(this, "Input Error", "This Tweet ID already exists. Please choose a unique ID.");
            m_idLineEdit->setFocus();
            m_idLineEdit->selectAll();
            return false;
        }
    } else { // Mode::Edit
        // If ID was changed (and it's not read-only), ensure it doesn't clash with another existing ID
        if (id != m_originalTweetIdForEdit && m_existingTweetIds.contains(id)) {
             QMessageBox::warning(this, "Input Error", "The new Tweet ID clashes with an existing ID. Please choose a unique ID or revert.");
            m_idLineEdit->setFocus();
            m_idLineEdit->selectAll();
            return false;
        }
    }

    return true;
}

void TweetEditDialog::onAccept()
{
    if (validateInput()) {
        QDialog::accept(); // If validation passes, accept the dialog
    }
    // If validation fails, the dialog remains open
}