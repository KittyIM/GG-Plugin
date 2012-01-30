#include "MessageSend.h"

#include "KittyGG/DataStream.h"
#include "KittyGG/Message.h"

#include <QtCore/QRegExp>

namespace KittyGG
{

QByteArray MessageSend::toData() const
{
	QByteArray data;
	DataStream str(&data);

	QString plain = m_htmlBody;
	plain.remove(QRegExp("<[^>]*>"));
	plain.replace("&quot;", "\"");
	plain.replace("&lt;", "<");
	plain.replace("&gt;", ">");
	plain.replace("&amp;", "&");

	str << m_uins.first();
	str << m_seq;
	str << m_msgClass;
	str << (quint32)(sizeof(quint32) * 5);					// offset_plain
	str << (quint32)(sizeof(quint32) * 5 + plain.size());	// offset_attributes
//	str << m_htmlBody.toLatin1();
	str << plain.toLocal8Bit();

	if(m_htmlBody.size()) {
		str <<  Message::htmlToPlain(m_htmlBody);
	}

	foreach(ImageDetails *img, m_imageRequests) {
		str << (quint8)0x04;
		str << img->size;
		str << img->crc32;
	}

	if(m_imageDownload) {
		str << m_imageDownload->type;
		str << m_imageDownload->size;
		str << m_imageDownload->crc32;

		if(m_imageDownload->type == 0x05) {
			str << m_imageDownload->fileName.toLatin1();
			str << (quint8)0x00;
		}

		str << m_imageDownload->data;
	}

	return data;
}

}
