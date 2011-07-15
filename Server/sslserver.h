#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QSslError>
#include <QTcpServer>

// QTcpServer extension to intercept new socket connection and enable SSL
// To be used by iTCH::Server for accepting encrypted connections
class SslServer : public QTcpServer
{
  Q_OBJECT

public:
  virtual void incomingConnection(int socketDescriptor);
};

#endif // SSLSERVER_H

