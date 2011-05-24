#include "GGClient.h"

#include "constants.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDateTime>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>

#define qDebug() qDebug() << "[GGClient]"
#define qWarning() qWarning() << "[GGClient]"

using namespace KittySDK;

void KittySDK::GGThread::run()
{
  forever {
    mutex.lock();

    if(buffer.size() >= (int)(sizeof(quint32) * 2)) {
      quint32 type, length;

      while(buffer.size() > 0) {
        QDataStream str(buffer);
        str.setByteOrder(QDataStream::LittleEndian);

        str >> type >> length;

        if(buffer.size() < (int)(length + sizeof(quint32) * 2)) {
          break;
        }

        emit packetReceived(type, length, buffer.mid(sizeof(quint32) * 2, length));

        buffer = buffer.mid(length + sizeof(quint32) * 2);
      }
    }

    mutex.unlock();
  }
}

void KittySDK::GGThread::bufferAppend(const QByteArray &buf)
{
  mutex.lock();

  buffer.append(buf);

  mutex.unlock();
}

KittySDK::GGClient::GGClient(QObject *parent): QObject(parent)
{
  m_socket = new QTcpSocket(this);

  connect(m_socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
  connect(m_socket, SIGNAL(connected()), this, SLOT(connected()));
  connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
  connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
  connect(m_socket, SIGNAL(hostFound()), this, SLOT(hostFound()));
  connect(m_socket, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)), this, SLOT(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)));
  connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));

  connect(&m_pingTimer, SIGNAL(timeout()), this, SLOT(sendPingPacket()));

  m_status = KittyGG::Statuses::S_UNAVAILABLE;

  m_initialStatus = KittyGG::Statuses::S_AVAILABLE;
  m_initialDescription = "Kitty is alive!";

  m_pingTimer.setInterval(3 * 60 * 1000);
  m_thread = new GGThread(this);
  connect(m_thread, SIGNAL(packetReceived(quint32,quint32,QByteArray)), this, SLOT(processPacket(quint32,quint32,QByteArray)));
}

KittySDK::GGClient::~GGClient()
{
  delete m_socket;
}

void KittySDK::GGClient::setStatus(const quint32 &status)
{
  if(!isConnected()) {
    m_initialStatus = status;

    connectToHost();
  } else {
    m_status = status;
    sendChangeStatusPacket();
  }
}

void KittySDK::GGClient::setDescription(const QString &description)
{
  m_description = description;

  if(isConnected()) {
    sendChangeStatusPacket();
  }
}

void KittySDK::GGClient::setAccount(const quint32 &uin, const QString &passsword)
{
  setUin(uin);
  setPassword(passsword);
}

bool KittySDK::GGClient::isConnected()
{
  return (m_socket->state() == QAbstractSocket::ConnectedState);
}

void KittySDK::GGClient::addContact(const quint32 &uin)
{
  if(!m_roster.contains(uin)) {
    m_roster.append(uin);
  }

  QByteArray data;
  data.append((char*)&uin, 4);
  data.append(0x03);

  sendPacket(KittyGG::Packets::P_NOTIFY_ADD, data, data.size());

}

void KittySDK::GGClient::removeContact(const quint32 &uin)
{
  m_roster.removeAll(uin);
}

void KittySDK::GGClient::connectToHost(const QString &host, const int &port)
{
  m_host = host;
  m_port = port;
  m_status = m_initialStatus;
  m_description = m_initialDescription;

  m_socket->connectToHost(host, port);
}

