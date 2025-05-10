#ifndef SCCODEPRETTYPRINTER_H
#define SCCODEPRETTYPRINTER_H

#include <QString>
#include <QSet> 

#include <tree_sitter/api.h> // <<< ADD THIS INCLUDE HERE

// Forward declarations are now less critical if api.h is included,
// but keeping them doesn't hurt for types not fully defined in api.h for some reason.
// struct TSParser; 
// struct TSLanguage;
// struct TSTree;
// struct TSNode; 
// TSCursor is defined in api.h

class SCCodePrettyPrinter
{
public:
    SCCodePrettyPrinter(int indentWidth = 4, int maxInlineArgs = 3);
    ~SCCodePrettyPrinter();

    bool initialize();
    bool parse(const QString& scCode);
    QString getASTasSExpression() const;

    QString formatCurrentTree() const; 

    QString getIndentString() const;

private:
    void formatNode(TSNode node, QString& builder, int currentIndentLevel, bool parentPermitsInline = false) const;
    
    QString getNodeText(TSNode node) const;
    bool areChildrenSimpleEnoughForInline(TSNode parentNode, const QSet<QString>& interestingChildTypes, int maxChildrenForInline) const;
    bool isNodeSimple(TSNode node, int depth = 0) const; 

    void appendWithIntelligentSpace(QString& builder, const QString& text, bool forceNoSpaceBefore = false) const;
    void appendNewlineAndIndent(QString& builder, int indentLevel) const;

    TSParser *m_parser;
    TSLanguage *m_scLanguage;
    TSTree *m_currentTree;   
    QString m_lastParsedCode; 

    int m_indentWidth;          
    QString m_indentString;     
    int m_maxInlineArgs;        

    QSet<QString> m_noSpaceAfterTypes;
    QSet<QString> m_noSpaceBeforeTypes;
    QSet<QString> m_binaryOperators;
};

#endif // SCCODEPRETTYPRINTER_H