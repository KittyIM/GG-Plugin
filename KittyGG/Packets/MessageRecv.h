#ifndef KITTYGG_MESSAGERECVPACKET_H
#define KITTYGG_MESSAGERECVPACKET_H

#include "KittyGG/Managers.h"
#include "KittyGG/Packet.h"

#include <QtCore/QDateTime>

namespace KittyGG
{
	struct ImageDownloadInfo: public ImageDownload
	{
		ImageDownloadInfo(const quint8 &type, const quint32 &size, const quint32 &crc32, const QByteArray &data, const QString &fileName = QString()):
			ImageDownload(size, crc32, fileName, data),
			type(type)
		{
		}

		quint8 type;
	};

	class MessageRecv: public Packet
	{
		public:
			MessageRecv();
			~MessageRecv();

			enum { Type = 0x2e };
			virtual quint32 packetType() const { return Type; }

			quint32 uin() const { return (m_uins.isEmpty()) ? 0 : m_uins.first(); }
			void setUin(const quint32 &uin) { (m_uins.isEmpty()) ? m_uins.append(uin) : m_uins.replace(0, uin); }

			QList<quint32> uins() const { return m_uins; }
			void setUins(const QList<quint32> &uins) { m_uins = uins; }

			quint32 seq() const { return m_seq; }
			void setSeq(const quint32 &seq) { m_seq = seq; }

			QDateTime timeStamp() const { return m_timeStamp; }
			void setTimeStamp(const QDateTime &time) { m_timeStamp = time; }

			quint32 msgClass() const { return m_msgClass; }
			void setMsgClass(const quint32 &msgClass) { m_msgClass = msgClass; }

			QString htmlBody() const { return m_htmlBody; }
			void setHtmlBody(const QString &htmlBody) { m_htmlBody = htmlBody; }

			QString plainBody() const { return m_plainBody; }
			void setPlainBody(const QString &plainBody) { m_plainBody = plainBody; }

			QList<ImageDetails*> imageRequests() { return m_imageRequests; }
			void addImageRequest(ImageDetails *imageRequest) { m_imageRequests << imageRequest; }

			ImageDetails* imageUpload() { return m_imageUpload; }
			void setImageUpload(ImageDetails *imageUpload) { m_imageUpload = imageUpload; }

			ImageDownloadInfo* imageDownload() { return m_imageDownload; }
			void setImageDownload(ImageDownloadInfo *imageDownload) { m_imageDownload = imageDownload; }

			static MessageRecv fromData(const QByteArray &data);
			QByteArray toData() const { return QByteArray(); }

		protected:
			QList<quint32> m_uins;
			quint32 m_seq;
			QDateTime m_timeStamp;
			quint32 m_msgClass;
			QString m_htmlBody;
			QString m_plainBody;
			QList<ImageDetails*> m_imageRequests;
			ImageDetails* m_imageUpload;
			ImageDownloadInfo* m_imageDownload;
	};
}

#endif // KITTYGG_MESSAGERECVPACKET_H