void KittySDK::GGClient::sendMessage(const quint32 &recipient, const QString &text)
{
  qDebug() << "sending message";
  QByteArray data;
  quint32 tmp32;

  // recipient
  tmp32 = recipient;
  data.append((char*)&tmp32, sizeof(tmp32));

  // seq
  tmp32 = QDateTime::currentDateTime().toTime_t();
  data.append((char*)&tmp32, sizeof(tmp32));

  // msgclass
  tmp32 = 0x0008;
  data.append((char*)&tmp32, sizeof(tmp32));

  // offset_plain
  tmp32 = sizeof(quint32) * 5 + text.size();
  data.append((char*)&tmp32, sizeof(tmp32));

  // offset_attributes
  tmp32 = sizeof(quint32) * 5 + (text.size() * 2);
  data.append((char*)&tmp32, sizeof(tmp32));

  data.append(text.toAscii().data(), text.length());
  data.append(text.toLocal8Bit().data(), text.length());
  data.append((char)0x02);
  data.append((char)0x06);
  data.append((char)0x00);
  data.append((char)0x00);
  data.append((char)0x00);
  data.append((char)0x08);
  data.append((char)0x00);
  data.append((char)0x00);
  data.append((char)0x00);


  sendPacket(KittyGG::Packets::P_MSG_SEND, data, data.size());
}

void KittySDK::GGClient::readSocket()
{
  m_thread->bufferAppend(m_socket->readAll());
}

void KittySDK::GGClient::connected()
{
  qDebug() << "Socket::Connected";
  m_thread->start();
}

void KittySDK::GGClient::disconnected()
{
  qDebug() << "Socket::Disconnected";

  emit statusChanged(uin(), m_status, m_description);

  m_pingTimer.stop();
}

void KittySDK::GGClient::error(QAbstractSocket::SocketError socketError)
{
  qDebug() << "Socket::error(" << socketError << ")";
}

void KittySDK::GGClient::hostFound()
{
  qDebug() << "Socket::hostNotFound";
  qDebug() << m_socket->errorString();
}

void KittySDK::GGClient::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
  qDebug() << "Socket::proxyAuthReq";
}

void KittySDK::GGClient::stateChanged(QAbstractSocket::SocketState socketState)
{
  qDebug() << "Socket::stateChanged(" << socketState << ")";
}

