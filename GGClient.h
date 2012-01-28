#ifndef GGCLIENT_H
#define GGCLIENT_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtNetwork/QSslSocket>


namespace KittySDK
{
	class GGClient;

	class GGThread: public QThread
	{
			Q_OBJECT

		public:
			GGThread(QObject *parent = 0): QThread(parent), stop(false) { }

			void run();
			void bufferAppend(const QByteArray &buf);

			QMutex mutex;
			QByteArray buffer;
			bool stop;

		signals:
			void packetReceived(quint32 type, quint32 length, QByteArray packet);
	};

	struct GGImgDownload
	{
			GGImgDownload(const QString &fileName, const quint32 &crc32, const quint32 &size):
				fileName(fileName),
				size(size),
				received(0),
				crc32(crc32)
			{ }

			QString fileName;
			QByteArray data;
			quint32 size;
			quint32 received;
			quint32 crc32;
	};

	struct GGImgUpload
	{
			GGImgUpload(const QString &fileName, const QString &filePath, const quint32 &crc32, const quint32 &size):
				fileName(fileName),
				filePath(filePath),
				size(size),
				crc32(crc32)
			{ }

			QString fileName;
			QString filePath;
			quint32 size;
			quint32 crc32;
	};

	class GGClient: public QObject
	{
			Q_OBJECT
			Q_PROPERTY(QString host READ host WRITE setHost)
			Q_PROPERTY(int port READ port WRITE setPort)
			Q_PROPERTY(quint32 uin READ uin WRITE setUin)
			Q_PROPERTY(QString password READ password WRITE setPassword)
			Q_PROPERTY(quint32 status READ status WRITE setStatus)
			Q_PROPERTY(QString description READ description WRITE setDescription)

			friend class GGThread;

		public:
			explicit GGClient(QObject *parent = 0);
			~GGClient();

			QString host() const { return m_host; }
			void setHost(const QString &host) { m_host = host; }

			int port() const { return m_port; }
			void setPort(const int &port) { m_port = port; }

			quint32 uin() const { return m_uin; }
			void setUin(const quint32 &uin) { m_uin = uin; }

			QString password() const { return m_password; }
			void setPassword(const QString &password) { m_password = password; }

			quint32 status() const { return m_status; }
			void setStatus(const quint32 &status);

			QString description() const { return m_description; }
			void setDescription(const QString &description);

			void setAccount(const quint32 &uin, const QString &passsword);
			bool isConnected();

			void addContact(const quint32 &uin);
			void removeContact(const quint32 &uin);

		public slots:
			void connectToHost(const QString &host, const int &port = 8074);
			void connectToHostSSL(const QString &host, const int &port = 443);
			void sendMessage(const quint32 &recipient, const QString &text, const QByteArray &footer = QByteArray());
			void sendImage(const quint32 &recipient, GGImgUpload *image);
			void changeStatus(const quint32 &status, const QString &description);
			void requestRoster();
			void parseXMLRoster(const QString &xml);

		signals:
			void statusChanged(const quint32 &uin, const quint32 &status, const QString &description);
			void messageReceived(QList<quint32> senders, const QDateTime &time, const QString &plain);
			void userDataReceived(const quint32 &uin, const QString &name, const QString &data);
			void xmlActionReceived(const QString &xmlAction);
			void contactImported(const quint32 &uin, const QMap<QString, QString> &data);
			void imageReceived(const quint32 &sender, const QString &fileName, const quint32 &crc32, const QByteArray &data);
			void typingNotifyReceived(const quint32 &sender, const int &length);

		private slots:
			void readSocket();
			void connected();
			void disconnected();
			void error(QAbstractSocket::SocketError socketError);
			void hostFound();
			void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
			void stateChanged(QAbstractSocket::SocketState socketState);
			void processPacket(const quint32 &type, const quint32 &length, QByteArray packet);
			void sendLoginPacket(const quint32 &seed);
			void sendRosterPacket();
			void sendChangeStatusPacket();
			void sendPingPacket();
			void sendPacket(const int &type, const QByteArray &data = QByteArray(), const quint32 &size = 0);
			QByteArray htmlToPlain(const QString &html);
			QString richToPlain(const QString &html);
			QString plainToHtml(const quint32 &sender, const QString &plain, const QByteArray &attr);
			void requestImage(const quint32 &sender, const quint32 &size, const quint32 &crc32);
			GGImgDownload *imgDownloadByCrc(const quint32 &crc32);
			GGImgUpload *imgUploadByCrc(const quint32 &crc32);
			GGImgUpload *imgUploadByFileName(const QString &fileName);

		private:
			QString m_host;
			int m_port;
			quint32 m_uin;
			QString m_password;
			quint32 m_status;
			QString m_description;
			quint32 m_initialStatus;
			QString m_initialDescription;
			QTimer m_pingTimer;
			QSslSocket *m_socket;
			QList<quint32> m_roster;
			QList<GGImgDownload*> m_imgDownloads;
			QList<GGImgUpload*> m_imgUploads;
			GGThread *m_thread;
	};
}

#endif // GGCLIENT_H
