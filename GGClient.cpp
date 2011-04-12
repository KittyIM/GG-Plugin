#include "GGClient.h"

#include "constants.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>

using namespace KittySDK;

GGClient::GGClient(QObject *parent): QObject(parent)
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
}

GGClient::~GGClient()
{
  delete m_socket;
}

void GGClient::setStatus(const quint32 &status)
{
  m_status = status;

  if(isConnected()) {
    sendChangeStatusPacket();
  }
}

void GGClient::setDescription(const QString &description)
{
  m_description = description;

  if(isConnected()) {
    sendChangeStatusPacket();
  }
}

void GGClient::setAccount(const quint32 &uin, const QString &passsword)
{
  setUin(uin);
  setPassword(passsword);
}

bool GGClient::isConnected()
{
  return (m_socket->state() == QAbstractSocket::ConnectedState);
}

void GGClient::addContact(const quint32 &uin)
{
  if(!m_roster.contains(uin)) {
    m_roster.append(uin);
  }
}

void GGClient::removeContact(const quint32 &uin)
{
  m_roster.removeAll(uin);
}

void GGClient::connectToHost(const QString &host, const int &port)
{
  m_host = host;
  m_port = port;
  m_status = m_initialStatus;
  m_description = m_initialDescription;

  m_socket->connectToHost(host, port);
}

void GGClient::sendMessage(const quint32 &recipient, const QString &text)
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

void GGClient::readSocket()
{
  m_buffer.append(m_socket->readAll());

  quint32 type, length;

  while(m_buffer.size() > 0) {
    QDataStream str(m_buffer);
    str.setByteOrder(QDataStream::LittleEndian);

    str >> type >> length;

    qDebug() << "New packet (" << type << ", " << length << ")";

    if(m_buffer.size() < (int)(length + sizeof(quint32) * 2)) {
      qDebug() << "But it's not complete";
      return;
    }

    m_buffer = m_buffer.mid(sizeof(quint32) * 2);

    processPacket(type, length);
  }
}

void GGClient::connected()
{
  qDebug() << "Socket::Connected";
  //m_thread->start();
}

void GGClient::disconnected()
{
  qDebug() << "Socket::Disconnected";
  m_pingTimer.stop();
}

void GGClient::error(QAbstractSocket::SocketError socketError)
{
  qDebug() << "Socket::error(" << socketError << ")";
}

void GGClient::hostFound()
{
  qDebug() << "Socket::hostNotFound";
}

void GGClient::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
  qDebug() << "Socket::proxyAuthReq";
}

void GGClient::stateChanged(QAbstractSocket::SocketState socketState)
{
  qDebug() << "Socket::stateChanged(" << socketState << ")";
}

