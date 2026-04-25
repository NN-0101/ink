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

    // YAML 格式
    yamlKeyFormat.setForeground(QColor("#C7254E"));
    yamlKeyFormat.setFontWeight(QFont::Bold);
    yamlValueFormat.setForeground(QColor("#005CC5"));

    // JSON 格式
    jsonPropertyFormat.setForeground(QColor("#22863A"));
    jsonPropertyFormat.setFontWeight(QFont::Bold);
    stringFormat.setForeground(QColor("#032F62"));

    // 操作符格式
    operatorFormat.setForeground(QColor("#D73A49"));
    operatorFormat.setFontWeight(QFont::Bold);

    // 类型格式
    typeFormat.setForeground(QColor("#6F42C1"));
    typeFormat.setFontWeight(QFont::Bold);

    // 属性格式
    attributeFormat.setForeground(QColor("#E36209"));

    // SQL 格式
    sqlKeywordFormat.setForeground(Qt::darkBlue);
    sqlKeywordFormat.setFontWeight(QFont::Bold);
    sqlFunctionFormat.setForeground(Qt::darkMagenta);
    sqlTableFormat.setForeground(QColor("#6F42C1"));

    // Shell 格式
    shellCommandFormat.setForeground(QColor("#24292E"));
    shellCommandFormat.setFontWeight(QFont::Bold);
    shellVariableFormat.setForeground(QColor("#005CC5"));

    // INI 格式
    iniSectionFormat.setForeground(QColor("#22863A"));
    iniSectionFormat.setFontWeight(QFont::Bold);
    iniKeyFormat.setForeground(QColor("#6F42C1"));
    iniValueFormat.setForeground(QColor("#032F62"));
}

