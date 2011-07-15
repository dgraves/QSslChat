#include <QSslSocket>
#include "sslserver.h"

// Intercept new socket connection and enable SSL
void SslServer::incomingConnection(int socketDescriptor)
{
  QSslSocket *serverSocket = new QSslSocket();
  if (serverSocket->setSocketDescriptor(socketDescriptor))
  {
    addPendingConnection (serverSocket);
  }
  else
  {
    delete serverSocket;
  }
}

