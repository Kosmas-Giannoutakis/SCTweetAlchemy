#ifndef NDEFGENERATOR_H
#define NDEFGENERATOR_H

#include <QString>
#include <QStringList> // For splitting lines

// Forward declaration, not strictly needed now but good for future
// class ISCCodePrettyPrinter; 

class NdefGenerator
{
public:
    NdefGenerator();
    // ~NdefGenerator(); // If we had m_prettyPrinter to delete

    // Generates Ndef with basic indentation for the inner code
    QString generateBasicNdef(const QString& originalCode, const QString& baseName) const;

    // Future:
    // void setPrettyPrinter(ISCCodePrettyPrinter* printer);

private:
    QString sanitizeNdefName(QString name) const;
    QString indentCodeBlock(const QString& code, const QString& indentString = "  ") const;

    // ISCCodePrettyPrinter* m_prettyPrinter; // For future tree-sitter integration
                                           // Initialized to nullptr in constructor
};

#endif // NDEFGENERATOR_H