void KittySDK::GGClient::processPacket(const quint32 &type, const quint32 &length, QByteArray packet)
{
  QDataStream str(packet);
  str.setByteOrder(QDataStream::LittleEndian);

  qDebug() << "New packet (" << type << ", " << length << ")";

  switch(type) {
    case KittyGG::Packets::P_WELCOME:
    {
      qDebug() << "It's P_WELCOME";

      quint32 seed;
      str >> seed;

      sendLoginPacket(seed);
    }
    break;

    case KittyGG::Packets::P_LOGIN_OK:
    {
      qDebug() << "It's P_LOGIN_OK";

      quint32 unknown;
      str >> unknown;

      m_pingTimer.start();

      sendRosterPacket();
    }
    break;

    case KittyGG::Packets::P_LOGIN_FAILED:
    {
      qDebug() << "It's P_LOGIN_FAILED";

      quint32 unknown;
      str >> unknown;

      m_socket->disconnectFromHost();
    }
    break;

    case KittyGG::Packets::P_NOTIFY_REPLY:
    case KittyGG::Packets::P_STATUS:
    {
      qDebug() << "It's P_STATUS";

      int left = length;

      while(left > 0) {
        quint32 uin;
        str >> uin;
        left -= sizeof(uin);

        quint32 status;
        str >> status;
        left -= sizeof(status);

        quint32 features;
        str >> features;
        left -= sizeof(features);

        quint32 remote_ip;
        str >> remote_ip;
        left -= sizeof(remote_ip);

        quint16 remote_port;
        str >>remote_port;
        left -= sizeof(remote_port);

        quint8 image_size;
        str >> image_size;
        left -= sizeof(image_size);

        quint8 unknown;
        str >> unknown;
        left -= sizeof(unknown);

        quint32 flags;
        str >> flags;
        left -= sizeof(flags);

        quint32 description_size;
        str >> description_size;
        left -= sizeof(description_size);

        char *description = new char[description_size];
        if(description_size > 0) {
          str.readRawData(description, description_size);
          left -= description_size;
        }

        if(uin == m_uin) {
          m_status = status;
          m_description = description;
        }

        //qDebug() << uin << status << QString::fromAscii(description, description_size);

        emit statusChanged(uin, status, QString::fromAscii(description, description_size));
      }

      if(left > 0) {
        qDebug() << "left" << left;
      }
    }
    break;

    case KittyGG::Packets::P_XML_ACTION:
    {
      qDebug() << "It's P_XML_ACTION";

      char *data = new char[length];
      str.readRawData(data, length);

      emit xmlActionReceived(QString::fromAscii(data, length));
    }
    break;

    case KittyGG::Packets::P_MSG_RECV:
    {
      qDebug() << "It's P_MSG_RECV";

      int left = length;
      while(left > 0) {
        int read = 0;

        quint32 sender;
        str >> sender;
        read += sizeof(sender);

        quint32 seq;
        str >> seq;
        read += sizeof(seq);

        quint32 time;
        str >> time;
        read += sizeof(time);

        quint32 msgclass;
        str >> msgclass;
        read += sizeof(msgclass);

        quint32 offset_plain;
        str >> offset_plain;
        read += sizeof(offset_plain);

        quint32 offset_attributes;
        str >> offset_attributes;
        read += sizeof(offset_attributes);

        quint32 html_length = offset_plain - read;
        char *html = 0;
        if(html_length > 0) {
          html = new char[html_length];

          str.readRawData(html, html_length);
          read += html_length;
        }

        quint32 plain_length = offset_attributes - offset_plain;
        char *plain = 0;
        if(plain_length > 0) {
          plain = new char[plain_length];

          str.readRawData(plain, plain_length);
          read += plain_length;
        }

        quint8 flag;
        str >> flag;
        read += sizeof(flag);

        quint16 attr_length;
        str >> attr_length;
        read += sizeof(attr_length);

        char *attr = 0;
        if(attr_length > 0) {
          attr = new char[attr_length];
          str.readRawData(attr, attr_length);
          read += attr_length;
        }

        QString text;
        if(html_length > 0) {
          text = QString::fromAscii(html);
          text.replace(QRegExp("\\s{0,}font-family:'[^']*';\\s{0,}", Qt::CaseInsensitive), "");
          text.replace(QRegExp("\\s{0,}font-size:[^pt]*pt;\\s{0,}", Qt::CaseInsensitive), "");
        } else {
          text = plainToHtml(QString::fromLocal8Bit(plain), QByteArray(attr, attr_length));
        }

        QDateTime qtime = QDateTime::currentDateTime();
        if(qtime.toTime_t() > time) {
          qtime.setTime_t(time);
        }

        emit messageReceived(sender, qtime, text);

        left -= read;
        if(left > 0) {
          qDebug() << "left" << left;
          left = 0;
        }
      }
    }
    break;

    case KittyGG::Packets::P_MSG_SEND_ACK:
    {
      qDebug() << "It's P_MSG_SEND_ACK";

      quint32 status;
      str >> status;

      quint32 recipient;
      str >> recipient;

      quint32 seq;
      str >> seq;

      if(status != KittyGG::MessageAck::ACK_DELIVERED) {
        qDebug() << "Message sent to" << recipient << "with seq" << seq << "has status" << status;
      }
    }
    break;

    case KittyGG::Packets::P_TYPING_NOTIFY:
    {
      qDebug() << "It's P_TYPING_NOTIFY";

      quint16 type;
      str >> type;

      quint32 uin;
      str >> uin;

      if(type > 0) {
        qDebug() << uin << "is typing:" << type;
      } else {
        qDebug() << uin << "stopped typing";
      }
    }
    break;

    case KittyGG::Packets::P_USER_DATA:
    {
      quint32 type;
      str >> type;

      int num;
      str >> num;

      while(num > 0) {
        quint32 uin;
        str >> uin;

        int num2;
        str >> num2;

        while(num2 > 0) {
          quint32 name_size;
          str >> name_size;

          char *name = new char[name_size];
          if(name_size > 0) {
            str.readRawData(name, name_size);
          }

          quint32 type;
          str >> type;

          quint32 value_size;
          str >> value_size;

          char *value = new char[value_size];
          if(value_size > 0) {
            str.readRawData(value, value_size);
          }

          emit userDataReceived(uin, QString::fromAscii(name, name_size), QString::fromAscii(value, value_size));

          num2--;
        }

        num--;
      }
    }
    break;

    case KittyGG::Packets::P_DISCONNECTING:
    {
      qDebug() << "It's P_DISCONNECTING";
    }
    break;

    case KittyGG::Packets::P_DISCONNECT_ACK:
    {
      qDebug() << "It's P_DISCONNECT_ACK";

      m_socket->disconnectFromHost();
    }
    break;

    default:
      qDebug() << "Unknown type" << type;
    break;
  }
}

