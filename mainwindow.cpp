#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql>
#include "QContextMenuEvent"
#include <QMessageBox>
#include <QFileDialog>
#include <QTcpSocket>
#include <QtNetwork>
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , selectedRow(-1) // Initialize selectedRow here
{
    ui->setupUi(this);

    socket = new QTcpSocket();  // Creating a new QTcpSocket object.

    // Connecting socket signals to corresponding slots.
    connect(socket, SIGNAL(connected()),this, SLOT(connected()));
    connect(socket, SIGNAL(bytesWritten(qint64)),this, SLOT(bytesWritten(qint64)));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    qDebug() <<"Connecting";  // Debug message indicating connection attempt.


    Time_And_Date(); // Call the function to initialize the real-time clock and date

    void on_tableWidget_cellDoubleClicked(int row, int column);

        QSqlDatabase mydb=QSqlDatabase::addDatabase("QSQLITE");
        mydb.setDatabaseName("C:/Users/rakes/OneDrive/Desktop/QT files/ClientAndServer/ClientAndServer.db");

        if(!mydb.open())
        {
            ui->label_Database_Status->setText("Failed to connect the database");
             qDebug() << "Error: connection with database failed";
        }
        else
        {
            ui->label_Database_Status->setText("Connected to database.....");
            qDebug() << "Database: connection ok";
        }


            // Set up the table widget with appropriate headers
            ui->tableWidget->setColumnCount(4);
            QStringList headers = {"Product_Name", "Volume", "Market_Capital", "Credit_Rating"};
            ui->tableWidget->setHorizontalHeaderLabels(headers);

            // Allow selecting multiple rows
            ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
            ui->tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);

            // Create the context menu and add the "Delete Products" action
            contextMenu = new QMenu(this);
            QAction *deleteProductsAction = new QAction("Delete Products", this);
            contextMenu->addAction(deleteProductsAction);

//            // Manually connect the delete action to the slot
//            connect(ui->deleteProductsAction, &QAction::triggered, this, &MainWindow::on_deleteProductsAction_triggered);

           // Connect the action to the slot for Delete Product Action
           connect(deleteProductsAction, &QAction::triggered, this, &MainWindow::on_deleteProductsAction_triggered);

           // Connect the cellDoubleClicked signal to the on_tableWidget_cellDoubleClicked slot
           connect(ui->tableWidget, &QTableWidget::cellDoubleClicked, this, &MainWindow::on_tableWidget_cellDoubleClicked);

}

MainWindow::~MainWindow()
{
    delete ui;

    delete contextMenu; // Ensure we delete the context menu to avoid memory leaks
}


void MainWindow::Time_And_Date()
{
    // Create a timer to update the time and date every second
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        // Get the current date and time
        QDateTime currentDateTime = QDateTime::currentDateTime();

        // Update labels with current time and date
        ui->label_Time->setText(currentDateTime.toString("HH:mm:ss"));  // Time in HH:mm:ss format
        ui->label_Date->setText(currentDateTime.toString("dd-MM-yyyy")); // Date in dd-MM-yyyy format
    });

    // Start the timer with a 1-second interval
    timer->start(1000);
}


