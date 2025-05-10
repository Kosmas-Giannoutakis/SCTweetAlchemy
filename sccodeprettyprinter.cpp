#include "sccodeprettyprinter.h" 
#include <tree_sitter/api.h>    
#include <QDebug>
#include <QStringBuilder>
#include <QStack> 

extern "C" TSLanguage* tree_sitter_supercollider();

SCCodePrettyPrinter::SCCodePrettyPrinter(int indentWidth, int maxInlineArgs)
    : m_parser(nullptr), 
      m_scLanguage(nullptr), 
      m_currentTree(nullptr),
      m_indentWidth(indentWidth),
      m_indentString(QString(indentWidth, ' ')),
      m_maxInlineArgs(maxInlineArgs)
{
    m_noSpaceAfterTypes << "(" << "[" << "{" << "." << "~" << "\\" << "#" << "`" << "->" ; 
    m_noSpaceBeforeTypes << ")" << "]" << "}" << "." << "," << ";" << "->" ;
    
    m_binaryOperators << "&&" << "||" << "&" << "|" << "==" << "!=" << "<" << "<=" << ">" << ">="
                      << "<<" << ">>" << "+" << "-" << "++" << "*" << "/" << "%" << "**" << "=" << "+/+"
                      << "->" << "?" << "!?" << "??"; 
}

SCCodePrettyPrinter::~SCCodePrettyPrinter()
{
    if (m_currentTree) {
        ts_tree_delete(m_currentTree);
    }
    if (m_parser) {
        ts_parser_delete(m_parser);
    }
}

bool SCCodePrettyPrinter::initialize()
{
    m_parser = ts_parser_new();
    if (!m_parser) {
        qCritical() << "SCCodePrettyPrinter: Failed to create Tree-sitter parser.";
        return false; 
    }
    m_scLanguage = tree_sitter_supercollider();
    if (!m_scLanguage) {
        qCritical() << "SCCodePrettyPrinter: Failed to load Tree-sitter SuperCollider language grammar.";
        ts_parser_delete(m_parser);
        m_parser = nullptr;
        return false; 
    }
    if (!ts_parser_set_language(m_parser, m_scLanguage)) {
        qCritical() << "SCCodePrettyPrinter: Failed to set Tree-sitter language to SuperCollider.";
        ts_parser_delete(m_parser);
        m_parser = nullptr;
        m_scLanguage = nullptr;
        return false; 
    }
    qInfo() << "SCCodePrettyPrinter initialized successfully with SuperCollider grammar.";
    return true; 
}

bool SCCodePrettyPrinter::parse(const QString& scCode)
{
    if (!m_parser || !m_scLanguage) {
        qWarning() << "SCCodePrettyPrinter not initialized. Cannot parse.";
        return false; 
    }
    if (m_currentTree) {
        ts_tree_delete(m_currentTree);
        m_currentTree = nullptr;
    }
    m_lastParsedCode = scCode; 
    QByteArray codeUtf8 = scCode.toUtf8(); 
    m_currentTree = ts_parser_parse_string(
        m_parser,
        nullptr, 
        codeUtf8.constData(),
        codeUtf8.length()
    );
    if (!m_currentTree) {
        qWarning() << "SCCodePrettyPrinter: Tree-sitter failed to parse (returned null tree).";
        return false; 
    }
    return true; 
}

QString SCCodePrettyPrinter::getASTasSExpression() const
{
    if (!m_currentTree) {
        return "No tree parsed yet."; 
    }
    TSNode root_node = ts_tree_root_node(m_currentTree);
    char *s_expression_c_str = ts_node_string(root_node); 
    if (!s_expression_c_str) {
        return "Failed to get S-expression string."; 
    }
    QString s_expression_qstr = QString::fromUtf8(s_expression_c_str);
    free(s_expression_c_str); 
    return s_expression_qstr; 
}

QString SCCodePrettyPrinter::getIndentString() const 
{
    return m_indentString;
}

