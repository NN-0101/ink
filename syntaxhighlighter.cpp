#include "syntaxhighlighter.h"
#include <QDebug>

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // 初始化各种格式
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);

    classFormat.setForeground(Qt::darkMagenta);
    classFormat.setFontWeight(QFont::Bold);

    singleLineCommentFormat.setForeground(Qt::darkGreen);

    multiLineCommentFormat.setForeground(Qt::darkGreen);

    quotationFormat.setForeground(Qt::darkRed);

    functionFormat.setForeground(Qt::blue);

    numberFormat.setForeground(Qt::darkCyan);

    preprocessorFormat.setForeground(Qt::darkYellow);

    markdownHeaderFormat.setForeground(Qt::darkBlue);
    markdownHeaderFormat.setFontWeight(QFont::Bold);

    markdownBoldFormat.setFontWeight(QFont::Bold);

    markdownItalicFormat.setFontItalic(true);

    markdownCodeFormat.setForeground(Qt::darkGreen);
    markdownCodeFormat.setFontFamily("Courier");
}

SyntaxHighlighter::FileType SyntaxHighlighter::detectFileType(const QString &fileName)
{
    QString suffix = fileName.mid(fileName.lastIndexOf('.') + 1).toLower();

    if (suffix == "cpp" || suffix == "cc" || suffix == "cxx" || suffix == "c++" ||
        suffix == "h" || suffix == "hh" || suffix == "hpp" || suffix == "hxx") {
        return Cpp;
    } else if (suffix == "py" || suffix == "pyw") {
        return Python;
    } else if (suffix == "js" || suffix == "jsx" || suffix == "ts" || suffix == "tsx") {
        return JavaScript;
    } else if (suffix == "md" || suffix == "markdown") {
        return Markdown;
    } else if (suffix == "html" || suffix == "htm" || suffix == "xhtml") {
        return HTML;
    } else if (suffix == "css" || suffix == "scss" || suffix == "sass" || suffix == "less") {
        return CSS;
    } else if (suffix == "java") {
        return Java;
    }

    return Unknown;
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // 检测文件类型并初始化相应的高亮规则
    if (highlightingRules.isEmpty()) {
        QTextDocument *doc = document();
        QString fileName = doc->property("fileName").toString();
        FileType fileType = detectFileType(fileName);

        switch (fileType) {
        case Cpp:
            initCppRules();
            break;
        case Python:
            initPythonRules();
            break;
        case JavaScript:
            initJavaScriptRules();
            break;
        case Markdown:
            initMarkdownRules();
            break;
        case HTML:
            initHTMLRules();
            break;
        case CSS:
            initCSSRules();
            break;
        case Java:
            initJavaRules();
            break;
        default:
            // 未知文件类型，不进行高亮
            return;
        }
    }

    // 应用高亮规则
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // 处理多行注释（C++/Java等）
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(QRegularExpression("/\\*"));

    while (startIndex >= 0) {
        QRegularExpressionMatch match = QRegularExpression("\\*/").match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(QRegularExpression("/\\*"), startIndex + commentLength);
    }
}