SyntaxHighlighter::FileType SyntaxHighlighter::detectFileType(const QString &fileName)
{
    QString suffix = fileName.mid(fileName.lastIndexOf('.') + 1).toLower();

    // C/C++
    if (suffix == "cpp" || suffix == "cc" || suffix == "cxx" || suffix == "c++" ||
        suffix == "h" || suffix == "hh" || suffix == "hpp" || suffix == "hxx" ||
        suffix == "c") {
        return Cpp;
    }
    // Python
    else if (suffix == "py" || suffix == "pyw" || suffix == "pyi") {
        return Python;
    }
    // JavaScript/TypeScript
    else if (suffix == "js" || suffix == "jsx" || suffix == "mjs") {
        return JavaScript;
    }
    else if (suffix == "ts" || suffix == "tsx") {
        return TypeScript;
    }
    // Markdown
    else if (suffix == "md" || suffix == "markdown" || suffix == "mdx") {
        return Markdown;
    }
    // HTML
    else if (suffix == "html" || suffix == "htm" || suffix == "xhtml" || suffix == "shtml") {
        return HTML;
    }
    // CSS
    else if (suffix == "css" || suffix == "scss" || suffix == "sass" || suffix == "less") {
        return CSS;
    }
    // Java
    else if (suffix == "java") {
        return Java;
    }
    // YAML
    else if (suffix == "yaml" || suffix == "yml") {
        return YAML;
    }
    // JSON
    else if (suffix == "json" || suffix == "jsonc" || suffix == "json5") {
        return JSON;
    }
    // XML
    else if (suffix == "xml" || suffix == "xsl" || suffix == "xsd" || suffix == "svg") {
        return XML;
    }
    // Rust
    else if (suffix == "rs") {
        return Rust;
    }
    // Go
    else if (suffix == "go") {
        return Go;
    }
    // Shell
    else if (suffix == "sh" || suffix == "bash" || suffix == "zsh" || suffix == "fish") {
        return Shell;
    }
    // Batch/CMD
    else if (suffix == "bat" || suffix == "cmd") {
        return Batch;
    }
    // INI/Config
    else if (suffix == "ini" || suffix == "cfg" || suffix == "conf" || suffix == "config") {
        return INI;
    }
    // SQL
    else if (suffix == "sql") {
        return SQL;
    }
    // C#
    else if (suffix == "cs" || suffix == "csx") {
        return CSharp;
    }
    // Swift
    else if (suffix == "swift") {
        return Swift;
    }
    // Kotlin
    else if (suffix == "kt" || suffix == "kts") {
        return Kotlin;
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
        case TypeScript:
            initTypeScriptRules();
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
        case YAML:
            initYAMLRules();
            break;
        case JSON:
            initJSONRules();
            break;
        case XML:
            initXMLRules();
            break;
        case Rust:
            initRustRules();
            break;
        case Go:
            initGoRules();
            break;
        case Shell:
            initShellRules();
            break;
        case Batch:
            initBatchRules();
            break;
        case INI:
            initINIRules();
            break;
        case SQL:
            initSQLRules();
            break;
        case CSharp:
            initCSharpRules();
            break;
        case Swift:
            initSwiftRules();
            break;
        case Kotlin:
            initKotlinRules();
            break;
        default:
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

    // 处理多行注释
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

// ==================== 新增的高亮初始化方法 ====================

void SyntaxHighlighter::initYAMLRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // YAML 注释
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // YAML 键（冒号前的部分）
    rule.pattern = QRegularExpression("^\\s*[\\w-]+(?=\\s*:)");
    rule.format = yamlKeyFormat;
    highlightingRules.append(rule);

    // YAML 键值对中的值
    rule.pattern = QRegularExpression("(?::\\s)(true|false|yes|no|null|~)");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // YAML 字符串值
    rule.pattern = QRegularExpression("\"[^\"]*\"|'[^']*'");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // YAML 数字
    rule.pattern = QRegularExpression(":\\s*\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // YAML 列表项
    rule.pattern = QRegularExpression("^\\s*-\\s");
    rule.format = operatorFormat;
    highlightingRules.append(rule);

    // YAML 锚点和别名
    rule.pattern = QRegularExpression("&\\w+|\\*\\w+");
    rule.format = typeFormat;
    highlightingRules.append(rule);

    // YAML 标签
    rule.pattern = QRegularExpression("!!\\w+");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initJSONRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // JSON 属性名
    rule.pattern = QRegularExpression("\"[^\"]*\"\\s*:");
    rule.format = jsonPropertyFormat;
    highlightingRules.append(rule);

    // JSON 字符串值
    rule.pattern = QRegularExpression(":\\s*\"[^\"]*\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // JSON 数字
    rule.pattern = QRegularExpression(":\\s*-?\\b\\d+(\\.\\d+)?([eE][+-]?\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // JSON 布尔值和 null
    rule.pattern = QRegularExpression(":\\s*(true|false|null)\\b");
    rule.format = keywordFormat;
    highlightingRules.append(rule);

    // JSON 括号和花括号
    rule.pattern = QRegularExpression("[\\{\\}\\[\\]]");
    rule.format = operatorFormat;
    highlightingRules.append(rule);

    // JSON 注释 (jsonc)
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initXMLRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // XML 注释
    rule.pattern = QRegularExpression("<!--[\\s\\S]*?-->");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // XML 标签
    rule.pattern = QRegularExpression("</?\\w+[^>]*/?>");
    rule.format = keywordFormat;
    highlightingRules.append(rule);

    // XML 属性名
    rule.pattern = QRegularExpression("\\b\\w+(?=\\s*=)");
    rule.format = attributeFormat;
    highlightingRules.append(rule);

    // XML 属性值
    rule.pattern = QRegularExpression("\"[^\"]*\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // XML 声明
    rule.pattern = QRegularExpression("<\\?[^?]*\\?>");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // XML 实体引用
    rule.pattern = QRegularExpression("&\\w+;");
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initRustRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // Rust 关键字
    QStringList keywords;
    keywords << "\\bas\\b" << "\\bbreak\\b" << "\\bconst\\b" << "\\bcontinue\\b"
             << "\\bcrate\\b" << "\\belse\\b" << "\\benum\\b" << "\\bextern\\b"
             << "\\bfalse\\b" << "\\bfn\\b" << "\\bfor\\b" << "\\bif\\b"
             << "\\bimpl\\b" << "\\bin\\b" << "\\blet\\b" << "\\bloop\\b"
             << "\\bmatch\\b" << "\\bmod\\b" << "\\bmove\\b" << "\\bmut\\b"
             << "\\bpub\\b" << "\\bref\\b" << "\\breturn\\b" << "\\bself\\b"
             << "\\bSelf\\b" << "\\bstatic\\b" << "\\bstruct\\b" << "\\bsuper\\b"
             << "\\btrait\\b" << "\\btrue\\b" << "\\btype\\b" << "\\bunsafe\\b"
             << "\\buse\\b" << "\\bwhere\\b" << "\\bwhile\\b" << "\\basync\\b"
             << "\\bawait\\b" << "\\bdyn\\b";

    for (const QString &pattern : keywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Rust 类型
    rule.pattern = QRegularExpression("\\b(i8|i16|i32|i64|i128|u8|u16|u32|u64|u128|f32|f64|bool|char|str|String|Vec|Option|Result)\\b");
    rule.format = typeFormat;
    highlightingRules.append(rule);

    // 单行注释
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 字符串
    rule.pattern = QRegularExpression("\"[^\"]*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 宏
    rule.pattern = QRegularExpression("\\w+!");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // 数字
    rule.pattern = QRegularExpression("\\b\\d+(_?\\d+)*(\\.\\d+)?(f32|f64|i32|i64|u32|u64)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 生命周期
    rule.pattern = QRegularExpression("'\\w+");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // 属性
    rule.pattern = QRegularExpression("#\\!?\\[\\w+[^\\]]*\\]");
    rule.format = attributeFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initGoRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // Go 关键字
    QStringList keywords;
    keywords << "\\bbreak\\b" << "\\bcase\\b" << "\\bchan\\b" << "\\bconst\\b"
             << "\\bcontinue\\b" << "\\bdefault\\b" << "\\bdefer\\b" << "\\belse\\b"
             << "\\bfallthrough\\b" << "\\bfor\\b" << "\\bfunc\\b" << "\\bgo\\b"
             << "\\bgoto\\b" << "\\bif\\b" << "\\bimport\\b" << "\\binterface\\b"
             << "\\bmap\\b" << "\\bpackage\\b" << "\\brange\\b" << "\\breturn\\b"
             << "\\bselect\\b" << "\\bstruct\\b" << "\\bswitch\\b" << "\\btype\\b"
             << "\\bvar\\b";

    for (const QString &pattern : keywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Go 内置类型
    rule.pattern = QRegularExpression("\\b(bool|byte|complex64|complex128|error|float32|float64|int|int8|int16|int32|int64|rune|string|uint|uint8|uint16|uint32|uint64|uintptr)\\b");
    rule.format = typeFormat;
    highlightingRules.append(rule);

    // 单行注释
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 字符串
    rule.pattern = QRegularExpression("\"[^\"]*\"|`[^`]*`");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 函数调用
    rule.pattern = QRegularExpression("\\b\\w+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // 数字
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?([eE][+-]?\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initTypeScriptRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // TypeScript 关键字
    QStringList keywords;
    keywords << "\\babstract\\b" << "\\bany\\b" << "\\bas\\b" << "\\bboolean\\b"
             << "\\bbreak\\b" << "\\bcase\\b" << "\\bcatch\\b" << "\\bclass\\b"
             << "\\bconst\\b" << "\\bconstructor\\b" << "\\bcontinue\\b" << "\\bdebugger\\b"
             << "\\bdeclare\\b" << "\\bdefault\\b" << "\\bdelete\\b" << "\\bdo\\b"
             << "\\belse\\b" << "\\benum\\b" << "\\bexport\\b" << "\\bextends\\b"
             << "\\bfalse\\b" << "\\bfinally\\b" << "\\bfor\\b" << "\\bfrom\\b"
             << "\\bfunction\\b" << "\\bif\\b" << "\\bimplements\\b" << "\\bimport\\b"
             << "\\bin\\b" << "\\binfer\\b" << "\\binstanceof\\b" << "\\binterface\\b"
             << "\\bkeyof\\b" << "\\blet\\b" << "\\bmodule\\b" << "\\bnamespace\\b"
             << "\\bnever\\b" << "\\bnew\\b" << "\\bnull\\b" << "\\bnumber\\b"
             << "\\bobject\\b" << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
             << "\\breadonly\\b" << "\\breturn\\b" << "\\bstatic\\b" << "\\bstring\\b"
             << "\\bsuper\\b" << "\\bswitch\\b" << "\\bthis\\b" << "\\bthrow\\b"
             << "\\btrue\\b" << "\\btry\\b" << "\\btype\\b" << "\\btypeof\\b"
             << "\\bundefined\\b" << "\\bvar\\b" << "\\bvoid\\b" << "\\bwhile\\b"
             << "\\bwith\\b" << "\\byield\\b" << "\\basync\\b" << "\\bawait\\b";

    for (const QString &pattern : keywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // TypeScript 类型
    rule.pattern = QRegularExpression(":\\s*(string|number|boolean|void|any|never|unknown|object|Array|Promise)\\b");
    rule.format = typeFormat;
    highlightingRules.append(rule);

    // 单行注释
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 字符串
    rule.pattern = QRegularExpression("'[^']*'|\"[^\"]*\"|`[^`]*`");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 函数调用
    rule.pattern = QRegularExpression("\\b[A-Za-z0]\\w*(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // 数字
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 装饰器
    rule.pattern = QRegularExpression("@\\w+");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initShellRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // Shell 注释
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Shell 命令
    QStringList commands;
    commands << "\\becho\\b" << "\\bexit\\b" << "\\bexport\\b" << "\\bsource\\b"
             << "\\bexec\\b" << "\\bread\\b" << "\\bcd\\b" << "\\bpwd\\b"
             << "\\bls\\b" << "\\bcat\\b" << "\\bgrep\\b" << "\\bsed\\b"
             << "\\bawk\\b" << "\\bchmod\\b" << "\\bchown\\b" << "\\bmkdir\\b"
             << "\\brm\\b" << "\\bcp\\b" << "\\bmv\\b" << "\\btouch\\b"
             << "\\bfind\\b" << "\\btar\\b" << "\\bgzip\\b" << "\\bssh\\b"
             << "\\bscp\\b" << "\\bcurl\\b" << "\\bwget\\b" << "\\bsudo\\b"
             << "\\bdocker\\b" << "\\bgit\\b" << "\\bnpm\\b" << "\\bnode\\b"
             << "\\bpython\\b" << "\\bjava\\b";

    for (const QString &pattern : commands) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = shellCommandFormat;
        highlightingRules.append(rule);
    }

    // Shell 变量
    rule.pattern = QRegularExpression("\\$[A-Za-z_]\\w*|\\$\\{[^}]+\\}");
    rule.format = shellVariableFormat;
    highlightingRules.append(rule);

    // 字符串
    rule.pattern = QRegularExpression("\"[^\"]*\"|'[^']*'");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // Shell 控制结构
    rule.pattern = QRegularExpression("\\b(if|then|else|elif|fi|case|esac|for|while|until|do|done|in|function)\\b");
    rule.format = keywordFormat;
    highlightingRules.append(rule);

    // Shell 操作符
    rule.pattern = QRegularExpression("[|&;<>]|&&|\\|\\|");
    rule.format = operatorFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initBatchRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // Batch 注释
    rule.pattern = QRegularExpression("(?:^|\\s)rem\\s.*$|::[^\n]*", QRegularExpression::CaseInsensitiveOption);
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Batch 命令
    QStringList commands;
    commands << "\\becho\\b" << "\\bset\\b" << "\\bif\\b" << "\\belse\\b"
             << "\\bfor\\b" << "\\bdo\\b" << "\\bgoto\\b" << "\\bcall\\b"
             << "\\bexit\\b" << "\\bpause\\b" << "\\bcd\\b" << "\\bdir\\b"
             << "\\bcopy\\b" << "\\bdel\\b" << "\\bren\\b" << "\\bmd\\b"
             << "\\bcls\\b" << "\\btype\\b" << "\\bfind\\b" << "\\bfindstr\\b"
             << "\\bstart\\b" << "\\btitle\\b" << "\\bcolor\\b" << "\\bver\\b";

    for (const QString &pattern : commands) {
        rule.pattern = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        rule.format = shellCommandFormat;
        highlightingRules.append(rule);
    }

    // Batch 变量
    rule.pattern = QRegularExpression("%[A-Za-z_]\\w*%|%\\d|%[^%]*%");
    rule.format = shellVariableFormat;
    highlightingRules.append(rule);

    // Batch 标签
    rule.pattern = QRegularExpression(":[A-Za-z]\\w*");
    rule.format = typeFormat;
    highlightingRules.append(rule);

    // 字符串
    rule.pattern = QRegularExpression("\"[^\"]*\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // 操作符
    rule.pattern = QRegularExpression("[><]=?|==|!=|\\|\\||&&");
    rule.format = operatorFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initINIRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // INI 注释
    rule.pattern = QRegularExpression("[;#][^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 节名
    rule.pattern = QRegularExpression("^\\s*\\[[^\\]]+\\]");
    rule.format = iniSectionFormat;
    highlightingRules.append(rule);

    // 键名
    rule.pattern = QRegularExpression("^\\s*[^\\s=;\\#]+(?=\\s*=)");
    rule.format = iniKeyFormat;
    highlightingRules.append(rule);

    // 等号后的值
    rule.pattern = QRegularExpression("(?<=\\=).*$");
    rule.format = iniValueFormat;
    highlightingRules.append(rule);

    // 布尔值
    rule.pattern = QRegularExpression("=\\s*(true|false|yes|no|on|off)\\b", QRegularExpression::CaseInsensitiveOption);
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 数字
    rule.pattern = QRegularExpression("=\\s*\\d+");
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initSQLRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // SQL 关键字
    QStringList keywords;
    keywords << "\\bSELECT\\b" << "\\bFROM\\b" << "\\bWHERE\\b" << "\\bINSERT\\b"
             << "\\bUPDATE\\b" << "\\bDELETE\\b" << "\\bCREATE\\b" << "\\bALTER\\b"
             << "\\bDROP\\b" << "\\bTABLE\\b" << "\\bINDEX\\b" << "\\bVIEW\\b"
             << "\\bINTO\\b" << "\\bVALUES\\b" << "\\bSET\\b" << "\\bJOIN\\b"
             << "\\bLEFT\\b" << "\\bRIGHT\\b" << "\\bINNER\\b" << "\\bOUTER\\b"
             << "\\bON\\b" << "\\bAND\\b" << "\\bOR\\b" << "\\bNOT\\b"
             << "\\bNULL\\b" << "\\bIS\\b" << "\\bLIKE\\b" << "\\bIN\\b"
             << "\\bBETWEEN\\b" << "\\bORDER\\b" << "\\bBY\\b" << "\\bASC\\b"
             << "\\bDESC\\b" << "\\bGROUP\\b" << "\\bHAVING\\b" << "\\bLIMIT\\b"
             << "\\bOFFSET\\b" << "\\bUNION\\b" << "\\bALL\\b" << "\\bDISTINCT\\b"
             << "\\bEXISTS\\b" << "\\bCASE\\b" << "\\bWHEN\\b" << "\\bTHEN\\b"
             << "\\bELSE\\b" << "\\bEND\\b" << "\\bAS\\b" << "\\bPRIMARY\\b"
             << "\\bKEY\\b" << "\\bFOREIGN\\b" << "\\bREFERENCES\\b" << "\\bCONSTRAINT\\b"
             << "\\bDEFAULT\\b" << "\\bUNIQUE\\b" << "\\bCHECK\\b" << "\\bIF\\b"
             << "\\bBEGIN\\b" << "\\bCOMMIT\\b" << "\\bROLLBACK\\b" << "\\bTRANSACTION\\b";

    for (const QString &pattern : keywords) {
        rule.pattern = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        rule.format = sqlKeywordFormat;
        highlightingRules.append(rule);
    }

    // SQL 函数
    QStringList functions;
    functions << "\\bCOUNT\\b" << "\\bSUM\\b" << "\\bAVG\\b" << "\\bMIN\\b" << "\\bMAX\\b"
              << "\\bCOALESCE\\b" << "\\bNULLIF\\b" << "\\bCAST\\b" << "\\bCONVERT\\b"
              << "\\bCONCAT\\b" << "\\bSUBSTRING\\b" << "\\bLENGTH\\b" << "\\bUPPER\\b"
              << "\\bLOWER\\b" << "\\bTRIM\\b" << "\\bNOW\\b" << "\\bCURDATE\\b";

    for (const QString &pattern : functions) {
        rule.pattern = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        rule.format = sqlFunctionFormat;
        highlightingRules.append(rule);
    }

    // SQL 注释
    rule.pattern = QRegularExpression("--[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // SQL 字符串
    rule.pattern = QRegularExpression("'[^']*'");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // SQL 数字
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initCSharpRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // C# 关键字
    QStringList keywords;
    keywords << "\\babstract\\b" << "\\bas\\b" << "\\bbase\\b" << "\\bbool\\b"
             << "\\bbreak\\b" << "\\bbyte\\b" << "\\bcase\\b" << "\\bcatch\\b"
             << "\\bchar\\b" << "\\bchecked\\b" << "\\bclass\\b" << "\\bconst\\b"
             << "\\bcontinue\\b" << "\\bdecimal\\b" << "\\bdefault\\b" << "\\bdelegate\\b"
             << "\\bdo\\b" << "\\bdouble\\b" << "\\belse\\b" << "\\benum\\b"
             << "\\bevent\\b" << "\\bexplicit\\b" << "\\bextern\\b" << "\\bfalse\\b"
             << "\\bfinally\\b" << "\\bfixed\\b" << "\\bfloat\\b" << "\\bfor\\b"
             << "\\bforeach\\b" << "\\bgoto\\b" << "\\bif\\b" << "\\bimplicit\\b"
             << "\\bin\\b" << "\\bint\\b" << "\\binterface\\b" << "\\binternal\\b"
             << "\\bis\\b" << "\\block\\b" << "\\blong\\b" << "\\bnamespace\\b"
             << "\\bnew\\b" << "\\bnull\\b" << "\\bobject\\b" << "\\boperator\\b"
             << "\\bout\\b" << "\\boverride\\b" << "\\bparams\\b" << "\\bprivate\\b"
             << "\\bprotected\\b" << "\\bpublic\\b" << "\\breadonly\\b" << "\\bref\\b"
             << "\\breturn\\b" << "\\bsbyte\\b" << "\\bsealed\\b" << "\\bshort\\b"
             << "\\bsizeof\\b" << "\\bstackalloc\\b" << "\\bstatic\\b" << "\\bstring\\b"
             << "\\bstruct\\b" << "\\bswitch\\b" << "\\bthis\\b" << "\\bthrow\\b"
             << "\\btrue\\b" << "\\btry\\b" << "\\btypeof\\b" << "\\buint\\b"
             << "\\bulong\\b" << "\\bunchecked\\b" << "\\bunsafe\\b" << "\\bushort\\b"
             << "\\busing\\b" << "\\bvar\\b" << "\\bvirtual\\b" << "\\bvoid\\b"
             << "\\bvolatile\\b" << "\\bwhile\\b" << "\\badd\\b" << "\\bremove\\b"
             << "\\bget\\b" << "\\bset\\b" << "\\bvalue\\b" << "\\bwhere\\b"
             << "\\byield\\b" << "\\basync\\b" << "\\bawait\\b";

    for (const QString &pattern : keywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // C# 类型
    rule.pattern = QRegularExpression("\\b(string|int|double|float|bool|char|var|void|object|dynamic|DateTime|List|Dictionary|Task)\\b");
    rule.format = typeFormat;
    highlightingRules.append(rule);

    // 单行注释
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 字符串
    rule.pattern = QRegularExpression("\"[^\"]*\"|\\$\"[^\"]*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 函数调用
    rule.pattern = QRegularExpression("\\b[A-Za-z_]\\w*(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // 数字
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?(f|d|m)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 预处理指令
    rule.pattern = QRegularExpression("^\\s*#\\w+");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // 属性
    rule.pattern = QRegularExpression("\\[\\w+(\\([^\\)]*\\))?\\]");
    rule.format = attributeFormat;
    highlightingRules.append(rule);

    // LINQ 关键字
    rule.pattern = QRegularExpression("\\b(from|select|where|orderby|group|into|join|let|ascending|descending)\\b");
    rule.format = sqlKeywordFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initSwiftRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // Swift 关键字
    QStringList keywords;
    keywords << "\\bclass\\b" << "\\bstruct\\b" << "\\benum\\b" << "\\bprotocol\\b"
             << "\\bextension\\b" << "\\bfunc\\b" << "\\bvar\\b" << "\\blet\\b"
             << "\\bif\\b" << "\\belse\\b" << "\\bguard\\b" << "\\bswitch\\b"
             << "\\bcase\\b" << "\\bdefault\\b" << "\\bfor\\b" << "\\bin\\b"
             << "\\bwhile\\b" << "\\brepeat\\b" << "\\bdo\\b" << "\\bcatch\\b"
             << "\\btry\\b" << "\\bthrow\\b" << "\\bthrows\\b" << "\\basync\\b"
             << "\\bawait\\b" << "\\bimport\\b" << "\\breturn\\b" << "\\bbreak\\b"
             << "\\bcontinue\\b" << "\\bfallthrough\\b" << "\\bwhere\\b" << "\\bis\\b"
             << "\\bas\\b" << "\\bself\\b" << "\\bSelf\\b" << "\\bsuper\\b"
             << "\\binit\\b" << "\\bdeinit\\b" << "\\bconvenience\\b" << "\\brequired\\b"
             << "\\boptional\\b" << "\\blazy\\b" << "\\bweak\\b" << "\\bstrong\\b"
             << "\\bunowned\\b" << "\\bpublic\\b" << "\\bprivate\\b" << "\\b fileprivate\\b"
             << "\\binternal\\b" << "\\bopen\\b" << "\\bfinal\\b" << "\\bmutating\\b"
             << "\\bnonmutating\\b" << "\\bstatic\\b" << "\\bclass\\b";

    for (const QString &pattern : keywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Swift 类型
    QStringList types;
    types << "\\bInt\\b" << "\\bDouble\\b" << "\\bFloat\\b" << "\\bBool\\b"
          << "\\bString\\b" << "\\bCharacter\\b" << "\\bVoid\\b" << "\\bArray\\b"
          << "\\bDictionary\\b" << "\\bSet\\b" << "\\bOptional\\b" << "\\bAny\\b"
          << "\\bAnyObject\\b";

    for (const QString &pattern : types) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = typeFormat;
        highlightingRules.append(rule);
    }

    // 单行注释
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 字符串
    rule.pattern = QRegularExpression("\"[^\"]*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 函数调用
    rule.pattern = QRegularExpression("\\b[A-Za-z_]\\w*(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // 数字
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 预处理指令
    rule.pattern = QRegularExpression("^\\s*#\\w+");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // 属性包装器
    rule.pattern = QRegularExpression("@\\w+");
    rule.format = attributeFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initKotlinRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    // Kotlin 关键字
    QStringList keywords;
    keywords << "\\bfun\\b" << "\\bval\\b" << "\\bvar\\b" << "\\bclass\\b"
             << "\\binterface\\b" << "\\bobject\\b" << "\\bdata\\b" << "\\bsealed\\b"
             << "\\benum\\b" << "\\babstract\\b" << "\\bfinal\\b" << "\\bopen\\b"
             << "\\boverride\\b" << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
             << "\\binternal\\b" << "\\blateinit\\b" << "\\bvararg\\b" << "\\bnoinline\\b"
             << "\\bcrossinline\\b" << "\\boperator\\b" << "\\binfix\\b" << "\\btailrec\\b"
             << "\\bexternal\\b" << "\\bannotation\\b" << "\\bcompanion\\b" << "\\bconst\\b"
             << "\\bif\\b" << "\\belse\\b" << "\\bwhen\\b" << "\\bfor\\b"
             << "\\bwhile\\b" << "\\bdo\\b" << "\\btry\\b" << "\\bcatch\\b"
             << "\\bfinally\\b" << "\\bthrow\\b" << "\\breturn\\b" << "\\bcontinue\\b"
             << "\\bbreak\\b" << "\\bin\\b" << "\\bis\\b" << "\\bas\\b"
             << "\\bwhere\\b" << "\\bthis\\b" << "\\bsuper\\b" << "\\binit\\b"
             << "\\bconstructor\\b" << "\\bby\\b" << "\\bdelegate\\b" << "\\bimport\\b"
             << "\\bpackage\\b" << "\\btrue\\b" << "\\bfalse\\b" << "\\bnull\\b"
             << "\\btypealias\\b" << "\\bexpect\\b" << "\\bactual\\b" << "\\b Suspend\\b";

    for (const QString &pattern : keywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Kotlin 类型
    QStringList types;
    types << "\\bInt\\b" << "\\bLong\\b" << "\\bDouble\\b" << "\\bFloat\\b"
          << "\\bBoolean\\b" << "\\bChar\\b" << "\\bString\\b" << "\\bArray\\b"
          << "\\bList\\b" << "\\bMutableList\\b" << "\\bSet\\b" << "\\bMutableSet\\b"
          << "\\bMap\\b" << "\\bMutableMap\\b" << "\\bUnit\\b" << "\\bNothing\\b"
          << "\\bAny\\b";

    for (const QString &pattern : types) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = typeFormat;
        highlightingRules.append(rule);
    }

    // 单行注释
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 字符串
    rule.pattern = QRegularExpression("\"[^\"]*\"|\"\"\"[\\s\\S]*?\"\"\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 函数调用
    rule.pattern = QRegularExpression("\\b[A-Za-z_]\\w*(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // 数字
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?(L|f)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 注解
    rule.pattern = QRegularExpression("@\\w+(:\\([^\\)]*\\))?");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // 字符串模板
    rule.pattern = QRegularExpression("\\$[A-Za-z_]\\w*|\\$\\{[^}]+\\}");
    rule.format = shellVariableFormat;
    highlightingRules.append(rule);
}

// ==================== 原有方法保持不变 ====================

void SyntaxHighlighter::initCppRules()
{
    highlightingRules.clear();

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

    HighlightingRule rule;
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\".*?\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("'.*?'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("^\\s*#.*");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initPythonRules()
{
    highlightingRules.clear();

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

    HighlightingRule rule;
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    rule.pattern = QRegularExpression("\\bclass\\s+(\\w+)");
    rule.format = classFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("'[^']*'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\"[^\"]*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("'''.*?'''");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\"\"\".*?\"\"\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\bdef\\s+(\\w+)");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("@\\w+");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\bself\\b");
    rule.format = functionFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initJavaScriptRules()
{
    highlightingRules.clear();

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

    HighlightingRule rule;
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("/\\*");
    rule.format = multiLineCommentFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\*/");
    rule.format = multiLineCommentFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("'[^']*'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\"[^\"]*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("`[^`]*`");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\bfunction\\s+(\\w+)");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("/([^/\\\\]*(?:\\\\.[^/\\\\]*)*)/[gimsuy]*");
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initMarkdownRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    rule.pattern = QRegularExpression("^#{1}\\s+(.*)$");
    rule.format = markdownHeaderFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("^#{2}\\s+(.*)$");
    rule.format = markdownHeaderFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("^#{3}\\s+(.*)$");
    rule.format = markdownHeaderFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("^#{4}\\s+(.*)$");
    rule.format = markdownHeaderFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("^#{5}\\s+(.*)$");
    rule.format = markdownHeaderFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("^#{6}\\s+(.*)$");
    rule.format = markdownHeaderFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\*\\*[^*]*\\*\\*");
    rule.format = markdownBoldFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("__[^_]*__");
    rule.format = markdownBoldFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\*[^*]*\\*");
    rule.format = markdownItalicFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("_[^_]*_");
    rule.format = markdownItalicFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("`[^`]*`");
    rule.format = markdownCodeFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\[[^\\]]*\\]\\([^\\)]*\\)");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("^\\s*[-*+]\\s+");
    rule.format = keywordFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("^>\\s+.*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initHTMLRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

    rule.pattern = QRegularExpression("<[^>]*>");
    rule.format = keywordFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\b\\w+\\s*=");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("<!--.*-->");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initCSSRules()
{
    highlightingRules.clear();

    HighlightingRule rule;

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
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    rule.pattern = QRegularExpression("[.#]?\\w+\\s*{");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(":\\s*[^;]+");
    rule.format = classFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("/\\*.*\\*/");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::initJavaRules()
{
    highlightingRules.clear();

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

    HighlightingRule rule;
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\".*?\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("@\\w+");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);
}