void GGClient::processPacket(const quint32 &type, const quint32 &length)
{
  QDataStream str(m_buffer);
  str.setByteOrder(QDataStream::LittleEndian);

  switch(type) {
    case KittyGG::Packets::P_WELCOME:
    {
      qDebug() << "It's P_WELCOME";

      quint32 seed;
      str >> seed;
      m_buffer = m_buffer.mid(4);

      sendLoginPacket(seed);
    }
    break;

    case KittyGG::Packets::P_LOGIN_OK:
    {
      qDebug() << "It's P_LOGIN_OK";

      quint32 unknown;
      str >> unknown;
      m_buffer = m_buffer.mid(sizeof(unknown));

      m_pingTimer.start();

      sendRosterPacket();
    }
    break;

    case KittyGG::Packets::P_LOGIN_FAILED:
    {
      qDebug() << "It's P_LOGIN_FAILED";

      quint32 unknown;
      str >> unknown;
      m_buffer = m_buffer.mid(sizeof(unknown));

      m_socket->disconnectFromHost();
    }
    break;

    case KittyGG::Packets::P_NOTIFY_REPLY:
    case KittyGG::Packets::P_STATUS:
    {
      qDebug() << "It's P_STATUS";

      int left = length;

      while(left > 0) {
        int read = 0;

        quint32 uin;
        str >> uin;
        read += sizeof(uin);

        quint32 status;
        str >> status;
        read += sizeof(status);

        quint32 features;
        str >> features;
        read += sizeof(features);

        quint32 remote_ip;
        str >> remote_ip;
        read += sizeof(remote_ip);

        quint16 remote_port;
        str >>remote_port;
        read += sizeof(remote_port);

        quint8 image_size;
        str >> image_size;
        read += sizeof(image_size);

        quint8 unknown;
        str >> unknown;
        read += sizeof(unknown);

        quint32 flags;
        str >> flags;
        read += sizeof(flags);

        quint32 description_size;
        str >> description_size;
        read += sizeof(description_size);

        char *description = new char[description_size];
        if(description_size > 0) {
          str.readRawData(description, description_size);
          read += description_size;
        }

        m_buffer = m_buffer.mid(read);

        if(uin == m_uin) {
          m_status = status;
          m_description = description;
        }

        qDebug() << uin << status << QString::fromAscii(description, description_size);

        emit statusChanged(uin, status, QString::fromAscii(description, description_size));

        left -= read;
      }

      if(left > 0) {
        qDebug() << "left" << left;
      }
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
        char *html = new char[html_length];
        if(html_length > 0) {
          str.readRawData(html, html_length);
          read += html_length;
        }

        quint32 plain_length = offset_attributes - offset_plain;
        char *plain = new char[plain_length];
        if(plain_length > 0) {
          str.readRawData(plain, plain_length);
          read += plain_length;
        }

        quint8 flag;
        str >> flag;
        read += sizeof(flag);

        quint16 attr_length;
        str >> attr_length;
        read += sizeof(attr_length);

        while(attr_length > 0) {
          quint16 pos;
          str >> pos;
          attr_length -= sizeof(pos);
          read += sizeof(pos);

          quint8 font;
          str >> font;
          attr_length -= sizeof(font);
          read += sizeof(font);

          if(font & KittyGG::Fonts::F_COLOR) {
            quint8 red;
            str >> red;
            attr_length -= sizeof(red);
            read += sizeof(red);

            quint8 green;
            str >> green;
            attr_length -= sizeof(green);
            read += sizeof(green);

            quint8 blue;
            str >> blue;
            attr_length -= sizeof(blue);
            read += sizeof(blue);

            qDebug() << "at pos" << pos << "there's color " << red << green << blue;
          }

          if(font & KittyGG::Fonts::F_BOLD) {
            qDebug() << "at pos" << pos << "there's bold";
          }

          if(font & KittyGG::Fonts::F_ITALIC) {
            qDebug() << "at pos" << pos << "there's italic";
          }

          if(font & KittyGG::Fonts::F_UNDERLINE) {
            qDebug() << "at pos" << pos << "there's underline";
          }
        }

        m_buffer = m_buffer.mid(read);

        QDateTime qtime;
        qtime.setTime_t(time);
        emit messageReceived(sender, qtime, QString::fromLocal8Bit(plain));

        left -= read;
        if(left > 0) {
          qDebug() << "left" << left;
          m_buffer = m_buffer.mid(left);
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

      m_buffer = m_buffer.mid(sizeof(status) + sizeof(recipient) + sizeof(seq));
      qDebug() << "Message sent to" << recipient << "with seq" << seq << "has status" << status;
    }
    break;

    case KittyGG::Packets::P_TYPING_NOTIFY:
    {
      qDebug() << "It's P_TYPING_NOTIFY";

      quint16 type;
      str >> type;

      quint32 uin;
      str >> uin;

      m_buffer = m_buffer.mid(sizeof(type) + sizeof(uin));

      if(type > 0) {
        qDebug() << uin << "is typing:" << type;
      } else {
        qDebug() << uin << "stopped typing";
      }
    }
    break;

    case KittyGG::Packets::P_USER_DATA:
    {
      int read = 0;

      quint32 type;
      str >> type;
      read += sizeof(type);

      int num;
      str >> num;
      read += sizeof(num);

      while(num > 0) {
        quint32 uin;
        str >> uin;
        read += sizeof(uin);

        int num2;
        str >> num2;
        read += sizeof(num2);

        while(num2 > 0) {
          quint32 name_size;
          str >> name_size;
          read += sizeof(name_size);

          char *name = new char[name_size];
          if(name_size > 0) {
            str.readRawData(name, name_size);
            read += name_size;
          }

          quint32 type;
          str >> type;
          read += sizeof(type);

          quint32 value_size;
          str >> value_size;
          read += sizeof(value_size);

          char *value = new char[value_size];
          if(value_size > 0) {
            str.readRawData(value, value_size);
            read += value_size;
          }

          qDebug() << uin << "attr[" << QString::fromAscii(name, name_size) << "] = \"" << QString::fromAscii(value, value_size) << "\"";

          num2--;
        }

        num--;
      }

      m_buffer = m_buffer.mid(read);
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

void GGClient::sendLoginPacket(const quint32 &seed)
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

void GGClient::sendRosterPacket()
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

void GGClient::sendChangeStatusPacket()
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

void GGClient::sendPingPacket()
{
  if(isConnected()) {
    qDebug() << "Sending P_PING";
    sendPacket(KittyGG::Packets::P_PING);
  }
}

void GGClient::sendPacket(const int &type, const QByteArray &data, const quint32 &size)
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

