#ifndef NDEFGENERATOR_H
#define NDEFGENERATOR_H

#include <QString>
#include <QStringList> 

class SCCodePrettyPrinter; 

// Structure to hold all formatting options
struct NdefFormattingOptions {
    enum class Style {
        SimplePlayable,
        ReformattedAST // This will later use full pretty-printing
        // Future: ReformattedSynthDef
    };

    Style style = Style::SimplePlayable;
    bool addReshapingExpanding = false;
    bool setFadeTime = false;
    double fadeTimeValue = 1.0;
    bool wrapWithSplayAz = false;
    int splayAzChannels = 2;
    // Add more flags/options here as needed
};

class NdefGenerator
{
public:
    NdefGenerator();
    ~NdefGenerator();

    // Main generation method
    QString generateNdef(const QString& originalCode, 
                         const QString& baseName, 
                         const NdefFormattingOptions& options) const;

private:
    QString sanitizeNdefName(QString name) const;
    
    // These can become private helpers or be inlined into generateNdef
    QString processCoreCodeSimple(const QString& originalCode, bool& outAppendPlayFlag) const;
    QString processCoreCodeAST(const QString& originalCode, bool& outAppendPlayFlag) const;


    SCCodePrettyPrinter* m_scPrettyPrinter; 
};

#endif // NDEFGENERATOR_H