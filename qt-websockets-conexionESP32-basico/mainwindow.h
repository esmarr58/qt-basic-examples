#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QUrl>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void sendMessage();
    void onError(QAbstractSocket::SocketError error);

private:
    Ui::MainWindow *ui;
    QWebSocket *m_webSocket;
    QTextEdit *m_textEdit;
    QLineEdit *m_input;
    QString ipESP32 = "192.168.0.100";
};
#endif // MAINWINDOW_H
