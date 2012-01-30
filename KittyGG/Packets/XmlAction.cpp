#include "XmlAction.h"

namespace KittyGG
{

XmlAction XmlAction::fromData(const QByteArray &data)
{
	QString action = data;

	XmlAction packet;
	packet.setAction(action);
	return packet;
}


}
