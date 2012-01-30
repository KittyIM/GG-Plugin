#ifndef KITTYGG_MANAGERS_H
#define KITTYGG_MANAGERS_H

#include <QtCore/QString>
#include <QtCore/QList>

namespace KittyGG
{
	struct ImageDetails
	{
		ImageDetails(const quint32 &size, const quint32 &crc32):
			size(size),
			crc32(crc32)
		{ }

		quint32 size;
		quint32 crc32;
	};

	struct ImageDownload: public ImageDetails
	{
			ImageDownload(const quint32 &size, const quint32 &crc32, const QString &fileName, const QByteArray &data = QByteArray()):
			ImageDetails(size, crc32),
			fileName(fileName),
			data(data)
		{
		}

		QString fileName;
		QByteArray data;
	};

	struct ImageUpload: public ImageDetails
	{
			ImageUpload(const QString &fileName, const QString &filePath, const quint32 &crc32, const quint32 &size):
				ImageDetails(size, crc32),
				fileName(fileName),
				filePath(filePath)
			{ }

			QString fileName;
			QString filePath;
	};

	class DownloadMgr
	{
		public:
			static ImageDownload* byCrc32(const quint32 &crc32);
			static void append(ImageDownload *item);
			static void remove(ImageDownload *item);

		private:
			static QList<ImageDownload*> m_list;
	};

	class UploadMgr
	{
		public:
			static ImageUpload* byCrc32(const quint32 &crc32);
			static ImageUpload* byFileName(const QString &fileName);
			static void append(ImageUpload *item);

		private:
			static QList<ImageUpload*> m_list;
	};
}

#endif // KITTYGG_MANAGERS_H
