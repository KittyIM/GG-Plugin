#include "Managers.h"

namespace KittyGG
{

QList<ImageDownload*> DownloadMgr::m_list;
QList<ImageUpload*> UploadMgr::m_list;


ImageDownload *DownloadMgr::byCrc32(const quint32 &crc32)
{
	foreach(ImageDownload *img, m_list) {
		if(img->crc32 == crc32) {
			return img;
		}
	}

	return 0;
}

void DownloadMgr::append(ImageDownload *item)
{
	m_list.append(item);
}

void DownloadMgr::remove(ImageDownload *item)
{
	m_list.removeAll(item);
}

ImageUpload *UploadMgr::byCrc32(const quint32 &crc32)
{
	foreach(ImageUpload *img, m_list) {
		if(img->crc32 == crc32) {
			return img;
		}
	}

	return 0;
}

ImageUpload *UploadMgr::byFileName(const QString &fileName)
{
	foreach(ImageUpload *img, m_list) {
		if(img->fileName == fileName) {
			return img;
		}
	}

	return 0;
}

void UploadMgr::append(ImageUpload *item)
{
	m_list.append(item);
}

}
