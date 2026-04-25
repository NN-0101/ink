// syntaxhighlighter.h
#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    enum FileType {
        Unknown,
        Cpp,
        Python,
        JavaScript,
        Markdown,
        HTML,
        CSS,
        Java,
        YAML,
        JSON,
        XML,
        Rust,
        Go,
        TypeScript,
        Shell,
        Batch,
        INI,
        SQL,
        CSharp,
        Swift,
        Kotlin
    };

    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);

    static FileType detectFileType(const QString &fileName);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    void initCppRules();
    void initPythonRules();
    void initJavaScriptRules();
    void initMarkdownRules();
    void initHTMLRules();
    void initCSSRules();
    void initJavaRules();

    void initYAMLRules();
    void initJSONRules();
    void initXMLRules();
    void initRustRules();
    void initGoRules();
    void initTypeScriptRules();
    void initShellRules();
    void initBatchRules();
    void initINIRules();
    void initSQLRules();
    void initCSharpRules();
    void initSwiftRules();
    void initKotlinRules();

    QVector<HighlightingRule> highlightingRules;

    // 格式定义
    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat markdownHeaderFormat;
    QTextCharFormat markdownBoldFormat;
    QTextCharFormat markdownItalicFormat;
    QTextCharFormat markdownCodeFormat;

    QTextCharFormat yamlKeyFormat;
    QTextCharFormat yamlValueFormat;
    QTextCharFormat jsonPropertyFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat attributeFormat;
    QTextCharFormat sqlKeywordFormat;
    QTextCharFormat sqlFunctionFormat;
    QTextCharFormat sqlTableFormat;
    QTextCharFormat shellCommandFormat;
    QTextCharFormat shellVariableFormat;
    QTextCharFormat iniSectionFormat;
    QTextCharFormat iniKeyFormat;
    QTextCharFormat iniValueFormat;
};

#endif // SYNTAXHIGHLIGHTER_H