void MainWindow::on_pushButton_Add_clicked()
{
        // Get the values from the text boxes
        QString Product_Name = ui->lineEdit_Product_Name->text();
        QString Volume = ui->lineEdit_Volume->text();
        QString Market_Capital = ui->lineEdit_Market_Capital->text();
        QString Credit_Rating = ui->lineEdit_Credit_Rating->text();

        // Check if any fields are empty
        if (Product_Name.isEmpty() || Volume.isEmpty() || Market_Capital.isEmpty() || Credit_Rating.isEmpty())
        {
            QMessageBox::warning(this, "Input Error", "All fields must be filled.");  // Show an error message
            return;
        }

        // 1. Product Name: Alphanumeric characters with at least one letter and one digit
            QRegExp alphanumericRegex("^(?=.*[a-zA-Z])(?=.*\\d)[a-zA-Z0-9]+$");
            QRegExpValidator alphanumericValidator(alphanumericRegex, this);
            int pos = 0;
            if (alphanumericValidator.validate(Product_Name, pos) != QValidator::Acceptable)
            {
                QMessageBox::warning(this, "Product Name Input Error", "Product Name should contain both letters and numeric characters (e.g., test1, Test1, TEST1).");
                return;
            }

            // 2. Volume: Integer values only
            bool volumeOk;
            int volumeValue = Volume.toInt(&volumeOk);
            if (!volumeOk) {
                QMessageBox::warning(this, "Volume Input Error", "Volume should contain integer values only.");
                return;
            }

            // 3. Market Capital: Unsigned real (positive float values)
            bool marketCapitalOk;
            double marketCapitalValue = Market_Capital.toDouble(&marketCapitalOk);
            if (!marketCapitalOk || marketCapitalValue <= 0) {
                QMessageBox::warning(this, "Market Capital Input Error", "Market Capital should contain positive float/decimal values only.");
                return;
            }

            // 4. Credit Rating: Signed real (positive or negative float values)
            bool creditRatingOk;
            double creditRatingValue = Credit_Rating.toDouble(&creditRatingOk);
            if (!creditRatingOk) {
                QMessageBox::warning(this, "Credit Rating Input Error", "Credit Rating should contain signed float/decimal values.");
                return;
            }


            // Check if the Product_Name already exists in the QTableWidget
                int rowCount = ui->tableWidget->rowCount();
                for (int i = 0; i < rowCount; ++i)
                {
                    QTableWidgetItem *item = ui->tableWidget->item(i, 0);  // Assuming Product_Name is in the first column
                    if (item && item->text() == Product_Name)
                    {
                        QMessageBox::warning(this, "Product Name Already Exists", "Product Name already exists in the table. Try a different Product Name.");
                        return;
                    }
                }

        // Add a new row to the table
            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);

            // Populate the row with the entered data
            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(Product_Name));
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(Volume));
            ui->tableWidget->setItem(row, 2, new QTableWidgetItem(Market_Capital));
            ui->tableWidget->setItem(row, 3, new QTableWidgetItem(Credit_Rating));

            // Clear the text fields for new entries
            ui->lineEdit_Product_Name->clear();
            ui->lineEdit_Volume->clear();
            ui->lineEdit_Market_Capital->clear();
            ui->lineEdit_Credit_Rating->clear();

            // Show a success message
            QMessageBox::information(this, "Add Product Name Success", "Product Name has been Added to the table successfully.");

}

void MainWindow::on_pushButton_Update_clicked()
{
    // Check if a row is selected
        if (selectedRow == -1) {
            QMessageBox::warning(this, "Update Error", "No product selected for update.");      // Show an error message
            return;
        }

        // Get the updated values from the text boxes
        QString Product_Name = ui->lineEdit_Product_Name->text();
        QString Volume = ui->lineEdit_Volume->text();
        QString Market_Capital = ui->lineEdit_Market_Capital->text();
        QString Credit_Rating = ui->lineEdit_Credit_Rating->text();

        // Check if any fields are empty
        if (Product_Name.isEmpty() || Volume.isEmpty() || Market_Capital.isEmpty() || Credit_Rating.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "All fields must be filled.");  // Show an error message
            return;
        }

        // 1. Validate Product Name: Alphanumeric with at least one letter and one digit
            QRegularExpression productNameRegex("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]+$");
            if (!productNameRegex.match(Product_Name).hasMatch()) {
                QMessageBox::warning(this, "Input Error", "Product Name must contain both letters and numeric characters.");
                return;
            }

            // 2. Validate Volume: Positive Integer values only
            bool volumeIsInt;
            int volumeValue = Volume.toInt(&volumeIsInt);
            if (!volumeIsInt || volumeValue <= 0) {
                QMessageBox::warning(this, "Input Error", "Volume must be a positive integer value.");
                return;
            }

            // 3. Validate Market Capital: Unsigned real (positive float values)
            bool marketCapitalIsFloat;
            double marketCapitalValue = Market_Capital.toDouble(&marketCapitalIsFloat);
            if (!marketCapitalIsFloat || marketCapitalValue <= 0) {
                QMessageBox::warning(this, "Input Error", "Market Capital must be a positive real (float) value.");
                return;
            }

            // 4. Validate Credit Rating: Signed real (positive or negative float values)
            bool creditRatingIsFloat;
            double creditRatingValue = Credit_Rating.toDouble(&creditRatingIsFloat);
            if (!creditRatingIsFloat) {
                QMessageBox::warning(this, "Input Error", "Credit Rating must be a signed real (float) value (positive or negative).");
                return;
            }

        // Update the selected row with the new data in the table widget
        ui->tableWidget->setItem(selectedRow, 0, new QTableWidgetItem(Product_Name));
        ui->tableWidget->setItem(selectedRow, 1, new QTableWidgetItem(Volume));
        ui->tableWidget->setItem(selectedRow, 2, new QTableWidgetItem(Market_Capital));
        ui->tableWidget->setItem(selectedRow, 3, new QTableWidgetItem(Credit_Rating));


        // Clear the text fields after update
        ui->lineEdit_Product_Name->clear();
        ui->lineEdit_Volume->clear();
        ui->lineEdit_Market_Capital->clear();
        ui->lineEdit_Credit_Rating->clear();

        // Reset the selected row variable
        selectedRow = -1;

        // Show a success message
        QMessageBox::information(this, "Update Success", "Product information has been updated successfully.");
}


