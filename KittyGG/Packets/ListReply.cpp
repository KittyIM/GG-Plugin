#include "ListReply.h"

#include "zlib/zlib.h"

namespace KittyGG
{

ListReply::ListReply(const quint8 &type, const quint32 &version):
	ListRequest(type, version)
{
}

ListReply ListReply::fromData(const QByteArray &data)
{
	qint8 type;
	quint32 ver;
	qint8 format;
	qint8 unknown;
	QByteArray reply;

	QDataStream str(data);
	str.setByteOrder(QDataStream::LittleEndian);

	str >> type;
	str >> ver;
	str >> format;
	str >> unknown;

	int left = data.size() - (3 * sizeof(qint8)) - sizeof(quint32);

	quint8 *raw = new quint8[left];
	str.readRawData((char*)raw, left);

	z_stream strm;
	quint8 out[65535];

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = left;
	strm.next_in = raw;

	int ret = inflateInit(&strm);
	if(ret == Z_OK) {
		do {
			strm.avail_out = sizeof(out);
			strm.next_out = out;

			ret = inflate(&strm, Z_NO_FLUSH);
			if(ret == Z_MEM_ERROR) {
				qWarning() << "inflate error";
				break;
			}

			reply.append((const char*)out, sizeof(out) - strm.avail_out);
		} while(ret != Z_STREAM_END);

		inflateEnd(&strm);

		delete raw;
	} else {
		qWarning() << "inflateInit failed";
	}

	ListReply packet(type, ver);
	packet.setReply(reply);
	return packet;
}

}
