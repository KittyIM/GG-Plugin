#ifndef KITTYGG_MESSAGE_H
#define KITTYGG_MESSAGE_H

#include <QtCore/QString>

namespace KittyGG
{
	class Message
	{
		public:
			enum Font
			{
				Bold		= 0x01,
				Italic		= 0x02,
				Underline	= 0x04,
				Color		= 0x08,
				Image		= 0x80
			};

		public:
			static QByteArray htmlToPlain(const QString &html);
			static QString plainToHtml(const quint32 &sender, const QString &plain, const QByteArray &attr, class MessageRecv *packet);
	};
}

#endif // KITTYGG_MESSAGE_H
