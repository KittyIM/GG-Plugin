#include "MessageRecv.h"

#include "constants.h"

#include <QtCore/QRegExp>

namespace KittyGG
{

MessageRecv::MessageRecv():
	m_seq(QDateTime::currentDateTime().toTime_t()),
	m_timeStamp(QDateTime::currentDateTime()),
	m_msgClass(0x08),
	m_imageUpload(0),
	m_imageDownload(0)
{
}

MessageRecv::~MessageRecv()
{
	qDeleteAll(m_imageRequests);
	delete m_imageUpload;
	delete m_imageDownload;
}

MessageRecv MessageRecv::fromData(const QByteArray &data)
{
	QList<quint32> senders;
	quint32 sender;
	quint32 seq;
	quint32 time;
	quint32 msgclass;
	quint32 offset_plain;
	quint32 offset_attributes;

	QDataStream str(data);
	str.setByteOrder(QDataStream::LittleEndian);

	str >> sender;
	str >> seq;
	str >> time;
	str >> msgclass;
	str >> offset_plain;
	str >> offset_attributes;

	senders.append(sender);

	quint32 html_length = offset_plain - (sizeof(quint32) * 6);
	char *html = 0;
	if(html_length > 0) {
		html = new char[html_length];

		str.readRawData(html, html_length);
	}

	quint32 plain_length = offset_attributes - offset_plain;
	char *plain = 0;
	if(plain_length > 0) {
		plain = new char[plain_length];

		str.readRawData(plain, plain_length);
	}

	QDateTime qtime = QDateTime::currentDateTime();
	if(qtime.toTime_t() > time) {
		qtime.setTime_t(time);
	}

	MessageRecv packet;
	packet.setSeq(seq);
	packet.setTimeStamp(qtime);
	packet.setMsgClass(msgclass);
	packet.setPlainBody(QString::fromLocal8Bit(plain));

	quint16 text_attr_length;
	char *text_attr = 0;
	quint8 flag;

	while(!str.atEnd()) {
		str >> flag;

		switch(flag) {
			case 0x01: //conference
			{
				quint32 count;
				str >> count;

				for(quint32 i = 0; i < count; ++i) {
					quint32 uin;
					str >> uin;

					senders.append(uin);
				}
			}
			break;

			//text attributes
			case 0x02:
			{
				str >> text_attr_length;

				if(text_attr_length > 0) {
					text_attr = new char[text_attr_length];
					str.readRawData(text_attr, text_attr_length);
				}
			}
			break;

			//image request
			case 0x04:
			{
				quint32 size;
				str >> size;

				quint32 crc32;
				str >> crc32;

				packet.setImageUpload(new ImageDetails(size, crc32));
			}
			break;

			//image
			case 0x05:
			case 0x06:
			{
				quint32 size;
				quint32 crc32;
				QString fileName;

				str >> size;
				str >> crc32;

				int data_length = str.device()->bytesAvailable();
				char *raw = 0;
				if(data_length > 0) {
					raw = new char[data_length];
					str.readRawData(raw, data_length);
				}

				QByteArray data(raw, data_length);

				//filename is specified only in 1st packet
				if(flag == 0x05) {
					int zero = data.indexOf((char)0x00);
					if(zero > -1) {
						fileName = data.mid(0, zero);
						data = data.mid(zero + 1);
					}
				}

				packet.setImageDownload(new ImageDownloadInfo(flag, size, crc32, data, fileName));

				delete raw;
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

			packet.addImageRequest(new ImageDetails(size, crc32));

			pos += imgs.matchedLength();
		}

		text.replace(imgs, "");
	} else {
		text = Message::plainToHtml(sender, QString::fromLocal8Bit(plain), QByteArray(text_attr, text_attr_length), &packet);
	}

	packet.setHtmlBody(text);
	packet.setUins(senders);

	delete text_attr;
	delete html;
	delete plain;

	return packet;
}

}
