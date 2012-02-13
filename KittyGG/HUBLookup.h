#ifndef KITTYGG_HUBLOOKUP_H
#define KITTYGG_HUBLOOKUP_H

#include <QtCore/QRunnable>
#include <QtCore/QObject>

namespace KittyGG
{
	class HUBLookup: public QObject, public QRunnable
	{
		Q_OBJECT

		public:
			void run();

		signals:
			void serverFound(QString hostname);
	};
}

#endif // KITTYGG_HUBLOOKUP_H
