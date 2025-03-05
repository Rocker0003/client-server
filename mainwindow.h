#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QMenu>
#include <QDateTime> // Include for QDateTime
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override; // Override to handle context menu

public slots:
    void onError(QAbstractSocket::SocketError socketError);  // Slot for handling socket errors.
    void connected();  // Slot for handling socket connection event.
    void disconnected();  // Slot for handling socket disconnection event.
    void bytesWritten(qint64 bytes);  // Slot for handling data written event in socket.

private slots:
    void on_pushButton_Add_clicked();
    void on_pushButton_Update_clicked();
    void on_tableWidget_cellDoubleClicked(int row, int column);
    void on_tableWidget_itemSelectionChanged();
    void on_deleteProductsAction_triggered(); // Slot for delete action
    void on_pushButton_Save_clicked();
    void on_pushButton_Show_clicked();
    void on_pushButton_Close_clicked();
    void Time_And_Date();      // Function to start the timer and update time and date
    void on_pushButton_Connect_To_Server_clicked();
    void on_pushButton_Send_Data_clicked();

private:
    Ui::MainWindow *ui;

    int selectedRow = -1; // Variable to track the selected row

    QMenu *contextMenu; // Context menu for deleting products
    QAction *deleteProductsAction; // Action for deleting products
    QTimer *timer;  // Timer to trigger updates
    QTcpSocket *socket;  // Socket object for TCP communication.

};
#endif // MAINWINDOW_H