void SyntaxHighlighter::initCppRules()
{
    highlightingRules.clear();

    // 关键字
    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bbool\\b"
                    << "\\breturn\\b" << "\\bif\\b" << "\\belse\\b"
                    << "\\bfor\\b" << "\\bwhile\\b" << "\\bdo\\b"
                    << "\\bbreak\\b" << "\\bcontinue\\b" << "\\bswitch\\b"
                    << "\\bcase\\b" << "\\bdefault\\b" << "\\btry\\b"
                    << "\\bcatch\\b" << "\\bthrow\\b" << "\\bnew\\b"
                    << "\\bdelete\\b" << "\\bthis\\b" << "\\btrue\\b"
                    << "\\bfalse\\b" << "\\bnullptr\\b" << "\\bNULL\\b"
                    << "\\binclude\\b" << "\\bdefine\\b" << "\\bifdef\\b"
                    << "\\bifndef\\b" << "\\bendif\\b";

    for (const QString &pattern : keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 类名
    HighlightingRule classRule;
    classRule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    classRule.format = classFormat;
    highlightingRules.append(classRule);

    // 单行注释
    HighlightingRule singleLineCommentRule;
    singleLineCommentRule.pattern = QRegularExpression("//[^\n]*");
    singleLineCommentRule.format = singleLineCommentFormat;
    highlightingRules.append(singleLineCommentRule);

    // 字符串
    HighlightingRule quotationRule;
    quotationRule.pattern = QRegularExpression("\".*?\"");
    quotationRule.format = quotationFormat;
    highlightingRules.append(quotationRule);

    // 字符
    HighlightingRule charRule;
    charRule.pattern = QRegularExpression("'.*?'");
    charRule.format = quotationFormat;
    highlightingRules.append(charRule);

    // 函数名
    HighlightingRule functionRule;
    functionRule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    functionRule.format = functionFormat;
    highlightingRules.append(functionRule);

    // 数字
    HighlightingRule numberRule;
    numberRule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    numberRule.format = numberFormat;
    highlightingRules.append(numberRule);

    // 预处理指令
    HighlightingRule preprocessorRule;
    preprocessorRule.pattern = QRegularExpression("^\\s*#.*");
    preprocessorRule.format = preprocessorFormat;
    highlightingRules.append(preprocessorRule);
}

void SyntaxHighlighter::initPythonRules()
{
    highlightingRules.clear();

    // Python关键字
    QStringList keywordPatterns;
    keywordPatterns << "\\bFalse\\b" << "\\bNone\\b" << "\\bTrue\\b"
                    << "\\band\\b" << "\\bas\\b" << "\\bassert\\b"
                    << "\\bbreak\\b" << "\\bclass\\b" << "\\bcontinue\\b"
                    << "\\bdef\\b" << "\\bdel\\b" << "\\belif\\b"
                    << "\\belse\\b" << "\\bexcept\\b" << "\\bfinally\\b"
                    << "\\bfor\\b" << "\\bfrom\\b" << "\\bglobal\\b"
                    << "\\bif\\b" << "\\bimport\\b" << "\\bin\\b"
                    << "\\bis\\b" << "\\blambda\\b" << "\\bnonlocal\\b"
                    << "\\bnot\\b" << "\\bor\\b" << "\\bpass\\b"
                    << "\\braise\\b" << "\\breturn\\b" << "\\btry\\b"
                    << "\\bwhile\\b" << "\\bwith\\b" << "\\byield\\b"
                    << "\\basync\\b" << "\\bawait\\b";

    for (const QString &pattern : keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 类名
    HighlightingRule classRule;
    classRule.pattern = QRegularExpression("\\bclass\\s+(\\w+)");
    classRule.format = classFormat;
    highlightingRules.append(classRule);

    // 单行注释
    HighlightingRule singleLineCommentRule;
    singleLineCommentRule.pattern = QRegularExpression("#[^\n]*");
    singleLineCommentRule.format = singleLineCommentFormat;
    highlightingRules.append(singleLineCommentRule);

    // 字符串
    HighlightingRule singleQuoteRule;
    singleQuoteRule.pattern = QRegularExpression("'[^']*'");
    singleQuoteRule.format = quotationFormat;
    highlightingRules.append(singleQuoteRule);

    HighlightingRule doubleQuoteRule;
    doubleQuoteRule.pattern = QRegularExpression("\"[^\"]*\"");
    doubleQuoteRule.format = quotationFormat;
    highlightingRules.append(doubleQuoteRule);

    HighlightingRule tripleSingleQuoteRule;
    tripleSingleQuoteRule.pattern = QRegularExpression("'''.*?'''");
    tripleSingleQuoteRule.format = quotationFormat;
    highlightingRules.append(tripleSingleQuoteRule);

    HighlightingRule tripleDoubleQuoteRule;
    tripleDoubleQuoteRule.pattern = QRegularExpression("\"\"\".*?\"\"\"");
    tripleDoubleQuoteRule.format = quotationFormat;
    highlightingRules.append(tripleDoubleQuoteRule);

    // 函数名
    HighlightingRule functionRule;
    functionRule.pattern = QRegularExpression("\\bdef\\s+(\\w+)");
    functionRule.format = functionFormat;
    highlightingRules.append(functionRule);

    // 数字
    HighlightingRule numberRule;
    numberRule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    numberRule.format = numberFormat;
    highlightingRules.append(numberRule);

    // 装饰器
    HighlightingRule decoratorRule;
    decoratorRule.pattern = QRegularExpression("@\\w+");
    decoratorRule.format = preprocessorFormat;
    highlightingRules.append(decoratorRule);

    // self参数
    HighlightingRule selfRule;
    selfRule.pattern = QRegularExpression("\\bself\\b");
    selfRule.format = functionFormat;
    highlightingRules.append(selfRule);
}

void SyntaxHighlighter::initJavaScriptRules()
{
    highlightingRules.clear();

    // JavaScript关键字
    QStringList keywordPatterns;
    keywordPatterns << "\\babstract\\b" << "\\barguments\\b" << "\\bawait\\b"
                    << "\\bboolean\\b" << "\\bbreak\\b" << "\\bbyte\\b"
                    << "\\bcase\\b" << "\\bcatch\\b" << "\\bchar\\b"
                    << "\\bclass\\b" << "\\bconst\\b" << "\\bcontinue\\b"
                    << "\\bdebugger\\b" << "\\bdefault\\b" << "\\bdelete\\b"
                    << "\\bdo\\b" << "\\bdouble\\b" << "\\belse\\b"
                    << "\\benum\\b" << "\\beval\\b" << "\\bexport\\b"
                    << "\\bextends\\b" << "\\bfalse\\b" << "\\bfinal\\b"
                    << "\\bfinally\\b" << "\\bfloat\\b" << "\\bfor\\b"
                    << "\\bfunction\\b" << "\\bgoto\\b" << "\\bif\\b"
                    << "\\bimplements\\b" << "\\bimport\\b" << "\\bin\\b"
                    << "\\binstanceof\\b" << "\\bint\\b" << "\\binterface\\b"
                    << "\\blet\\b" << "\\blong\\b" << "\\bnative\\b"
                    << "\\bnew\\b" << "\\bnull\\b" << "\\bpackage\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\breturn\\b" << "\\bshort\\b" << "\\bstatic\\b"
                    << "\\bsuper\\b" << "\\bswitch\\b" << "\\bsynchronized\\b"
                    << "\\bthis\\b" << "\\bthrow\\b" << "\\bthrows\\b"
                    << "\\btransient\\b" << "\\btrue\\b" << "\\btry\\b"
                    << "\\btypeof\\b" << "\\bvar\\b" << "\\bvoid\\b"
                    << "\\bvolatile\\b" << "\\bwhile\\b" << "\\bwith\\b"
                    << "\\byield\\b";

    for (const QString &pattern : keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 单行注释
    HighlightingRule singleLineCommentRule;
    singleLineCommentRule.pattern = QRegularExpression("//[^\n]*");
    singleLineCommentRule.format = singleLineCommentFormat;
    highlightingRules.append(singleLineCommentRule);

    // 多行注释开始
    HighlightingRule multiLineCommentStartRule;
    multiLineCommentStartRule.pattern = QRegularExpression("/\\*");
    multiLineCommentStartRule.format = multiLineCommentFormat;
    highlightingRules.append(multiLineCommentStartRule);

    // 多行注释结束
    HighlightingRule multiLineCommentEndRule;
    multiLineCommentEndRule.pattern = QRegularExpression("\\*/");
    multiLineCommentEndRule.format = multiLineCommentFormat;
    highlightingRules.append(multiLineCommentEndRule);

    // 字符串
    HighlightingRule singleQuoteRule;
    singleQuoteRule.pattern = QRegularExpression("'[^']*'");
    singleQuoteRule.format = quotationFormat;
    highlightingRules.append(singleQuoteRule);

    HighlightingRule doubleQuoteRule;
    doubleQuoteRule.pattern = QRegularExpression("\"[^\"]*\"");
    doubleQuoteRule.format = quotationFormat;
    highlightingRules.append(doubleQuoteRule);

    HighlightingRule templateLiteralRule;
    templateLiteralRule.pattern = QRegularExpression("`[^`]*`");
    templateLiteralRule.format = quotationFormat;
    highlightingRules.append(templateLiteralRule);

    // 函数名
    HighlightingRule functionRule;
    functionRule.pattern = QRegularExpression("\\bfunction\\s+(\\w+)");
    functionRule.format = functionFormat;
    highlightingRules.append(functionRule);

    // 数字
    HighlightingRule numberRule;
    numberRule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    numberRule.format = numberFormat;
    highlightingRules.append(numberRule);

    // 正则表达式
    HighlightingRule regexRule;
    regexRule.pattern = QRegularExpression("/([^/\\\\]*(?:\\\\.[^/\\\\]*)*)/[gimsuy]*");
    regexRule.format = numberFormat;
    highlightingRules.append(regexRule);
}

void SyntaxHighlighter::initMarkdownRules()
{
    highlightingRules.clear();

    // 标题
    HighlightingRule h1Rule;
    h1Rule.pattern = QRegularExpression("^#{1}\\s+(.*)$");
    h1Rule.format = markdownHeaderFormat;
    highlightingRules.append(h1Rule);

    HighlightingRule h2Rule;
    h2Rule.pattern = QRegularExpression("^#{2}\\s+(.*)$");
    h2Rule.format = markdownHeaderFormat;
    highlightingRules.append(h2Rule);

    HighlightingRule h3Rule;
    h3Rule.pattern = QRegularExpression("^#{3}\\s+(.*)$");
    h3Rule.format = markdownHeaderFormat;
    highlightingRules.append(h3Rule);

    HighlightingRule h4Rule;
    h4Rule.pattern = QRegularExpression("^#{4}\\s+(.*)$");
    h4Rule.format = markdownHeaderFormat;
    highlightingRules.append(h4Rule);

    HighlightingRule h5Rule;
    h5Rule.pattern = QRegularExpression("^#{5}\\s+(.*)$");
    h5Rule.format = markdownHeaderFormat;
    highlightingRules.append(h5Rule);

    HighlightingRule h6Rule;
    h6Rule.pattern = QRegularExpression("^#{6}\\s+(.*)$");
    h6Rule.format = markdownHeaderFormat;
    highlightingRules.append(h6Rule);

    // 粗体
    HighlightingRule boldRule1;
    boldRule1.pattern = QRegularExpression("\\*\\*[^*]*\\*\\*");
    boldRule1.format = markdownBoldFormat;
    highlightingRules.append(boldRule1);

    HighlightingRule boldRule2;
    boldRule2.pattern = QRegularExpression("__[^_]*__");
    boldRule2.format = markdownBoldFormat;
    highlightingRules.append(boldRule2);

    // 斜体
    HighlightingRule italicRule1;
    italicRule1.pattern = QRegularExpression("\\*[^*]*\\*");
    italicRule1.format = markdownItalicFormat;
    highlightingRules.append(italicRule1);

    HighlightingRule italicRule2;
    italicRule2.pattern = QRegularExpression("_[^_]*_");
    italicRule2.format = markdownItalicFormat;
    highlightingRules.append(italicRule2);

    // 代码块
    HighlightingRule inlineCodeRule;
    inlineCodeRule.pattern = QRegularExpression("`[^`]*`");
    inlineCodeRule.format = markdownCodeFormat;
    highlightingRules.append(inlineCodeRule);

    // 链接
    HighlightingRule linkRule;
    linkRule.pattern = QRegularExpression("\\[[^\\]]*\\]\\([^\\)]*\\)");
    linkRule.format = functionFormat;
    highlightingRules.append(linkRule);

    // 列表
    HighlightingRule listRule;
    listRule.pattern = QRegularExpression("^\\s*[-*+]\\s+");
    listRule.format = keywordFormat;
    highlightingRules.append(listRule);

    // 引用
    HighlightingRule blockquoteRule;
    blockquoteRule.pattern = QRegularExpression("^>\\s+.*");
    blockquoteRule.format = singleLineCommentFormat;
    highlightingRules.append(blockquoteRule);
}

void SyntaxHighlighter::initHTMLRules()
{
    highlightingRules.clear();

    // HTML标签
    HighlightingRule tagRule;
    tagRule.pattern = QRegularExpression("<[^>]*>");
    tagRule.format = keywordFormat;
    highlightingRules.append(tagRule);

    // 属性
    HighlightingRule attrRule;
    attrRule.pattern = QRegularExpression("\\b\\w+\\s*=");
    attrRule.format = functionFormat;
    highlightingRules.append(attrRule);

    // 注释
    HighlightingRule commentRule;
    commentRule.pattern = QRegularExpression("<!--.*-->");
    commentRule.format = singleLineCommentFormat;
    highlightingRules.append(commentRule);
}

void SyntaxHighlighter::initCSSRules()
{
    highlightingRules.clear();

    // CSS属性
    QStringList propertyPatterns;
    propertyPatterns << "\\bcolor\\b" << "\\bbackground\\b" << "\\bfont\\b"
                     << "\\bmargin\\b" << "\\bpadding\\b" << "\\bborder\\b"
                     << "\\bwidth\\b" << "\\bheight\\b" << "\\bdisplay\\b"
                     << "\\bposition\\b" << "\\bfloat\\b" << "\\bclear\\b"
                     << "\\btext\\b" << "\\bline\\b" << "\\bvertical\\b"
                     << "\\bwhite\\b" << "\\bword\\b" << "\\bletter\\b"
                     << "\\bvisibility\\b" << "\\bopacity\\b" << "\\bz\\b"
                     << "\\boverflow\\b" << "\\bcursor\\b" << "\\boutline\\b"
                     << "\\btransition\\b" << "\\banimation\\b" << "\\btransform\\b"
                     << "\\bflex\\b" << "\\bgrid\\b" << "\\balign\\b"
                     << "\\bjustify\\b";

    for (const QString &pattern : propertyPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 选择器
    HighlightingRule selectorRule;
    selectorRule.pattern = QRegularExpression("[.#]?\\w+\\s*{");
    selectorRule.format = functionFormat;
    highlightingRules.append(selectorRule);

    // 值
    HighlightingRule valueRule;
    valueRule.pattern = QRegularExpression(":\\s*[^;]+");
    valueRule.format = classFormat;
    highlightingRules.append(valueRule);

    // 注释
    HighlightingRule commentRule;
    commentRule.pattern = QRegularExpression("/\\*.*\\*/");
    commentRule.format = singleLineCommentFormat;
    highlightingRules.append(commentRule);
}

void SyntaxHighlighter::initJavaRules()
{
    highlightingRules.clear();

    // Java关键字
    QStringList keywordPatterns;
    keywordPatterns << "\\babstract\\b" << "\\bassert\\b" << "\\bboolean\\b"
                    << "\\bbreak\\b" << "\\bbyte\\b" << "\\bcase\\b"
                    << "\\bcatch\\b" << "\\bchar\\b" << "\\bclass\\b"
                    << "\\bconst\\b" << "\\bcontinue\\b" << "\\bdefault\\b"
                    << "\\bdo\\b" << "\\bdouble\\b" << "\\belse\\b"
                    << "\\benum\\b" << "\\bextends\\b" << "\\bfinal\\b"
                    << "\\bfinally\\b" << "\\bfloat\\b" << "\\bfor\\b"
                    << "\\bgoto\\b" << "\\bif\\b" << "\\bimplements\\b"
                    << "\\bimport\\b" << "\\binstanceof\\b" << "\\bint\\b"
                    << "\\binterface\\b" << "\\blong\\b" << "\\bnative\\b"
                    << "\\bnew\\b" << "\\bpackage\\b" << "\\bprivate\\b"
                    << "\\bprotected\\b" << "\\bpublic\\b" << "\\breturn\\b"
                    << "\\bshort\\b" << "\\bstatic\\b" << "\\bstrictfp\\b"
                    << "\\bsuper\\b" << "\\bswitch\\b" << "\\bsynchronized\\b"
                    << "\\bthis\\b" << "\\bthrow\\b" << "\\bthrows\\b"
                    << "\\btransient\\b" << "\\btry\\b" << "\\bvoid\\b"
                    << "\\bvolatile\\b" << "\\bwhile\\b" << "\\btrue\\b"
                    << "\\bfalse\\b" << "\\bnull\\b";

    for (const QString &pattern : keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 单行注释
    HighlightingRule singleLineCommentRule;
    singleLineCommentRule.pattern = QRegularExpression("//[^\n]*");
    singleLineCommentRule.format = singleLineCommentFormat;
    highlightingRules.append(singleLineCommentRule);

    // 字符串
    HighlightingRule quotationRule;
    quotationRule.pattern = QRegularExpression("\".*?\"");
    quotationRule.format = quotationFormat;
    highlightingRules.append(quotationRule);

    // 函数名
    HighlightingRule functionRule;
    functionRule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    functionRule.format = functionFormat;
    highlightingRules.append(functionRule);

    // 数字
    HighlightingRule numberRule;
    numberRule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    numberRule.format = numberFormat;
    highlightingRules.append(numberRule);

    // 注解
    HighlightingRule annotationRule;
    annotationRule.pattern = QRegularExpression("@\\w+");
    annotationRule.format = preprocessorFormat;
    highlightingRules.append(annotationRule);
}
