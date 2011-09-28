#include "GGClient.h"

#include "constants.h"
#include "zlib/zlib.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDateTime>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtNetwork/QNetworkProxy>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

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
  m_socket->setProxy(QNetworkProxy::applicationProxy());

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
  if(!isConnected()) {
    m_initialDescription = description;
  } else {
    m_description = description;
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

  if(isConnected()) {
    QByteArray data;
    data.append((char*)&uin, 4);
    data.append(0x03);

    sendPacket(KittyGG::Packets::P_NOTIFY_ADD, data, data.size());
  }
}

void KittySDK::GGClient::removeContact(const quint32 &uin)
{
  m_roster.removeAll(uin);
}

void KittySDK::GGClient::connectToHost(const QString &host, const int &port)
{
  m_host = host;
  m_port = port;

  m_socket->connectToHost(host, port);
}

void KittySDK::GGClient::sendMessage(const quint32 &recipient, const QString &text, const QByteArray &footer)
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
  data.append(footer);

  sendPacket(KittyGG::Packets::P_MSG_SEND, data, data.size());
}

void KittySDK::GGClient::changeStatus(const quint32 &status, const QString &description)
{
  if(isConnected()) {
    m_status = status;
    m_description = description;

    sendChangeStatusPacket();
  } else {
    setDescription(description);
    setStatus(status);
  }
}

void KittySDK::GGClient::requestRoster()
{
  QByteArray data;

  //type
  data.append((char)0x02);

  //version
  int ver = 0;
  data.append((char*)&ver, sizeof(ver));

  //format
  data.append((char)0x02);

  //unknown
  data.append((char)0x01);

  sendPacket(KittyGG::Packets::P_LIST_REQUEST, data, data.size());
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

  m_status = KittyGG::Statuses::S_UNAVAILABLE_D;
  emit statusChanged(uin(), m_status, m_description);

  m_thread->terminate();
  m_pingTimer.stop();
}

