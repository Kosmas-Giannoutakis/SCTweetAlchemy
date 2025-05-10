#include "ndefgenerator.h"
#include <QRegularExpression>
#include <QStringBuilder> // For efficient string concatenation
#include <QDebug>         // For logging

NdefGenerator::NdefGenerator()
{
}

QString NdefGenerator::sanitizeNdefName(QString name) const
{
    if (name.isEmpty()) {
        name = "tweetNdef";
    }
    if (!name.at(0).isLetter() && name.at(0) != '_') {
        name.prepend('_');
    }
    name.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
    return name;
}

// indentCodeBlock is no longer used for this simple, single-line Ndef body
/*
QString NdefGenerator::indentCodeBlock(const QString& code, const QString& indentString) const
{
    // ... previous indentation logic ...
}
*/

QString NdefGenerator::generateBasicNdef(const QString& originalCode, const QString& baseName) const
{
    QString ndefName = sanitizeNdefName(baseName);
    QString coreCode = originalCode.trimmed();

    // 1. Remove comments
    coreCode.replace(QRegularExpression("/\\*.*?\\*/", QRegularExpression::DotMatchesEverythingOption), "");
    coreCode.replace(QRegularExpression("//.*"), "");
    coreCode = coreCode.trimmed();

    bool appendPlay = false;

    // 2. Handle leading play{ ... } and trailing }; or }
    QRegularExpression playBlockRegex(R"(^\s*play\s*\{(.*)\}\s*;?\s*$)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch playMatch = playBlockRegex.match(coreCode);

    if (playMatch.hasMatch()) {
        coreCode = playMatch.captured(1).trimmed();
        appendPlay = true;
    } else {
        QRegularExpression funcPlayRegex(R"(^\s*(\{.*\})\s*\.play\s*;?\s*$)", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch funcPlayMatch = funcPlayRegex.match(coreCode);
        if (funcPlayMatch.hasMatch()) {
            coreCode = funcPlayMatch.captured(1).trimmed();
            appendPlay = true;
        }
    }
    coreCode = coreCode.trimmed();

    // 3. Ensure the core code is a valid function body for Ndef
    if (coreCode.startsWith('{') && coreCode.endsWith('}')) {
        // It's already a block, remove the outer braces as Ndef adds its own { } for the function
        coreCode = coreCode.mid(1, coreCode.length() - 2).trimmed();
    }
    
    // 4. Process coreCode to be a single logical line for the Ndef body
    //    Replace internal newlines with a space, and condense multiple spaces.
    //    This makes it more robust for pasting directly into SC.
    QString singleLineCoreCode = coreCode;
    singleLineCoreCode.replace(QRegularExpression("\\s*\\n\\s*"), " "); // Replace newlines (and surrounding whitespace) with a single space
    singleLineCoreCode = singleLineCoreCode.trimmed(); // Trim ends
    singleLineCoreCode.replace(QRegularExpression("\\s{2,}"), " "); // Condense multiple spaces into one

    // 5. Construct the Ndef string
    //    The Ndef function body itself will now be on one effective line.
    QString ndefString = QString("Ndef(\\") % ndefName % ", { " % singleLineCoreCode % " })";
                                        // ^ space after {      ^ space before }

    if (appendPlay) {
        ndefString += ".play;";
    }
    // Optional: Add a semicolon if not playing, for assignment contexts.
    // else {
    //    ndefString += ";";
    // }
    
    return ndefString;
}