void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);  // We don't need the column value for this operation

    // Check if the row index is valid
    if (row < 0 || row >= ui->tableWidget->rowCount()) {
        return;
    }

    // Set the selectedRow variable to the row that was double-clicked
        selectedRow = row;

    // Select the entire row in the table widget
        ui->tableWidget->selectRow(row);

    // Load the selected row's data into the line edits
    ui->lineEdit_Product_Name->setText(ui->tableWidget->item(row, 0)->text());
    ui->lineEdit_Volume->setText(ui->tableWidget->item(row, 1)->text());
    ui->lineEdit_Market_Capital->setText(ui->tableWidget->item(row, 2)->text());
    ui->lineEdit_Credit_Rating->setText(ui->tableWidget->item(row, 3)->text());
}


    //This function is for modifying the values and saving in the table widget after clicking the update button
void MainWindow::on_tableWidget_itemSelectionChanged()
{
    // Get the currently selected row
    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
    if (selectedItems.isEmpty())
    {
        selectedRow = -1;
        return;
    }

    selectedRow = selectedItems.first()->row();

    // Load the selected row's data into the text fields
    ui->lineEdit_Product_Name->setText(ui->tableWidget->item(selectedRow, 0)->text());
    ui->lineEdit_Volume->setText(ui->tableWidget->item(selectedRow, 1)->text());
    ui->lineEdit_Market_Capital->setText(ui->tableWidget->item(selectedRow, 2)->text());
    ui->lineEdit_Credit_Rating->setText(ui->tableWidget->item(selectedRow, 3)->text());
}



    // For the function Right Click in table widget (the user right-clicks on a row in your TableWidget,
   //          and you want to provide a menu with options like "Edit", "Delete").
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    // Check if the context menu is triggered within the table widget
    if (ui->tableWidget->underMouse())
    {
        // Show the context menu at the cursor position
        contextMenu->exec(event->globalPos());
    }
}


void MainWindow::on_deleteProductsAction_triggered()
{
    // Get the selected items from the table widget
    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();

    // Check if there are any items selected
        if (selectedItems.isEmpty()) {
            QMessageBox::warning(this, "Delete Error", "No products selected for deletion.");
            qDebug() << "Error: No selection made for deletion.";
            return;
        }

    // Create a set of rows to delete (avoid duplicates)
    QSet<int> rowsToDelete;
    for (QTableWidgetItem *item : selectedItems) {
        rowsToDelete.insert(item->row());
    }

    // Convert the set to a list and sort in reverse order
        QList<int> rows = rowsToDelete.toList();
        std::sort(rows.begin(), rows.end(), std::greater<int>());

        // Delete products from the database and table widget
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM ProductInfo WHERE Product_Name = :Product_Name");

            // Iterate through rows to delete
            for (int row : rows)
            {
                // Get the product name from the first column of the row
                QString productName = ui->tableWidget->item(row, 0)->text();

                // Bind the product name to the delete query
                deleteQuery.bindValue(":Product_Name", productName);

                // Execute the delete query
                if (!deleteQuery.exec())
                {
                    QMessageBox::warning(this, "Delete Error", "Failed to delete product '" + productName + "' from the database: " + deleteQuery.lastError().text());
                    qDebug() << "Failed to delete product from database: " << productName;
                    return;
                }

                qDebug() << "Deleted product from database: " << productName;

                // Remove the row from the table widget
                ui->tableWidget->removeRow(row);
            }

    // Clear the text fields and reset selectedRow variable
    ui->lineEdit_Product_Name->clear();
    ui->lineEdit_Volume->clear();
    ui->lineEdit_Market_Capital->clear();
    ui->lineEdit_Credit_Rating->clear();
    selectedRow = -1;

    // Show a success message
    QMessageBox::information(this, "Delete Product Success", "Product has been Deleted successfully.");
    qDebug() << "Delete operation successful.";
}


