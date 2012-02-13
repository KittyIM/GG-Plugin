#include "Parser.h"

#include <QtCore/QDataStream>

namespace KittyGG
{

Parser::Parser()
{
	setAutoDelete(false);
}

void Parser::run()
{
	m_mutex.lock();

	while(m_buffer.size() >= (int)(sizeof(quint32) * 2)) {
		quint32 type, length;

		QDataStream str(m_buffer);
		str.setByteOrder(QDataStream::LittleEndian);

		str >> type >> length;

		if((quint32)m_buffer.size() >= (length + sizeof(quint32) * 2)) {
			emit packetReceived(type, length, m_buffer.mid(sizeof(quint32) * 2, length));

			m_buffer = m_buffer.mid(length + sizeof(quint32) * 2);
		} else {
			break;
		}
	}

	m_mutex.unlock();
}

void Parser::append(const QByteArray &data)
{
	m_mutex.lock();

	m_buffer.append(data);

	m_mutex.unlock();
}

}
