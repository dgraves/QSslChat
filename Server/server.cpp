#include <cassert>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include "server.h"
#include "ui_server.h"

const QString INVALID_FILE_MESSAGE = "Existing and readable key and certificate files must be specified.";

Server::Server(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Server)
{
  ui->setupUi(this);

  // Check for SSL support.  If SSL support is not available, show a
  // message to the user describing what to do to enable SSL support.
  if (QSslSocket::supportsSsl())
  {
    ui->sslFilesGroupBox->setEnabled(true);
    ui->logTextEdit->setText(INVALID_FILE_MESSAGE);
  }
  else
  {
    QString noSslMsg = QString("%1\n%2")
        .arg("*** Your version of Qt does support SSL ***")
        .arg("You must obtain a version of Qt that has SSL"
             "support enabled.  If you believe that your "
             "version of Qt has SSL support enabeld, you may "
             "need to install the OpenSSL run-time libraries.");

    ui->logTextEdit->setText(noSslMsg);
  }

  connect(&server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

Server::~Server()
{
  delete ui;
}

void Server::startStopButtonClicked()
{
  if (server.isListening())
  {
    server.close();
    ui->startStopButton->setText("Start Server");
    ui->logTextEdit->clear();
  }
  else
  {
    int port = ui->portSpinBox->value();
    if (server.listen(QHostAddress::Any, port))
    {
      ui->startStopButton->setText("Stop Server");
      ui->logTextEdit->setText(QString("Server listenting for connections on %1\n").arg(port));
    }
    else
    {
      ui->logTextEdit->setText(QString("Failed to start server: %1\n").arg(server.errorString()));
    }
  }
}

void Server::keyButtonClicked()
{
  QString filename = QFileDialog::getOpenFileName(this, "Select Key File");
  if (!filename.isNull())
  {
    ui->keyLineEdit->setText(filename);
  }
}

void Server::certificateButtonClicked()
{
  QString filename = QFileDialog::getOpenFileName(this, "Select Certificate File");
  if (!filename.isNull())
  {
    ui->certificateLineEdit->setText(filename);
  }
}

void Server::keyFileChanged(const QString &filename)
{
  key = filename;
  checkFileStatus();
}

void Server::certificateFileChanged(const QString &filename)
{
  certificate = filename;
  checkFileStatus();
}

void Server::checkFileStatus()
{
  QFileInfo keyInfo(key);
  QFileInfo certificateInfo(certificate);
  if (keyInfo.exists() && keyInfo.isReadable() &&
      certificateInfo.exists() && certificateInfo.isReadable())
  {
    ui->startStopButton->setEnabled(true);
    ui->logTextEdit->clear();
  }
  else
  {
    ui->startStopButton->setEnabled(false);
    ui->logTextEdit->setText(INVALID_FILE_MESSAGE);
  }
}

// Accept connection from server and initiate the SSL handshake
void Server::acceptConnection()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(server.nextPendingConnection());
  assert(socket);

  // QSslSocket emits the encrypted() signal after the encrypted connection is established
  connect(socket, SIGNAL(encrypted()), this, SLOT(handshakeComplete()));

  // Report any SSL errors that occur
  connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sslErrors(const QList<QSslError> &)));

  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionFailure()));

  socket->setPrivateKey(key);
  socket->setLocalCertificate(certificate);

  socket->setPeerVerifyMode(QSslSocket::VerifyNone);
  socket->startServerEncryption();
}

// Receive notification that the SSL handshake has completed successfully
void Server::handshakeComplete()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  connect(socket, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
  connect(socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));

  ui->logTextEdit->append(QString("[%1] Accepted connection from %2:%3")
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ap"))
                          .arg(socket->peerAddress().toString())
                          .arg(socket->peerPort()));

  sockets.push_back(socket);
}

void Server::sslErrors(const QList<QSslError> &errors)
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  QString errorStrings;
  foreach (QSslError error, errors)
  {
    errorStrings += error.errorString();
    if (error != errors.last())
    {
      errorStrings += ';';
    }
  }

  ui->logTextEdit->append(QString("[%1] %2:%3 reported the following SSL errors: %4")
      .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ap"))
                          .arg(socket->peerAddress().toString())
                          .arg(socket->peerPort())
                          .arg(errorStrings));
}

void Server::receiveMessage()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  if (socket->canReadLine())
  {
    QByteArray message = socket->readLine();
    QString sender = QString("%1:%2")
        .arg(socket->peerAddress().toString())
        .arg(socket->peerPort());

    ui->logTextEdit->append(QString("[%1] %2 sent: %3")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ap"))
        .arg(sender)
        .arg(message.constData()));

    sender += " -> ";
    foreach (QSslSocket *s, sockets)
    {
      s->write(sender.toLocal8Bit().constData());
      s->write(message);
    }
  }
}

void Server::connectionClosed()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  ui->logTextEdit->append(QString("[%1] Connection from %2:%3 closed: %4")
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ap"))
                          .arg(socket->peerAddress().toString())
                          .arg(socket->peerPort())
                          .arg(socket->errorString()));
  sockets.removeOne(socket);
  socket->disconnect();
  socket->deleteLater();
}

void Server::connectionFailure()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  ui->logTextEdit->append(QString("[%1] Connection from %2:%3 failed: %4")
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ap"))
                          .arg(socket->peerAddress().toString())
                          .arg(socket->peerPort())
                          .arg(socket->errorString()));
  sockets.removeOne(socket);
  socket->disconnect();
  socket->deleteLater();
}