void MainWindow::on_pushButton_Save_clicked()
{

        // Fetch all existing product names from the database
        QSqlQuery fetchQuery;
        fetchQuery.prepare("SELECT Product_Name FROM ProductInfo");
        if (!fetchQuery.exec())
        {
            QMessageBox::warning(this, "Fetch Error", "Failed to fetch existing products: " + fetchQuery.lastError().text());
            qDebug() << "Failed to fetch existing products";
            return;
        }



        // Store existing product names from the database in a set for quick lookup
            QSet<QString> existingProductNames;
            while (fetchQuery.next())
            {
                existingProductNames.insert(fetchQuery.value(0).toString());
            }


            // Check for duplicates with in the table widget
                QSet<QString> tableProductNames;
                for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
                {
                    QString Product_Name = ui->tableWidget->item(row, 0)->text();
                    if (tableProductNames.contains(Product_Name))
                    {
                        QMessageBox::warning(this, "Save Error", QString("Duplicate product '%1' found in the table. Please modify or delete duplicates.").arg(Product_Name));
                        qDebug() << "Duplicate product found in table widget";
                        return;
                    }
                    tableProductNames.insert(Product_Name);
                }

                // Check for duplicates between the table widget and the database
                    bool hasDuplicates = false;
                    for (const QString &productName : tableProductNames)
                    {
                        if (existingProductNames.contains(productName))
                        {
                            hasDuplicates = true;
                            break;
                        }
                    }

                    // Ask the user whether to override all existing records if there are duplicates
                    if (hasDuplicates)
                    {
                        QMessageBox::StandardButton reply;
                        reply = QMessageBox::question(this, "Product Exists",
                                                      "Some products already exist in the database. Do you want to override all existing records?",
                                                      QMessageBox::Yes | QMessageBox::No);

                        if (reply == QMessageBox::No)
                        {
                            // If the user chooses "No", exit the function without saving
                            qDebug() << "User chose not to override existing products.";
                            return;
                        }
                    }


    // Prepare SQL query to insert data or updating into the database
        QSqlQuery query;
        // Iterate over the rows of the table widget and insert each product information into the database
        // (OR)   Insert unique products into the database
        for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
        {
            QString Product_Name = ui->tableWidget->item(row, 0)->text();
            QString Volume = ui->tableWidget->item(row, 1)->text();
            QString Market_Capital = ui->tableWidget->item(row, 2)->text();
            QString Credit_Rating = ui->tableWidget->item(row, 3)->text();


     // Check if Product_Name already exists in the database
   if (existingProductNames.contains(Product_Name))
         {
             // If the user confirmed, update all existing records
             query.prepare("UPDATE ProductInfo SET Volume = :Volume, Market_Capital = :Market_Capital, Credit_Rating = :Credit_Rating "
                           "WHERE Product_Name = :Product_Name");
            qDebug() << "Updating existing product: " << Product_Name;
         }
      else
        {
             // If Product_Name does not exist, insert a new record
            query.prepare("INSERT INTO ProductInfo (Product_Name, Volume, Market_Capital, Credit_Rating) "
                          "VALUES (:Product_Name, :Volume, :Market_Capital, :Credit_Rating)");
           qDebug() << "Inserting new product: " << Product_Name;
        }


            // Bind values for both INSERT and UPDATE queries
            query.bindValue(":Product_Name", Product_Name);
            query.bindValue(":Volume", Volume);
            query.bindValue(":Market_Capital", Market_Capital);
            query.bindValue(":Credit_Rating", Credit_Rating);

            // Execute the query and check for errors
            if (!query.exec())
            {
                QMessageBox::warning(this, "Save Error", "Failed to save product information: " + query.lastError().text());
                qDebug() << "Failed to save product information";
                return;
            }
        }




//           // Clear the table after saving
//            ui->tableWidget->setRowCount(0);
//            QMessageBox::information(this, "Save Success", "Product information has been saved successfully.");
//            qDebug() << "Save successful";

}


void MainWindow::on_pushButton_Show_clicked()
{
    // Clear the existing rows in the table widget
        ui->tableWidget->setRowCount(0);

        // Prepare the SQL query to select data from the database
        QSqlQuery query;
        query.prepare("SELECT Product_Name, Volume, Market_Capital, Credit_Rating FROM ProductInfo");

        // Execute the query and check for errors
        if (!query.exec())
        {
            QMessageBox::warning(this, "Database Error", "Failed to fetch data from the database: " + query.lastError().text());
            return;
        }

        // Disable sorting during data population to avoid performance issues
            ui->tableWidget->setSortingEnabled(false);

        // Iterate over the query results and populate the table widget
        int row = 0;
        while (query.next())
        {
            // Insert a new row in the table widget
            ui->tableWidget->insertRow(row);

            // Retrieve the data from the query result
            QString Product_Name = query.value(0).toString();
            QString Volume = query.value(1).toString();
            QString Market_Capital = query.value(2).toString();
            QString Credit_Rating = query.value(3).toString();

            // Populate the row with the retrieved data
            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(Product_Name));
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(Volume));
            ui->tableWidget->setItem(row, 2, new QTableWidgetItem(Market_Capital));
            ui->tableWidget->setItem(row, 3, new QTableWidgetItem(Credit_Rating));

            // Move to the next row
            row++;
        }

        // Enable sorting now that data is populated
            ui->tableWidget->setSortingEnabled(true);