void KittySDK::GGClient::sendLoginPacket(const quint32 &seed)
{
  QByteArray data;
  quint8 tmp8;
  quint32 tmp32;

  QCryptographicHash hash(QCryptographicHash::Sha1);
  hash.addData(password().toLatin1(), password().length());
  hash.addData((char*)&seed, sizeof(seed));

  // uin
  tmp32 = uin();
  data.append((char*)&tmp32, sizeof(tmp32));

  // language
  data.append("pl", strlen("pl"));

  // hash_type
  tmp8 = KittyGG::HashMethods::H_SHA1;
  data.append((char*)&tmp8, sizeof(tmp8));

  // hash
  data.append(hash.result().data(), 64);

  // status
  tmp32 = status() | 0x4000;
  data.append((char*)&tmp32, sizeof(tmp32));

  // flags
  tmp32 = KittyGG::Flags::F_UNKNOWN | KittyGG::Flags::F_SPAM;
  data.append((char*)&tmp32, sizeof(tmp32));

  // features
  tmp32 = KittyGG::Features::F_STATUS80 | KittyGG::Features::F_MSG80 | KittyGG::Features::F_NEW_LOGIN | KittyGG::Features::F_DND_FFC | KittyGG::Features::F_IMAGE_DESCR |  KittyGG::Features::F_TYPING_NOTIFICATION | KittyGG::Features::F_USER_DATA;
  data.append((char*)&tmp32, sizeof(tmp32));

  // deprecated (local_ip, local_port, external_ip, external_port)
  data.append(QByteArray(12, 0x00));

  // image_size
  tmp8 = 255;
  data.append((char*)&tmp8, sizeof(tmp8));

  // unknown
  tmp8 = 0x64;
  data.append((char*)&tmp8, sizeof(tmp8));

  // version_size & version
  tmp32 = strlen("KittyIM");
  data.append((char*)&tmp32, sizeof(tmp32));
  data.append("KittyIM", strlen("KittyIM"));

  // description_size & description
  tmp32 = m_description.size();
  data.append((char*)&tmp32, sizeof(tmp32));
  data.append(m_description.toLocal8Bit().data(), m_description.length());

  sendPacket(KittyGG::Packets::P_LOGIN, data, data.size());
}

void KittySDK::GGClient::sendRosterPacket()
{
  if(m_roster.empty()) {
    sendPacket(KittyGG::Packets::P_LIST_EMPTY);
    return;
  }

  int count = m_roster.size();

  while(count > 0) {
    QByteArray data;
    int part_count, packet_type;

    if(count > 400) {
      part_count = 400;
      packet_type = KittyGG::Packets::P_NOTIFY_FIRST;
    } else {
      part_count = count;
      packet_type = KittyGG::Packets::P_NOTIFY_LAST;
    }

    data.resize((sizeof(quint32) + sizeof(quint8)) * part_count);

    for(int i = 0; i < part_count; i++) {
      data.append((char*)&m_roster.at(i), 4);
      data.append(0x03);
    }

    sendPacket(packet_type, data, data.size());

    count -= part_count;
  }
}