QString SCCodePrettyPrinter::getNodeText(TSNode node) const {
    if (ts_node_is_null(node)) return QString();
    uint32_t start_byte = ts_node_start_byte(node);
    uint32_t end_byte = ts_node_end_byte(node);
    if (end_byte < start_byte) return QString(); 
    int length = end_byte - start_byte;
    if (length < 0) return QString();

    QByteArray codeUtf8 = m_lastParsedCode.toUtf8();
    if (start_byte + length > (uint32_t)codeUtf8.length()) { 
        qWarning() << "Node byte range out of bounds:" << start_byte << "-" << end_byte << "for code length" << codeUtf8.length();
        if (start_byte >= (uint32_t)codeUtf8.length()) return QString();
        length = codeUtf8.length() - start_byte;
        if (length < 0) return QString();
    }
    return QString::fromUtf8(codeUtf8.constData() + start_byte, length);
}

void SCCodePrettyPrinter::appendWithIntelligentSpace(QString& builder, const QString& text, bool forceNoSpaceBefore) const {
    if (text.isEmpty()) return;

    bool noSpaceBeforeThis = forceNoSpaceBefore || 
                             (text.length() > 0 && m_noSpaceBeforeTypes.contains(text.at(0))) ||
                             (text.length() > 1 && m_noSpaceBeforeTypes.contains(text.left(2)));
    
    bool noSpaceAfterPrevious = false;
    if (!builder.isEmpty()) {
        noSpaceAfterPrevious = m_noSpaceAfterTypes.contains(builder.right(1)) ||
                               (builder.length() > 1 && m_noSpaceAfterTypes.contains(builder.right(2)));
    }

    if (!builder.isEmpty() && 
        !builder.endsWith(' ') && 
        !builder.endsWith('\n') &&
        !noSpaceAfterPrevious &&
        !noSpaceBeforeThis) 
    {
        builder.append(' ');
    }
    builder.append(text);
}

void SCCodePrettyPrinter::appendNewlineAndIndent(QString& builder, int indentLevel) const {
    int i = builder.length() - 1;
    while (i >= 0 && (builder.at(i) == QChar(' ') || builder.at(i) == QChar('\t'))) {
        i--;
    }
    if (i < 0 || builder.at(i) != QChar('\n')) { 
        builder.append('\n');
    } else { 
        builder.truncate(i + 1);
    }
    
    for (int j = 0; j < indentLevel; ++j) {
        builder.append(m_indentString);
    }
}

QString SCCodePrettyPrinter::formatCurrentTree() const
{
    if (!m_currentTree) {
        qWarning() << "SCCodePrettyPrinter: No tree to format. Parse code first.";
        return m_lastParsedCode; 
    }
    QString formattedCode;
    TSNode rootNode = ts_tree_root_node(m_currentTree);
    
    if (ts_node_is_error(rootNode) && ts_node_child_count(rootNode) == 0) { 
        qWarning() << "SCCodePrettyPrinter: Root node is an error or missing, returning original code.";
        return m_lastParsedCode;
    }

    formatNode(rootNode, formattedCode, 0, true); 
    return formattedCode.trimmed(); 
}