void KittySDK::GGClient::error(QAbstractSocket::SocketError socketError)
{
  qDebug() << "Socket::error(" << socketError << ")" << m_socket->errorString();
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

 // qDebug() << "New packet (" << type << ", " << length << ")";

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

      m_status = m_initialStatus;
      m_description = m_initialDescription;

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
        memset(description, 0, sizeof(description));
        if(description_size > 0) {
          str.readRawData(description, description_size);
          left -= description_size;
        }

        if(uin == m_uin) {
          m_status = status;
          m_description = QString::fromAscii(description, description_size);
        }

        //qDebug() << uin << status << QString::fromAscii(description, description_size);

        emit statusChanged(uin, status, QString::fromAscii(description, description_size));

        delete description;
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

      delete data;
    }
    break;

    case KittyGG::Packets::P_MSG_RECV:
    {
      //qDebug() << "It's P_MSG_RECV";

      int left = length;
      while(left > 0) {
        int read = 0;

        QList<quint32> senders;

        quint32 sender;
        str >> sender;
        read += sizeof(sender);

        senders.append(sender);

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

        quint16 text_attr_length;
        char *text_attr = 0;

        while(read != left) {
          quint8 flag;
          str >> flag;
          //qDebug() << "flag" << flag;
          read += sizeof(flag);

          switch(flag) {
            case 0x01: //conference
            {
              quint32 count;
              str >> count;
              read += sizeof(count);

              for(quint32 i = 0; i < count; i++) {
                quint32 uid;
                str >> uid;
                read += sizeof(uid);

                senders.append(uid);
              }
            }
            break;

            //text attributes
            case 0x02:
            {
              str >> text_attr_length;
              read += sizeof(text_attr_length);

              if(text_attr_length > 0) {
                text_attr = new char[text_attr_length];
                str.readRawData(text_attr, text_attr_length);
                read += text_attr_length;
              }
            }
            break;

            //image
            case 0x05:
            case 0x06:
            {
              quint32 size;
              str >> size;
              read += sizeof(size);

              quint32 crc32;
              str >> crc32;
              read += sizeof(crc32);

              int data_length = left - read;
              char *raw = 0;
              if(data_length > 0) {
                raw = new char[data_length];
                str.readRawData(raw, data_length);
                read += data_length;
              }

              QByteArray data(raw, data_length);

              //filename is specified only in 1st packet
              if(flag == 0x05) {
                int zero = data.indexOf((char)0x00);
                if(zero > -1) {
                  QByteArray fileName = data.mid(0, zero);
                  data = data.mid(zero + 1);

                  GGImgTransfer *img = new GGImgTransfer(fileName, crc32, size);
                  m_imgTransfers.append(img);
                }
              }

              delete raw;

              GGImgTransfer *img = imgTransferByCrc(crc32);
              if(img) {
                img->data.append(data);
                img->received += data.size();
                if(img->received == img->size) {
                  emit imageReceived(sender, img->fileName, img->crc32, img->data);

                  m_imgTransfers.removeAll(img);
                  delete img;
                }
              } else {
                qWarning() << "Image not in transfer list!" << crc32;
              }
            }
            break;
          }
        }

        QString text;
        if(html_length > 0) {
          text = QString::fromAscii(html);
          text.replace(QRegExp("\\s{0,}font-family:'[^']*';\\s{0,}", Qt::CaseInsensitive), "");
          text.replace(QRegExp("\\s{0,}font-size:[^pt]*pt;\\s{0,}", Qt::CaseInsensitive), "");

          QRegExp imgs("<img name=\"([0-9a-f]{8})([0-9a-f]{8})\">", Qt::CaseInsensitive);
          int pos = 0;
          while((pos = imgs.indexIn(text, pos)) != -1) {
            quint32 size = imgs.cap(2).toUInt(0, 16);
            quint32 crc32 = imgs.cap(1).toUInt(0, 16);

            GGImgTransfer *img = imgTransferByCrc(crc32);
            if(!img) {
              requestImage(sender, size, crc32);
            }

            pos += imgs.matchedLength();
          }

          text.replace(imgs, "");
        } else {
          text = plainToHtml(sender, QString::fromLocal8Bit(plain), QByteArray(text_attr, text_attr_length));
        }

        QDateTime qtime = QDateTime::currentDateTime();
        if(qtime.toTime_t() > time) {
          qtime.setTime_t(time);
        }

        if(text.length() > 0) {
          emit messageReceived(senders, qtime, text);
        }

        delete text_attr;
        delete html;
        delete plain;

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

          delete name;
          delete value;

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

    case KittyGG::Packets::P_LIST_REPLY:
    {
      qDebug() << "It's P_LIST_REPLY";

      qint8 type;
      str >> type;

      int ver;
      str >> ver;

      qint8 format;
      str >> format;

      qint8 unknown;
      str >> unknown;

      int left = length - (3 * sizeof(qint8)) - sizeof(int);

      quint8 *data = new quint8[left];
      str.readRawData((char*)data, left);

      QByteArray outData;

      z_stream strm;
      quint8 out[65535];

      strm.zalloc = Z_NULL;
      strm.zfree = Z_NULL;
      strm.opaque = Z_NULL;
      strm.avail_in = left;
      strm.next_in = data;

      int ret = inflateInit(&strm);
      if(ret != Z_OK) {
        qWarning() << "inflateInit failed";
        return;
      }

      do {
        strm.avail_out = sizeof(out);
        strm.next_out = out;

        ret = inflate(&strm, Z_NO_FLUSH);
        if(ret == Z_MEM_ERROR) {
          qWarning() << "inflate error";
          break;
        }

        outData.append((const char*)out, sizeof(out) - strm.avail_out);
      } while(ret != Z_STREAM_END);

      inflateEnd(&strm);

      delete data;

      parseXMLRoster(outData);
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

  qDebug() << uin() << password();

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
  tmp32 = m_initialStatus | 0x4000;
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
  tmp32 = m_initialDescription.size();
  data.append((char*)&tmp32, sizeof(tmp32));
  data.append(m_initialDescription.toLocal8Bit().data(), m_initialDescription.length());

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

QString KittySDK::GGClient::plainToHtml(const quint32 &sender, const QString &plain, const QByteArray &attr)
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

      quint8 length;
      str >> length;
      attr_length -= sizeof(length);

      quint8 type;
      str >> type;
      attr_length -= sizeof(type);

      quint32 size;
      str >> size;
      attr_length -= sizeof(size);

      quint32 crc32;
      str >> crc32;
      attr_length -= sizeof(crc32);

      qDebug() << length << type << size << crc32;
      requestImage(sender, size, crc32);
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

void KittySDK::GGClient::parseXMLRoster(const QString &xml)
{
  QDomDocument doc;
  doc.setContent(xml);

  QDomElement root = doc.documentElement();
  if(root.nodeName() == "ContactBook") {
    QMap <QString, QString> groups;

    QDomElement groupsElement = root.namedItem("Groups").toElement();
    QDomNodeList groupList = groupsElement.elementsByTagName("Group");
    for(int i = 0; i < groupList.count(); i++) {
      QDomElement group = groupList.at(i).toElement();

      QDomElement id = group.namedItem("Id").toElement();
      QDomElement name = group.namedItem("Name").toElement();

      groups.insert(id.firstChild().nodeValue(), name.firstChild().nodeValue());
    }

    QDomElement contactsElement = root.namedItem("Contacts").toElement();
    QDomNodeList contactList = contactsElement.elementsByTagName("Contact");
    for(int i = 0; i < contactList.count(); i++) {
      QDomElement contact = contactList.at(i).toElement();

      QDomElement number = contact.namedItem("GGNumber").toElement();
      QDomElement display = contact.namedItem("ShowName").toElement();
      QDomElement phone = contact.namedItem("MobilePhone").toElement();
      QDomElement email = contact.namedItem("Email").toElement();
      QDomElement homepage = contact.namedItem("WwwAddress").toElement();
      QDomElement firstname = contact.namedItem("FirstName").toElement();
      QDomElement lastname = contact.namedItem("LastName").toElement();
      QDomElement sex = contact.namedItem("Gender").toElement();
      QDomElement birthday = contact.namedItem("Birth").toElement();
      QDomElement city = contact.namedItem("City").toElement();
      QDomElement state = contact.namedItem("Province").toElement();
      QDomElement group = contact.namedItem("Groups").toElement();

      QMap<QString, QString> data;
      data.insert("ShowName", display.firstChild().nodeValue());
      data.insert("MobilePhone", phone.firstChild().nodeValue());
      data.insert("Email", email.firstChild().nodeValue());
      data.insert("WwwAddress", homepage.firstChild().nodeValue());
      data.insert("FirstName", firstname.firstChild().nodeValue());
      data.insert("LastName", lastname.firstChild().nodeValue());
      data.insert("Gender", sex.firstChild().nodeValue());
      data.insert("Birth", birthday.firstChild().nodeValue());
      data.insert("City", city.firstChild().nodeValue());
      data.insert("Province", state.firstChild().nodeValue());

      QString groupId = group.namedItem("GroupId").firstChild().nodeValue();
      if((groupId == "00000000-0000-0000-0000-000000000000") && (group.childNodes().count() > 1)) {
        groupId = group.elementsByTagName("GroupId").at(1).firstChild().nodeValue();
      }

      data.insert("Group", groups.value(groupId));

      quint32 uin = number.firstChild().nodeValue().toUInt();

      emit contactImported(uin, data);
    }
  } else {
    qWarning() << "Wrong format!";
  }
}

void KittySDK::GGClient::requestImage(const quint32 &sender, const quint32 &size, const quint32 &crc32)
{
  QByteArray footer;
  footer.append((char)0x04);
  footer.append((char*)&size, sizeof(size));
  footer.append((char*)&crc32, sizeof(crc32));

  sendMessage(sender, "", footer);
}

KittySDK::GGImgTransfer *KittySDK::GGClient::imgTransferByCrc(const quint32 &crc32)
{
  foreach(GGImgTransfer *img, m_imgTransfers) {
    if(img->crc32 == crc32) {
      return img;
    }
  }

  return 0;
}
