#include "ndefgenerator.h"
#include "sccodeprettyprinter.h" 
#include <QRegularExpression>
#include <QStringBuilder> 
#include <QDebug>      
#include <QtGlobal> // For qFuzzyCompare

NdefGenerator::NdefGenerator()
    : m_scPrettyPrinter(nullptr) 
{
    m_scPrettyPrinter = new SCCodePrettyPrinter();
    if (!m_scPrettyPrinter || !m_scPrettyPrinter->initialize()) {
        qCritical() << "NdefGenerator: Failed to initialize SCCodePrettyPrinter!";
        delete m_scPrettyPrinter; 
        m_scPrettyPrinter = nullptr; 
    }
}

NdefGenerator::~NdefGenerator()
{
    delete m_scPrettyPrinter; 
}

QString NdefGenerator::sanitizeNdefName(QString name) const
{
    if (name.isEmpty()) {
        name = "tweetNdef";
    }
    if (!name.isEmpty() && !name.at(0).isLetter() && name.at(0) != '_') {
        name.prepend('_'); 
    }
    name.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
    return name;
}

QString NdefGenerator::processCoreCodeSimple(const QString& originalCodeInput, bool& outAppendPlayFlag) const {
    QString coreCode = originalCodeInput.trimmed();
    outAppendPlayFlag = false;

    coreCode.replace(QRegularExpression("/\\*.*?\\*/", QRegularExpression::DotMatchesEverythingOption), "");
    coreCode.replace(QRegularExpression("//.*"), "");
    coreCode = coreCode.trimmed();

    QRegularExpression playBlockRegex(R"(^\s*play\s*\{(.*)\}\s*;?\s*$)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch playMatch = playBlockRegex.match(coreCode);

    if (playMatch.hasMatch()) {
        coreCode = playMatch.captured(1).trimmed();
        outAppendPlayFlag = true;
    } else {
        QRegularExpression funcPlayRegex(R"(^\s*(\{.*\})\s*\.play\s*;?\s*$)", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch funcPlayMatch = funcPlayRegex.match(coreCode);
        if (funcPlayMatch.hasMatch()) {
            coreCode = funcPlayMatch.captured(1).trimmed(); 
            outAppendPlayFlag = true;
        }
    }
    coreCode = coreCode.trimmed();

    if (coreCode.startsWith('{') && coreCode.endsWith('}')) {
        coreCode = coreCode.mid(1, coreCode.length() - 2).trimmed();
    }
    return coreCode;
}


QString NdefGenerator::generateNdef(const QString& originalCode, 
                                    const QString& baseName, 
                                    const NdefFormattingOptions& options) const
{
    QString ndefName = sanitizeNdefName(baseName); // Declared here
    bool ndefShouldPlay = false;                   // Declared here

    QString coreLogic = processCoreCodeSimple(originalCode, ndefShouldPlay);
    QString formattedInnerCode; // This will be the code inside Ndef's { }

    QString indentUnit = "  "; // Default indent unit, declared here
    if (m_scPrettyPrinter) {
        indentUnit = m_scPrettyPrinter->getIndentString();
    }

    if (options.style == NdefFormattingOptions::Style::ReformattedAST) {
        if (m_scPrettyPrinter) {
            if (m_scPrettyPrinter->parse(coreLogic)) { 
                formattedInnerCode = m_scPrettyPrinter->formatCurrentTree(); 
                if (formattedInnerCode.isEmpty()) {
                    qWarning() << "NdefGenerator (AST): Formatting/reconstruction returned empty for" << baseName << ". Using pre-processed core code.";
                    formattedInnerCode = coreLogic; 
                } else {
                     qInfo() << "NdefGenerator (AST): Formatted code via AST for" << baseName;
                }
            } else {
                qWarning() << "NdefGenerator (AST): Parsing failed for" << baseName << ". Using pre-processed core code.";
                formattedInnerCode = coreLogic; 
            }
        } else {
             qWarning() << "NdefGenerator (AST): SCCodePrettyPrinter not available. Using pre-processed core code.";
             formattedInnerCode = coreLogic; 
        }
        // formattedInnerCode IS NOW THE (potentially multi-line) output of the pretty printer
    } else { // SimplePlayable Style
        formattedInnerCode = coreLogic;
        // Flatten for simple playable style
        formattedInnerCode.replace(QRegularExpression("\\s*\\n\\s*"), " ");
        formattedInnerCode = formattedInnerCode.trimmed();
        formattedInnerCode.replace(QRegularExpression("\\s{2,}"), " ");
    }

    QString ndefFunctionBody = formattedInnerCode; // Initial body
    QString suffixChain; // For .reshaping, .fadeTime

    if (options.style == NdefFormattingOptions::Style::ReformattedAST) {
        if (options.wrapWithSplayAz) {
            QString splayIndentedSigBody = formattedInnerCode;
            if (formattedInnerCode.contains('\n')) { // Check if the input for SplayAz is multi-line
                QStringList lines = formattedInnerCode.split('\n');
                splayIndentedSigBody.clear();
                for (int i = 0; i < lines.size(); ++i) {
                    splayIndentedSigBody.append(indentUnit); 
                    splayIndentedSigBody.append(lines[i]);
                    if (i < lines.size() - 1) splayIndentedSigBody.append('\n');
                }
            } else {
                 splayIndentedSigBody = indentUnit + formattedInnerCode; // Indent single line
            }
            
            // SplayAz block itself has structure, apply indent to its lines
            ndefFunctionBody = QString("var sig = (\n%1\n%2);\n%2SplayAz.ar(%3, sig)")
                               .arg(splayIndentedSigBody) 
                               .arg(indentUnit) // Indent for closing paren of var sig and SplayAz line
                               .arg(options.splayAzChannels);
        }
        
        if (options.addReshapingExpanding) {
            suffixChain += QString("\n%1.reshaping_(\\expanding)").arg(indentUnit);
        }
        if (options.setFadeTime) {
            double fadeVal = options.fadeTimeValue;
            QString fadeValStr; // Declared here
            if (qFuzzyCompare(1.0 + fadeVal, 1.0 + static_cast<double>(static_cast<qint64>(fadeVal)))) { 
                fadeValStr = QString::number(static_cast<qint64>(fadeVal)); 
            } else {
                fadeValStr = QString::number(fadeVal, 'g', 15); 
            }
            suffixChain += QString("\n%1.fadeTime_(%2)").arg(indentUnit).arg(fadeValStr); 
        }
    }
    
    QString baseNdefString;
    if (options.style == NdefFormattingOptions::Style::ReformattedAST) {
        // Indent each line of the (potentially SplayAz'd) ndefFunctionBody
        QString properlyIndentedBody = ndefFunctionBody.replace("\n", "\n" + indentUnit);
        baseNdefString = QString("Ndef(\\%1, {\n%2%3\n%2})") 
                           .arg(ndefName)
                           .arg(indentUnit)        
                           .arg(properlyIndentedBody); 
    } else { // SimplePlayable
        baseNdefString = QString("Ndef(\\") % ndefName % ", { " % ndefFunctionBody % " })";
    }
    
    QString finalNdefString = baseNdefString + suffixChain;
    
    if (ndefShouldPlay) {
        if (options.style == NdefFormattingOptions::Style::ReformattedAST) {
             finalNdefString += QString("\n%1.play").arg(indentUnit); 
        } else {
            finalNdefString += ".play";
        }
    }

    if (!suffixChain.isEmpty() || ndefShouldPlay) {
        if (!finalNdefString.endsWith(";")) {
            finalNdefString += ";";
        }
    }
    
    if (options.style == NdefFormattingOptions::Style::ReformattedAST) {
        finalNdefString = QString("(") + finalNdefString + ")"; 
    }
    
    return finalNdefString;
}