//        ui->tableWidget->sortItems(0, Qt::AscendingOrder); // Sort by the first column (Product_Name) in ascending order


        // If no rows were fetched, show a message
        if (row == 0)
        {
            QMessageBox::information(this, "No Data", "No data found in the database.");
            qDebug() << "No data found in the database.";
        }


        // Clear the data from the database after displaying it in the table widget
            QSqlQuery clearQuery;
            clearQuery.prepare("DELETE FROM ProductInfo");

            // Execute the delete query and check for errors
            if (!clearQuery.exec())
            {
                QMessageBox::warning(this, "Database Error", "Failed to clear data from the database: " + clearQuery.lastError().text());
                qDebug() << "Failed to clear data from the database.";
                return;
            }

            // Notify the user that the data has been displayed and cleared from the database
            QMessageBox::information(this, "Data Cleared", "Data has been displayed and cleared from the database.");
            qDebug() << "Data displayed and cleared from the database.";
}


void MainWindow::on_pushButton_Close_clicked()
{
        // Close the current window
        this->close();
}



// Slot function triggered when the "Connect To Server" button is clicked.
void MainWindow::on_pushButton_Connect_To_Server_clicked()
{

     qDebug() << "Attempting to connect to the server...";
     socket = new QTcpSocket(this);     //creates a new QTcpSocket object and assigns it to the socket variable,
                                        //and sets its parent to this (the current MainWindow).

     // Connect socket signals to corresponding slots.
     connect(socket, SIGNAL(connected()), this, SLOT(connected()));
     connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
     connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
     connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));

     // Connect to the server at the specified IP address and port.
     socket->connectToHost("192.168.1.7", 34567);
}

// Slot function triggered on socket error.
void MainWindow::onError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::ConnectionRefusedError:
            qDebug() << "Connection refused; the server may be down or the port is not open.";
            QMessageBox::information(this, "Connection refused", "The server may be down or the port is not open.");
            break;
        case QAbstractSocket::RemoteHostClosedError:
            qDebug() << "The remote host closed the connection.";
            QMessageBox::information(this, "Remote Host Closed", "The remote host closed the connection.");
            break;
        default:
            qDebug() << "Socket Error:" << socket->errorString();
            QMessageBox::information(this, "Socket Error", "The Socket connection error.");
    }
}

// Slot function triggered when the socket is connected to the server.
void MainWindow::connected()
{
    qDebug() << "Server Connected";
    QMessageBox::information(this, "Server Connected", "Connected to the Server sucessfully.");
}

// Slot function triggered after writing bytes to the socket.
void MainWindow::bytesWritten(qint64 bytes)
{
    qDebug() << "Bytes Written:" << bytes;
}


// Slot function triggered when the "Send Data" button is clicked.
void MainWindow::on_pushButton_Send_Data_clicked()
{
    if (!socket)
    {
        qDebug() << "Socket is not valid";
        QMessageBox::warning(this, "Invalid Socket", "Socket is not valid.");
        return;
    }
    if (socket->state() != QAbstractSocket::ConnectedState)
    {
        qDebug() << "Not connected to the server";
        QMessageBox::information(this, "Server Not Connected", "Not connected to the server.");
        return;
    }

    QString sendData;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        bool isEmptyRow = true;
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col)
        {
            QString cellText = ui->tableWidget->item(row, col)->text();
            if (!cellText.isEmpty())
            {
                sendData.append(cellText);
                sendData.append(",");
                isEmptyRow = false;
            }
        }
        if (!isEmptyRow && row != ui->tableWidget->rowCount() - 1)
        {
            sendData.append("\n");
        }
    }

    qint64 byteWritten = socket->write(sendData.toUtf8());
    if (byteWritten == -1)
    {
        qDebug() << "Error writing to socket:" << socket->errorString();
        QMessageBox::warning(this, "Error", "Error writing to socket.");
    }
}


// Slot function triggered when the socket connection is lost.
void MainWindow::disconnected()
{
    qDebug() << "Server Connection Lost";
    QMessageBox::warning(this, "Connection Lost", "Server Connection Lost.");
}

