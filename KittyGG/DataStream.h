#ifndef KITTYGG_DATASTREAM_H
#define KITTYGG_DATASTREAM_H

#include <QtCore/QByteArray>

namespace KittyGG
{
	class DataStream
	{
		public:
			DataStream(QByteArray *data);

			DataStream &operator<<(const quint8 &num);
			DataStream &operator<<(const quint16 &num);
			DataStream &operator<<(const quint32 &num);
			DataStream &operator<<(const QByteArray &data);

		private:
			QByteArray *m_data;
	};
}

#endif // KITTYGG_DATASTREAM_H
