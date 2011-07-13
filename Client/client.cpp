#include <QDateTime>
#include "client.h"
#include "ui_client.h"

Client::Client(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Client)
{
  ui->setupUi(this);

  // Check for SSL support.  If SSL support is not available, show a
  // message to the user describing what to do to enable SSL support.
  if (QSslSocket::supportsSsl())
  {
    ui->connectDisconnectButton->setEnabled(true);
  }
  else
  {
    QString noSslMsg = QString("%1\n%2")
        .arg("*** Your version of Qt does support SSL ***")
        .arg("You must obtain a version of Qt that has SSL"
             "support enabled.  If you believe that your "
             "version of Qt has SSL support enabeld, you may "
             "need to install the OpenSSL run-time libraries.");

    ui->chatDisplayTextEdit->setText(noSslMsg);
  }

  // QSslSocket emits the encrypted() signal after the encrypted connection is established
  connect(&socket, SIGNAL(encrypted()), this, SLOT(connectedToServer()));

  // Report any SSL errors that occur
  connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sslErrors(const QList<QSslError> &)));

  connect(&socket, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
  connect(&socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
  connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
}

Client::~Client()
{
  delete ui;
}

void Client::connectDisconnectButtonPressed()
{
  if (socket.mode() == QAbstractSocket::UnconnectedState)
  {
    // Initiate an SSL connection to the chat server.
    socket.connectToHostEncrypted(ui->hostnameLineEdit->text(), ui->portSpinBox->value());
  }
  else
  {
    socket.close();
  }

  ui->connectDisconnectButton->setEnabled(false);
}

void Client::sendButtonPressed()
{
  QString message = ui->inputLineEdit->text();
  if (!message.isEmpty())
  {
    message += '\n';
    socket.write(message.toLocal8Bit().constData());
  }
}

void Client::connectedToServer()
{
  ui->connectDisconnectButton->setText("Disconnect");
  ui->connectDisconnectButton->setEnabled(true);
  ui->inputLineEdit->setEnabled(true);
  ui->sendButton->setEnabled(true);
  ui->chatDisplayTextEdit->clear();
}

void Client::sslErrors(const QList<QSslError> &errors)
{
  if (errors.first() == QSslError::SelfSignedCertificate)
  {
  }
}

void Client::receiveMessage()
{
  if (socket.canReadLine())
  {
    ui->chatDisplayTextEdit->append(QString("[%1] %2")
                                    .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ap"))
                                    .arg(socket.readLine().constData()));
  }
}

void Client::connectionClosed()
{
  ui->connectDisconnectButton->setText("Connect");
  ui->connectDisconnectButton->setEnabled(true);
  ui->inputLineEdit->setEnabled(false);
  ui->sendButton->setEnabled(false);
}

void Client::socketError()
{
  connectionClosed();
  ui->chatDisplayTextEdit->setText(QString("Socket Error: %1").arg(socket.errorString()));
}
