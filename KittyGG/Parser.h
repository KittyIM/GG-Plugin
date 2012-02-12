#ifndef KITTYGG_PARSER_H
#define KITTYGG_PARSER_H

#include <QtCore/QRunnable>
#include <QtCore/QObject>
#include <QtCore/QMutex>

namespace KittyGG
{
	class Parser: public QObject, public QRunnable
	{
		Q_OBJECT

		public:
			Parser();

			void run();

			void append(const QByteArray &data);

		signals:
			void packetReceived(quint32 type, quint32 length, QByteArray packet);

		private:
			QByteArray m_buffer;
			QMutex m_mutex;

	};
}

#endif // KITTYGG_PARSER_H
