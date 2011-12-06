#ifndef CLIENT_H
#define CLIENT_H

#include <QtNetwork/QTcpSocket>
#include <QStringList>

#define CONNECT_TIMEOUT 15*1000 //15s
#define DISCONNECT_TIMEOUT 5*1000 //5s
#define READ_TIMEOUT 15*1000 //15s
#define WRITE_TIMEOUT 15*1000 //15s

class Pop3Client
{
        enum Pop3ConnectionState
                {
                        NotConnected,
                        Authorization,
                        Transaction,
                        Update
                };
        private:

                QTcpSocket m_sock;
                Pop3ConnectionState state;
                bool readOnly;


                QString doCommand(QString command,bool isMultiline);
                bool ReadResponse(bool isMultiline,QString& response);
                bool SendUser(QString& user);
                bool SendPasswd(QString& pwd);


        protected:

        public:
                Pop3Client(bool readOnly = true);
                bool Connect(QString host, unsigned port);
                bool Login(QString user, QString pwd);
                bool LoginWithDigest(QString user, QString digest);
                bool Quit();
                bool GetMailboxStatus(int& nbMessages, int& totalSize);
                bool ResetDeleted();
                bool NoOperation();
                //bool GetUniqueIdList(QVector< QPair<QString,QString> >& uIdList);
                //bool GetUniqueIdList(QString msgId, QPair<QString,QString>& uIdList);
                //bool GetMsgList(QVector< QPair<QString,int> >& uIdList);
                //bool GetMsgList(QString msgId, QPair<QString,int>& uIdList);
                bool Delete(QString msgId);
                //bool GetMessageTop(QString msgId, int nbLines, QString& msgTop);
                //bool GetMessage(QString msgId, QString& msg);
};


#endif // CLIENT_H
