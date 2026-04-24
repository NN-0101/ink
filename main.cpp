#include "mainwindow.h"
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序信息，用于保存设置
    QApplication::setApplicationName("TidyText");
    QApplication::setOrganizationName("TidyText");
    QApplication::setApplicationVersion("1.0.0");

    MainWindow w;

    // 处理命令行参数
    QStringList arguments = QApplication::arguments();

    // 第一个参数是程序自身，所以从索引1开始
    if (arguments.size() > 1) {
        // 可能有多个文件被选中（但右键菜单通常是一次打开一个文件）
        for (int i = 1; i < arguments.size(); ++i) {
            QString filePath = arguments[i];

            // 确保路径格式正确（有些时候传递的路径可能包含引号）
            filePath = filePath.trimmed();
            if (filePath.startsWith('"') && filePath.endsWith('"')) {
                filePath = filePath.mid(1, filePath.length() - 2);
            }

            // 检查文件是否存在
            QFileInfo fileInfo(filePath);
            if (fileInfo.exists() && fileInfo.isFile()) {
                // 使用 QTimer 确保窗口完全显示后再打开文件
                QTimer::singleShot(100, [&w, filePath]() {
                    w.openFileFromCommandLine(filePath);
                });
            }
        }
    }

    w.show();
    return a.exec();
}
