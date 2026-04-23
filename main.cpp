/**
 * @file main.cpp
 * @brief 程序主入口文件
 * 
 * 这个文件包含程序的 main 函数，是应用程序的启动点。
 * 它创建 QApplication 实例和主窗口，然后启动事件循环。
 */

#include "mainwindow.h"
#include <QApplication>

/**
 * @brief 程序主入口函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 程序退出码
 * 
 * 函数执行流程：
 * 1. 创建 QApplication 实例，初始化 Qt 应用程序框架
 * 2. 创建 MainWindow 实例，构建主窗口
 * 3. 显示主窗口
 * 4. 启动 Qt 事件循环，等待用户交互
 * 
 * Qt 事件循环会处理用户输入、定时器、网络事件等，
 * 直到应用程序退出（如关闭主窗口）。
 */
int main(int argc, char *argv[])
{
    // 创建 QApplication 实例，初始化 Qt 应用程序
    QApplication a(argc, argv);
    
    // 创建主窗口实例
    MainWindow w;
    
    // 显示主窗口
    w.show();
    
    // 启动 Qt 事件循环，等待用户交互
    return a.exec();
}
