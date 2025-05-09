#ifndef TWEETEDITDIALOG_H
#define TWEETEDITDIALOG_H

#include <QDialog>
#include "tweetdata.h" // For TweetData struct

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTextEdit;
class QDialogButtonBox;
class QFormLayout; // For laying out labels and fields
QT_END_NAMESPACE

class TweetEditDialog : public QDialog
{
    Q_OBJECT

public:
    // Enum to distinguish between Add and Edit mode
    enum class Mode { Add, Edit };

    explicit TweetEditDialog(Mode mode, QWidget *parent = nullptr);

    // Call this before exec() when in Edit mode
    void setTweetData(const TweetData& data);
    // Call this after a successful exec() to get the data
    TweetData getTweetData() const;
    // Call this to provide existing IDs for uniqueness validation (in Add mode)
    void setExistingTweetIds(const QSet<QString>& ids);


private slots:
    void onAccept(); // Custom slot for validation before accepting

private:
    void setupUi();
    bool validateInput(); // For input validation

    Mode m_mode;
    QSet<QString> m_existingTweetIds; // For uniqueness check in Add mode
    QString m_originalTweetIdForEdit; // To allow saving with same ID in Edit mode

    // UI Elements
    QLineEdit *m_idLineEdit;
    QTextEdit *m_codeTextEdit;
    QLineEdit *m_authorLineEdit;
    QLineEdit *m_sourceUrlLineEdit;
    QTextEdit *m_descriptionTextEdit; // QTextEdit for potentially longer descriptions
    QLineEdit *m_publicationDateLineEdit; // Simple QLineEdit for date for now
    QLineEdit *m_sonicTagsLineEdit;
    QLineEdit *m_techniqueTagsLineEdit;
    QLineEdit *m_genericTagsLineEdit;

    QDialogButtonBox *m_buttonBox;
};

#endif // TWEETEDITDIALOG_H