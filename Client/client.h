#ifndef CLIENT_H
#define CLIENT_H

#include <QSslSocket>
#include <QMainWindow>

namespace Ui {
  class Client;
}

class Client : public QMainWindow
{
  Q_OBJECT

public:
  explicit Client(QWidget *parent = 0);
  ~Client();

protected slots:
  // Slots to receive signals from UI
  void connectDisconnectButtonPressed();
  void sendButtonPressed();

  // Slots to receive signals from QSslSocket
  void connectedToServer();
  void sslErrors(const QList<QSslError> &errors);
  void receiveMessage();
  void connectionClosed();
  void socketError();

private:
  QSslSocket socket;
  Ui::Client *ui;
};

#endif // CLIENT_H
