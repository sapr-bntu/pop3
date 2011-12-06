#include "pop3.h"
#include "ui_pop3.h"
#include "Client.h"

bool Pop3Client::Connect(QString host, unsigned port)
{
        m_sock.connectToHost(host,port);
        if (!m_sock.waitForConnected(CONNECT_TIMEOUT))
        {
                return false;
        }
        QString response;
        ReadResponse(false,response);
        if (response.startsWith("+OK"))
                state = Authorization;
        else
                return false;
        return true;
}

bool Pop3Client::ReadResponse(bool isMultiline,QString& response)
{
        char buff[1512+1]; // according to RFC1939 the response can only be 512 chars
        bool couldRead = m_sock.waitForReadyRead( READ_TIMEOUT ) ;
        //if (!couldRead) _ERROR("could not receive data: ");
        bool complete=false;
        bool completeLine=false;
        unsigned int offset=0;
        while (couldRead && !complete)
        {
//		qDebug() << "reading front\n";
                if (offset >= sizeof(buff))
                {
                        qDebug() << "avoiding buffer overflow, server is not RFC1939 compliant\n";
                        return false;
                }
                qint64 bytesRead = m_sock.readLine(buff + offset,sizeof(buff)-offset);
                if (bytesRead == -1)
                        return false;
                couldRead = bytesRead > 0;
                completeLine = buff[offset+bytesRead-1]=='\n';
                if (couldRead)
                {
                        if (completeLine)
                        {
//				qDebug() << "Line Complete\n";
                                offset = 0;
                                if (response.size() == 0)
                                {//first line, check for error
                                        response.append(buff);
                                        if (!response.startsWith("+OK"))
                                                complete = true;
                                        else
                                                complete = !isMultiline;
                                }
                                else
                                {//NOT first line, check for byte-stuffing
                                        //check if the response was complete
                                        complete = ( strcmp(buff,".\r\n") == 0 );
                                        if (!complete)
                                        {
                                                if (buff[0] == '.' && buff[1]== '.')
                                                        response.append(buff+1);	//remove the stuffed byte and add to the response
                                                else
                                                        response.append(buff);	//add to the response
                                        }
                                }
                        }
                        else
                        {
//				qDebug() << "Line INComplete: " << buff <<"\n";
                                offset += bytesRead;
                        }
                }
                if (couldRead && !complete)
                {
                        if (m_sock.bytesAvailable() <= 0)
                        {
//		qDebug() << "waiting for data\n";
                                couldRead = m_sock.waitForReadyRead( READ_TIMEOUT ) ;
//		qDebug() << "waiting for data finished, couldread: " << couldRead << "\n";
                        }
                }
        }
        return couldRead && complete;
}
Pop3Client::Pop3Client(bool readOnly)
{
        this->readOnly = readOnly;
        state = NotConnected;
        m_sock.blockSignals(true);
}
bool Pop3Client::Login(QString user, QString pwd)
{

        if (!SendUser(user))
                return false;
        if (!SendPasswd(pwd))
                return false;
        state = Transaction;
        return true;
}
bool Pop3Client::SendUser(QString& user)
{
        QString res = doCommand("USER "+user+"\r\n",false);
        if (res.startsWith("+OK"))
                return true;
        else
                return false;
}
bool Pop3Client::SendPasswd(QString& pwd)
{
        QString res = doCommand("PASS "+pwd+"\r\n",false);
        if (res.startsWith("+OK"))
                return true;
        else
                return false;
}
bool Pop3Client::Quit()
{
        if (readOnly)
                if (!ResetDeleted())
                        return false;
        QString res = doCommand("QUIT\r\n",false);
        if (res.startsWith("+OK"))
        {
                if (!m_sock.waitForDisconnected(DISCONNECT_TIMEOUT))
                {
                        //_ERROR("Connection was not closed by server: ");
                        return false;
                }
                else
                        state = NotConnected;
                return true;
        }
        else
        {
                return false;
        }
}
QString Pop3Client::doCommand(QString command,bool isMultiline)
{
//	qDebug() << "sending command: " << command << "\n";
        QString response;
        qint64 writeResult = m_sock.write(command.toAscii());
//        if (writeResult != command.size()) qDebug()<<"Could not write all bytes: ";
        if (writeResult > 0 && !m_sock.waitForBytesWritten(WRITE_TIMEOUT))  qDebug()<<"Could not write bytes from buffer: ";
        if (!ReadResponse(isMultiline,response))
                return "";
        return response;
}
bool Pop3Client::GetMailboxStatus(int& nbMessages, int& totalSize)
{
        QString res = doCommand("STAT\r\n",false);
        if (res.startsWith("+OK"))
        {
                QStringList sl = res.split(' ',QString::SkipEmptyParts);
                if (sl.count() < 3)
                        return false;
                else
                {
                        nbMessages = sl[1].toInt();
                        totalSize = sl[2].toInt();
                }
                return true;
        }
        else
                return false;
}
bool Pop3Client::ResetDeleted()
{
        QString res = doCommand("RSET\r\n",false);
        if (res.startsWith("+OK"))
                return true;
        else
                return false;
}
bool Pop3Client::NoOperation()
{
        QString res = doCommand("NOOP\r\n",false);
        if (res.startsWith("+OK"))
                return true;
        else
                return false;
}
Pop3::Pop3(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Pop3)
{
    ui->setupUi(this);
    QObject::connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(MyEventHandler()));
    QObject::connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(Authorize()));
    client = new Pop3Client(true);
    //QObject::connect(ui->pushButton_2, SIGNAL(clicked()), ui->textEdit_2, SLOT(setText(tr("Пробуем авторизоваться..."))));
}
bool Pop3Client::Delete(QString msgId)
{
        if (readOnly)
                return false;
        QString res = doCommand("DELE "+msgId+"\r\n",false);
        if (res.startsWith("+OK"))
                return true;
        else
                return false;
}
bool Pop3Client::LoginWithDigest(QString user, QString digest)
{
        QString res = doCommand("APOP "+user+" "+digest+"\r\n",false);
        if (res.startsWith("+OK"))
                return true;
        else
                return false;
}
void Pop3::MyEventHandler()
{


//model = new QString(this);

//QObject::connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(MyEventHandler()));
if (!client->Connect(ui->lineEdit->text(), 110))
{
    ui->textEdit->setText(tr("Не удаётся подключиться к серверу"));
    //ui->textEdit->setStyle(QStyle "color:red;");
}
else
{
    ui->textEdit->setText(tr("Подключение к серверу успешно выполнено!"));
    //ui->textEdit->setStyle("color:green;");
}

}
void Pop3::Authorize()
{

    ui->textEdit_2->setText(tr("Пробуем авторизоваться..."));
    if (!client->Login("ya.test.pop", "monstermail")) {
        ui->textEdit_2->setText(tr("Неверный логин или пароль!"));
    } else {
        ui->textEdit_2->setText(tr("Вы успешно авторизованы!"));
    }
}

Pop3::~Pop3()
{
    delete ui;
}