void SCCodePrettyPrinter::formatNode(TSNode node, QString& builder, int currentIndentLevel, bool parentPermitsInline) const
{
    if (ts_node_is_null(node)) return;

    const char* type = ts_node_type(node);
    bool isNamed = ts_node_is_named(node);
    uint32_t childCount = ts_node_child_count(node);
    uint32_t namedChildCount = ts_node_named_child_count(node);

    if (strcmp(type, "ERROR") == 0 || (ts_node_is_missing(node) && !isNamed) ) { 
        QString nodeStr = getNodeText(node).trimmed();
        if(!nodeStr.isEmpty()) appendWithIntelligentSpace(builder, nodeStr);
        return;
    }
     if (ts_node_is_missing(node) && isNamed) { 
        return;
    }

    if (strcmp(type, "source_file") == 0) {
        TSTreeCursor cursor = ts_tree_cursor_new(node);
        if (ts_tree_cursor_goto_first_child(&cursor)) {
            bool firstInFile = true;
            do {
                TSNode currentNode = ts_tree_cursor_current_node(&cursor);
                if (!firstInFile) {
                    // Ensure previous statement ended with a newline before starting new one
                    if (!builder.isEmpty() && !builder.endsWith('\n')) {
                        builder.append('\n'); // Add newline if missing
                    }
                     // Apply indent for the new line if builder isn't just a newline
                    if (builder.endsWith('\n') && builder.length() > 1) { // length > 1 to avoid indenting an empty builder
                        for(int i=0; i < currentIndentLevel; ++i) builder.append(m_indentString);
                    }
                }
                formatNode(currentNode, builder, currentIndentLevel, false);
                firstInFile = false;
            } while (ts_tree_cursor_goto_next_sibling(&cursor));
        }
        ts_tree_cursor_delete(&cursor);
    } else if (strcmp(type, "_expression_sequence") == 0) { 
        TSTreeCursor cursor = ts_tree_cursor_new(node);
        if (ts_tree_cursor_goto_first_child(&cursor)) {
            do {
                formatNode(ts_tree_cursor_current_node(&cursor), builder, currentIndentLevel, false); 
            } while (ts_tree_cursor_goto_next_sibling(&cursor));
        }
        ts_tree_cursor_delete(&cursor);
    } else if (strcmp(type, "_expression") == 0) { 
        TSNode statementNode = ts_node_child(node, 0);
        TSNode semicolonNode = {}; 
        if (childCount > 1) semicolonNode = ts_node_child(node, 1); 

        if (!ts_node_is_null(statementNode)) {
            formatNode(statementNode, builder, currentIndentLevel, parentPermitsInline); 
        }
        if (!ts_node_is_null(semicolonNode) && strcmp(ts_node_type(semicolonNode), ";") == 0) {
            if (builder.endsWith(' ')) builder.chop(1);
            builder.append(getNodeText(semicolonNode));
        }
        
        if (!parentPermitsInline ) {
            if (!builder.isEmpty() && !builder.endsWith('\n')) {
                 builder.append('\n');
            }
            // Don't add indent here, next statement in source_file/expression_sequence will handle its own leading indent.
            // Only add indent if we are sure a new *indented line* should start,
            // but _expression itself doesn't start a new scope.
        }

    } else if (strcmp(type, "_expression_statement") == 0 || strcmp(type, "_object") == 0 || 
               strcmp(type, "unnamed_argument") == 0 || strcmp(type, "named_argument") == 0 ||
               strcmp(type, "value") == 0 || strcmp(type, "receiver") == 0 || 
               strcmp(type, "left") == 0 || strcmp(type, "right") == 0 ||
               strcmp(type, "identifier") == 0 || strcmp(type, "class") == 0 || strcmp(type, "method_name") == 0 ) {
        if (isNamed && childCount == 0) { 
            appendWithIntelligentSpace(builder, getNodeText(node).trimmed(), parentPermitsInline && builder.endsWith("."));
        } else { 
            TSTreeCursor cursor = ts_tree_cursor_new(node);
            if (ts_tree_cursor_goto_first_child(&cursor)) {
                do {
                     formatNode(ts_tree_cursor_current_node(&cursor), builder, currentIndentLevel, parentPermitsInline);
                } while (ts_tree_cursor_goto_next_sibling(&cursor));
            }
            ts_tree_cursor_delete(&cursor);
        }
    } else if (strcmp(type, "function_block") == 0 || strcmp(type, "code_block") == 0) {
        bool isCodeBlock = strcmp(type, "code_block") == 0;
        appendWithIntelligentSpace(builder, isCodeBlock ? "(" : "{", true);
        
        bool hasSignificantContent = false;
        TSTreeCursor contentCheckCursor = ts_tree_cursor_new(node);
        if (ts_tree_cursor_goto_first_child(&contentCheckCursor)) {
            do {
                TSNode child = ts_tree_cursor_current_node(&contentCheckCursor);
                const char* ct = ts_node_type(child);
                 if (strcmp(ct, "{") != 0 && strcmp(ct, "}") != 0 && 
                    strcmp(ct, "(") != 0 && strcmp(ct, ")") != 0 &&
                    strcmp(ct, "parameter_list") !=0 ) { 
                    if (strcmp(ct, "_expression_sequence") == 0 && ts_node_child_count(child) == 0) {
                    } else {
                        hasSignificantContent = true;
                        break;
                    }
                }
            } while (ts_tree_cursor_goto_next_sibling(&contentCheckCursor));
        }
        ts_tree_cursor_delete(&contentCheckCursor);
        
        if (hasSignificantContent) {
            appendNewlineAndIndent(builder, currentIndentLevel + 1);
        } else if (!isCodeBlock && childCount <= 2 && parentPermitsInline ) { 
             builder.append(' '); 
        }

        TSTreeCursor childCursor = ts_tree_cursor_new(node);
        if (ts_tree_cursor_goto_first_child(&childCursor)) {
            do {
                TSNode child = ts_tree_cursor_current_node(&childCursor);
                const char* childType = ts_node_type(child);
                if (strcmp(childType, "{") == 0 || strcmp(childType, "}") == 0 ||
                    strcmp(childType, "(") == 0 || strcmp(childType, ")") == 0 ) continue; 
                formatNode(child, builder, currentIndentLevel + 1, !hasSignificantContent); 
            } while (ts_tree_cursor_goto_next_sibling(&childCursor));
        }
        ts_tree_cursor_delete(&childCursor);

        if (hasSignificantContent) {
            if (builder.endsWith(m_indentString.repeated(currentIndentLevel + 1))) {
                 builder.chop(m_indentString.length() * (currentIndentLevel + 1));
                 if (builder.endsWith('\n')) builder.chop(1);
            }
            appendNewlineAndIndent(builder, currentIndentLevel); 
        } else if (!isCodeBlock && builder.endsWith(' ') && childCount <=2) { 
             builder.chop(1); 
        }
        
        if (builder.endsWith(' ') && hasSignificantContent && !isCodeBlock) builder.chop(1);
        else if (builder.endsWith(' ') && isCodeBlock) builder.chop(1);

        builder.append(isCodeBlock ? ")" : "}");

    } else if (strcmp(type, "parameter_list") == 0) {
        TSNode firstToken = ts_node_child(node, 0); 
        appendWithIntelligentSpace(builder, getNodeText(firstToken).trimmed());
        if (strcmp(ts_node_type(firstToken), "arg")==0) appendWithIntelligentSpace(builder, " ");

        TSTreeCursor cursor = ts_tree_cursor_new(node); 
        if (ts_tree_cursor_goto_first_child(&cursor)) { 
            bool firstParamContent = true;
            while(ts_tree_cursor_goto_next_sibling(&cursor)) { 
                TSNode child = ts_tree_cursor_current_node(&cursor); 
                const char* childType = ts_node_type(child);
                if (strcmp(childType, ";") == 0 || strcmp(childType, "|") == 0) break; 
                
                if(strcmp(childType, ",") == 0) {
                     if(builder.endsWith(' ')) builder.chop(1); builder.append(getNodeText(child)); appendWithIntelligentSpace(builder, " ");
                } else {
                     formatNode(child, builder, currentIndentLevel, true); 
                }
                firstParamContent = false;
            }
        }
        ts_tree_cursor_delete(&cursor); 

        if (builder.endsWith(' ')) builder.chop(1); 
        appendWithIntelligentSpace(builder, getNodeText(ts_node_child(node, childCount-1)).trimmed()); 

    } else if (strcmp(type, "function_call") == 0 || strcmp(type, "method_call") == 0) {
        TSNode receiverNode = {}; TSNode nameNode = {}; TSNode paramListNode = {};
        TSNode openParenToken = {}; TSNode closeParenToken = {}; TSNode trailingFunctionBlock = {};

        TSTreeCursor c = ts_tree_cursor_new(node);
        if(ts_tree_cursor_goto_first_child(&c)) {  
            do {
                TSNode child = ts_tree_cursor_current_node(&c);
                const char* ct = ts_node_type(child);
                const char* fn = ts_tree_cursor_current_field_name(&c); 

                if (fn && strcmp(fn, "receiver") == 0) receiverNode = child;
                else if (fn && (strcmp(fn, "name") == 0 || strcmp(fn, "method_name") == 0)) nameNode = child;
                else if (strcmp(ct, "parameter_call_list") == 0) paramListNode = child;
                else if (strcmp(ct, "(") == 0 && ts_node_is_null(openParenToken)) openParenToken = child;
                else if (strcmp(ct, ")") == 0) closeParenToken = child; 
                else if (strcmp(ct, "function_block") == 0 && ts_node_is_null(paramListNode) && ts_node_is_null(openParenToken)) {
                    trailingFunctionBlock = child;
                } else if (ts_node_is_null(nameNode) && ts_node_is_null(receiverNode) && 
                         (strcmp(ct, "identifier") == 0 || strcmp(ct, "class") == 0)) {
                    nameNode = child;
                }
            } while (ts_tree_cursor_goto_next_sibling(&c));
        }
        ts_tree_cursor_delete(&c);

        if (!ts_node_is_null(receiverNode)) {
            formatNode(receiverNode, builder, currentIndentLevel, true); 
            builder.append("."); 
        }
        if (!ts_node_is_null(nameNode)) {
            appendWithIntelligentSpace(builder, getNodeText(nameNode), !ts_node_is_null(receiverNode));
        }

        if (!ts_node_is_null(openParenToken)) { 
            appendWithIntelligentSpace(builder, "(", true);
        }

        bool breakArgs = false;
        uint32_t argCount = 0; 
        if (!ts_node_is_null(paramListNode)) {
            argCount = ts_node_named_child_count(paramListNode); 
             if (argCount > 0) {
                 if (!parentPermitsInline || argCount > m_maxInlineArgs || 
                     !areChildrenSimpleEnoughForInline(paramListNode, {"unnamed_argument", "named_argument"}, m_maxInlineArgs)) {
                     breakArgs = true;
                 }
             }
        }
        
        if (breakArgs && argCount > 0 && !ts_node_is_null(openParenToken)) { 
             appendNewlineAndIndent(builder, currentIndentLevel + 1);
        }

        if (!ts_node_is_null(paramListNode)) {
            TSTreeCursor argCursor = ts_tree_cursor_new(paramListNode); 
            if (ts_tree_cursor_goto_first_child(&argCursor)) { 
                bool firstArgInLine = true;
                do {
                    TSNode argChildNode = ts_tree_cursor_current_node(&argCursor); 
                    const char* argChildType = ts_node_type(argChildNode);
                    if (strcmp(argChildType, ",") == 0) {
                        if(builder.endsWith(' ')) builder.chop(1);
                        builder.append(getNodeText(argChildNode)); 
                        if (breakArgs) appendNewlineAndIndent(builder, currentIndentLevel + 1);
                        else appendWithIntelligentSpace(builder, " "); 
                        firstArgInLine = true; 
                    } else { 
                        if (!firstArgInLine && !breakArgs && !builder.endsWith(" ") && !(builder.endsWith(m_indentString) && builder.endsWith("\n"+m_indentString))) {
                             appendWithIntelligentSpace(builder, " "); 
                        }
                        formatNode(argChildNode, builder, currentIndentLevel + (breakArgs ? 1:0), !breakArgs);
                        firstArgInLine = false;
                    }
                } while (ts_tree_cursor_goto_next_sibling(&argCursor)); 
            }
            ts_tree_cursor_delete(&argCursor); 
        }
            
        if (breakArgs && argCount > 0 && !ts_node_is_null(openParenToken)) { 
            if (!builder.endsWith(m_indentString.repeated(currentIndentLevel))) { // If last arg didn't add newline+indent for this level
                 appendNewlineAndIndent(builder, currentIndentLevel);
            }
        }
        
        if (!ts_node_is_null(closeParenToken)) { 
             if (builder.endsWith(m_indentString) && builder.endsWith("\n" + m_indentString) && breakArgs && argCount > 0) {
                 builder.chop(m_indentString.length()); 
                 if(builder.endsWith('\n')) builder.chop(1); 
             } else if (builder.endsWith(' ')) {
                 builder.chop(1);
             }
            builder.append(")");
        }
        
        if(!ts_node_is_null(trailingFunctionBlock)){ 
            appendWithIntelligentSpace(builder, " "); 
            formatNode(trailingFunctionBlock, builder, currentIndentLevel, false); 
        }

    } else if (strcmp(type, "binary_expression") == 0) {
        TSNode left = ts_node_child_by_field_name(node, "left", strlen("left"));
        TSNode opNode   = ts_node_child_by_field_name(node, "operator", strlen("operator"));
        TSNode right= ts_node_child_by_field_name(node, "right", strlen("right"));

        formatNode(left, builder, currentIndentLevel, true); 
        if (!ts_node_is_null(opNode)) appendWithIntelligentSpace(builder, getNodeText(opNode).trimmed()); 
        formatNode(right, builder, currentIndentLevel, true); 

    } else if (strcmp(type, "collection") == 0 || strcmp(type, "arithmetic_series") == 0) {
        bool isArithmetic = strcmp(type, "arithmetic_series") == 0;
        QString openBracket = isArithmetic ? "(" : "[";
        QString closeBracket = isArithmetic ? ")" : "]";
        TSNode classTypeNode = {}; TSNode refNode = {}; TSNode contentSequenceNode = {}; 

        TSTreeCursor cursor = ts_tree_cursor_new(node); 
        if(ts_tree_cursor_goto_first_child(&cursor)) { 
            do {
                TSNode child = ts_tree_cursor_current_node(&cursor); 
                const char* childType = ts_node_type(child);
                const char* fieldName = ts_tree_cursor_current_field_name(&cursor); 
                
                if (fieldName && strcmp(fieldName, "collection_type") == 0) classTypeNode = child; 
                else if (strcmp(childType, "ref") == 0 || strcmp(childType, "#")==0) refNode = child;
                else if (strcmp(childType, "_collection_sequence") == 0 || strcmp(childType, "_paired_associative_sequence") == 0) contentSequenceNode = child;
                else if (isArithmetic && (strcmp(childType, "number")==0 || strcmp(childType, ",")==0 || strcmp(childType, "..")==0) ) {
                    if(ts_node_is_null(contentSequenceNode)) contentSequenceNode = node; 
                }
            } while(ts_tree_cursor_goto_next_sibling(&cursor)); 
        }
        ts_tree_cursor_delete(&cursor); 

        if (!ts_node_is_null(refNode)) appendWithIntelligentSpace(builder, getNodeText(refNode), true);
        if (!ts_node_is_null(classTypeNode)) formatNode(classTypeNode, builder, currentIndentLevel, true); 
        appendWithIntelligentSpace(builder, openBracket, !ts_node_is_null(classTypeNode) || !ts_node_is_null(refNode) );

        bool breakElements = false;
        uint32_t actualElementCount = 0;
        if (!ts_node_is_null(contentSequenceNode)) {
            TSTreeCursor tempCursor = ts_tree_cursor_new(contentSequenceNode);
            if (ts_tree_cursor_goto_first_child(&tempCursor)) {
                do {
                    if (strcmp(ts_node_type(ts_tree_cursor_current_node(&tempCursor)), ",") !=0) actualElementCount++;
                } while (ts_tree_cursor_goto_next_sibling(&tempCursor));
            }
            ts_tree_cursor_delete(&tempCursor);

            if (actualElementCount > 0 && (!parentPermitsInline || !areChildrenSimpleEnoughForInline(contentSequenceNode, {"_object", "associative_item", "number"}, m_maxInlineArgs))) {
                breakElements = true;
            }
        }
        
        if (breakElements && actualElementCount > 0) appendNewlineAndIndent(builder, currentIndentLevel + 1);

        if (!ts_node_is_null(contentSequenceNode)) {
            TSTreeCursor elCursor = ts_tree_cursor_new(contentSequenceNode); 
            if (ts_tree_cursor_goto_first_child(&elCursor)) { 
                bool firstElementInLine = true;
                do {
                    TSNode element = ts_tree_cursor_current_node(&elCursor); 
                    const char* elType = ts_node_type(element);
                    if (strcmp(elType, ",") == 0) {
                        if (builder.endsWith(' ')) builder.chop(1);
                        builder.append(getNodeText(element));
                        if (breakElements) appendNewlineAndIndent(builder, currentIndentLevel + 1);
                        else appendWithIntelligentSpace(builder, " ");
                        firstElementInLine = true; 
                    } else if (isArithmetic && (strcmp(elType, openBracket.toUtf8().constData()) == 0 || 
                                                strcmp(elType, closeBracket.toUtf8().constData()) == 0 )) {
                        continue;
                    }
                     else {
                        if (!firstElementInLine && !breakElements) appendWithIntelligentSpace(builder, " ");
                        formatNode(element, builder, currentIndentLevel + (breakElements ? 1:0), !breakElements);
                        firstElementInLine = false;
                    }
                } while (ts_tree_cursor_goto_next_sibling(&elCursor)); 
            }
            ts_tree_cursor_delete(&elCursor); 
        }
        if (breakElements && actualElementCount > 0 && !builder.endsWith('\n') ) {
             appendNewlineAndIndent(builder, currentIndentLevel);
        }
        
        if (builder.endsWith(' ') || (builder.endsWith(m_indentString) && breakElements && actualElementCount > 0)) {
            if (builder.endsWith(m_indentString)) builder.chop(m_indentString.length());
            else if (builder.endsWith(' ')) builder.chop(1);
        }
        builder.append(closeBracket);
    }
    else if (isNamed && childCount == 0) { 
        appendWithIntelligentSpace(builder, getNodeText(node).trimmed(), parentPermitsInline && builder.endsWith("."));
    } 
    else if (!isNamed && childCount == 0) { 
        QString txt = getNodeText(node).trimmed();
        bool forceNoSpace = m_noSpaceAfterTypes.contains(txt) || builder.isEmpty(); 
        if (txt == ";" && builder.endsWith(' ')) builder.chop(1); 
        appendWithIntelligentSpace(builder, txt, forceNoSpace);
    }
    else if (isNamed) { 
        TSTreeCursor cursor = ts_tree_cursor_new(node); 
        if (ts_tree_cursor_goto_first_child(&cursor)) { 
            do {
                formatNode(ts_tree_cursor_current_node(&cursor), builder, currentIndentLevel, true); 
            } while (ts_tree_cursor_goto_next_sibling(&cursor)); 
        }
        ts_tree_cursor_delete(&cursor); 
    }
}