void KittySDK::GGClient::sendChangeStatusPacket()
{
  QByteArray data;
  quint32 tmp32;

  // status
  tmp32 = status() | 0x4000;
  data.append((char*)&tmp32, sizeof(tmp32));

  // flags
  tmp32 = 0;
  data.append((char*)&tmp32, sizeof(tmp32));

  // description_size & description
  tmp32 = m_description.size();
  data.append((char*)&tmp32, sizeof(tmp32));
  data.append(m_description.toLocal8Bit().data(), m_description.length());

  sendPacket(KittyGG::Packets::P_NEW_STATUS, data, data.size());
}

void KittySDK::GGClient::sendPingPacket()
{
  if(isConnected()) {
    qDebug() << "Sending P_PING";
    sendPacket(KittyGG::Packets::P_PING);
  }
}

void KittySDK::GGClient::sendPacket(const int &type, const QByteArray &data, const quint32 &size)
{
  QByteArray buffer;

  buffer.append((char*)&type, 4);
  buffer.append((char*)&size, 4);
  buffer.append(data);

  qint64 res = m_socket->write(buffer);
  if(res != buffer.size()) {
    qDebug() << "Didn't send whole packet!";
  }
}

QString KittySDK::GGClient::plainToHtml(const QString &plain, const QByteArray &attr)
{
  QDataStream str(attr);
  str.setByteOrder(QDataStream::LittleEndian);

  QString text, fragment;

  bool opened = false;
  quint16 last_pos = 0;
  quint8 red = 0;
  quint8 green = 0;
  quint8 blue = 0;

  int attr_length = attr.size();
  while(attr_length > 0) {
    quint16 pos;
    str >> pos;
    attr_length -= sizeof(pos);

    quint8 font;
    str >> font;
    attr_length -= sizeof(font);

    pos++;
    fragment = plain.mid(last_pos, pos - last_pos);
    last_pos = pos;

    if(opened) {
      text.append("</span>");
      opened = false;
    }

    if(font & KittyGG::Fonts::F_IMAGE) {
      qDebug() << "Image!";
    } else {
      QString style;

      if(font & KittyGG::Fonts::F_COLOR) {
        str >> red;
        attr_length -= sizeof(red);

        str >> green;
        attr_length -= sizeof(green);

        str >> blue;
        attr_length -= sizeof(blue);

        //qDebug() << "at pos" << pos << "there's color " << red << green << blue;
      }

      style.append(QString("color: #%1%2%3;").arg(QString::number(red, 16)).arg(QString::number(green, 16)).arg(QString::number(blue, 16)));

      if(font & KittyGG::Fonts::F_BOLD) {
        style.append("font-weight: bold;");

        //qDebug() << "at pos" << pos << "there's bold";
      }

      if(font & KittyGG::Fonts::F_ITALIC) {
        style.append("font-style: italic;");

        //qDebug() << "at pos" << pos << "there's italic";
      }

      if(font & KittyGG::Fonts::F_UNDERLINE) {
        style.append("text-decoration: underline;");

        //qDebug() << "at pos" << pos << "there's underline";
      }

      QString code("<span");
      if(!style.isEmpty()) {
        code.append(QString(" style=\"%1\"").arg(style));
      }

      code.append(">");

      fragment.replace("&", "&amp;");
      fragment.replace("<", "&lt;");;
      fragment.replace(">", "&gt;");
      fragment.replace("\"", "&quot;");

      text.append(code);
      text.append(fragment);
      opened = true;
    }
  }

  fragment = plain.mid(last_pos);
  fragment.replace("&", "&amp;");
  fragment.replace("<", "&lt;");;
  fragment.replace(">", "&gt;");
  fragment.replace("\"", "&quot;");

  text.append(fragment);

  if(opened) {
    text.append("</span>");
  }

  text.replace("\n", "<br>");
  text.replace("\r", "");
  text.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");
  text.replace("  ", " &nbsp;");

  return text;
}