bool SCCodePrettyPrinter::areChildrenSimpleEnoughForInline(TSNode parentNode, const QSet<QString>& interestingChildTypes, int maxChildrenForInline) const {
    if (ts_node_is_null(parentNode)) return true;
    QList<TSNode> relevantNodes;

    TSTreeCursor cursor = ts_tree_cursor_new(parentNode); 
    if (ts_tree_cursor_goto_first_child(&cursor)) { 
        do {
            TSNode child = ts_tree_cursor_current_node(&cursor); 
            if (interestingChildTypes.contains(QLatin1String(ts_node_type(child)))) {
                relevantNodes.append(child);
            }
        } while (ts_tree_cursor_goto_next_sibling(&cursor)); 
    }
    ts_tree_cursor_delete(&cursor); 
    
    if(relevantNodes.count() > maxChildrenForInline) return false;

    for(const TSNode& child : relevantNodes) {
        if (!isNodeSimple(child)) { 
            return false;
        }
    }
    return true; 
}

bool SCCodePrettyPrinter::isNodeSimple(TSNode node, int depth) const {
    if (ts_node_is_null(node)) return true; 
    if (depth > 1) { 
        return false; 
    }

    const char* type = ts_node_type(node);
    uint32_t namedChildCount = ts_node_named_child_count(node);
    uint32_t childCount = ts_node_child_count(node); // Total children, including anonymous

    // Terminals we consider simple
    if (strcmp(type, "number") == 0 || strcmp(type, "integer") == 0 || strcmp(type, "float") == 0 ||
        strcmp(type, "string") == 0 || strcmp(type, "symbol") == 0 || strcmp(type, "char") == 0 ||
        strcmp(type, "bool") == 0 || strcmp(type, "identifier") == 0 ||
        strcmp(type, "local_var") == 0 || strcmp(type, "environment_var") == 0 ||
        strcmp(type, "builtin_var") == 0 || strcmp(type, "instance_var") == 0 ||
        strcmp(type, "class") == 0 || strcmp(type, "method_name") == 0 ) 
    {
        // These are simple if they don't have further complex named children
        for(uint32_t i=0; i < namedChildCount; ++i) { 
            if(!isNodeSimple(ts_node_named_child(node,i), depth+1)) return false;
        }
        return true;
    }

    if (strcmp(type, "unnamed_argument") == 0) {
        if (namedChildCount > 0) {
            return isNodeSimple(ts_node_named_child(node, 0), depth + 1);
        }
        return true; 
    }
    
    if (strcmp(type, "named_argument") == 0) {
        TSNode nameFieldNode = ts_node_child_by_field_name(node, "name", strlen("name"));
        TSNode valueNode = {}; 
        if (!ts_node_is_null(nameFieldNode)) {
            uint32_t nameFieldChildCount = ts_node_child_count(nameFieldNode);
            if (nameFieldChildCount > 0) {
                TSNode lastChildOfNameField = ts_node_child(nameFieldNode, nameFieldChildCount - 1);
                if (strcmp(ts_node_type(lastChildOfNameField), "seq") == 0 && ts_node_child_count(lastChildOfNameField) > 0) { 
                     valueNode = ts_node_child(lastChildOfNameField, ts_node_child_count(lastChildOfNameField) -1 );
                } else if (ts_node_is_named(lastChildOfNameField)) { 
                    valueNode = lastChildOfNameField; 
                } 
            }
        }
        return isNodeSimple(valueNode, depth + 1);
    }
    
    if (strcmp(type, "function_call") == 0 || strcmp(type, "method_call") == 0) {
        TSNode paramListNode = {};
        for(uint32_t i=0; i < childCount; ++i) { // Check all children for paramListNode
            TSNode child = ts_node_child(node,i);
            if(strcmp(ts_node_type(child), "parameter_call_list") == 0) {
                paramListNode = child;
                break;
            }
        }
        if (ts_node_is_null(paramListNode)) return true; 

        uint32_t argCount = ts_node_named_child_count(paramListNode);
        if (argCount == 0) return true;
        if (depth > 0 && argCount > 0) return false; 
        if (argCount > m_maxInlineArgs) return false;

        for (uint32_t i = 0; i < argCount; ++i) {
            TSNode argNode = ts_node_named_child(paramListNode, i);
            if (!isNodeSimple(argNode, depth + 1)) {
                return false; 
            }
        }
        return true; 
    }
    
    if (strcmp(type, "collection") == 0 || strcmp(type, "arithmetic_series") == 0) {
         uint32_t items = 0;
         bool complexItemFound = false;
         TSTreeCursor cursor = ts_tree_cursor_new(node); 
         if(ts_tree_cursor_goto_first_child(&cursor)){ 
            do {
                TSNode item = ts_tree_cursor_current_node(&cursor); 
                const char* itemType = ts_node_type(item);
                if(ts_node_is_named(item) && 
                   strcmp(itemType, "collection_type") != 0 && 
                   strcmp(itemType, "ref") !=0 &&
                   strcmp(itemType, "#") !=0 &&
                   strcmp(itemType, "[") !=0 && strcmp(itemType, "]") !=0 && 
                   strcmp(itemType, "(") !=0 && strcmp(itemType, ")") !=0 && 
                   strcmp(itemType, ",") !=0 ) { 
                    items++;
                    if (!isNodeSimple(item, depth + 1)) {
                        complexItemFound = true;
                        break;
                    }
                }
            } while(ts_tree_cursor_goto_next_sibling(&cursor)); 
         }
         ts_tree_cursor_delete(&cursor); 
         if(complexItemFound) return false;
         return items <= (uint32_t)m_maxInlineArgs; 
    }
    
    if (strcmp(type, "binary_expression") == 0) {
        TSNode left = ts_node_child_by_field_name(node, "left", strlen("left"));
        TSNode right = ts_node_child_by_field_name(node, "right", strlen("right"));
        return isNodeSimple(left, depth + 1) && isNodeSimple(right, depth + 1);
    }

    // Fallback logic:
    if (namedChildCount > 0) { // If it's an unhandled internal node with named children, assume complex.
        return false;
    }
    // If no named children (could be an unhandled leaf, or internal node with only anonymous children like operators/punctuation)
    // consider it simple for now. This might need refinement e.g. by checking text length or types of anon children.
    return true; 
}