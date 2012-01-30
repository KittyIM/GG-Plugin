#include "Message.h"


#include "Packets/MessageRecv.h"
#include "Managers.h"
#include "zlib/zlib.h"

#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtGui/QTextDocument>
#include <QtGui/QTextBlock>

namespace KittyGG
{

QByteArray Message::htmlToPlain(const QString &html)
{
	QByteArray attr;

	quint16 position = 0;
	quint8 font = 0;
	quint8 color;

	QTextDocument doc;
	doc.setHtml(html);

	for(QTextBlock block = doc.begin(); block != doc.end(); block = block.next()) {
		for(QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
			if(!it.fragment().isValid()) {
				continue;
			}

			QTextCharFormat format = it.fragment().charFormat();
			QTextImageFormat image = format.toImageFormat();

			font = 0;

			if(image.isValid()) {
				font = KittyGG::Message::Image;
			} else {
				font = KittyGG::Message::Color;

				if(format.font().bold()) {
					font |= KittyGG::Message::Bold;
				}

				if(format.font().italic()) {
					font |= KittyGG::Message::Italic;
				}

				if(format.font().underline()) {
					font |= KittyGG::Message::Underline;
				}
			}

			attr.append((char*)&position, sizeof(position));
			attr.append((char*)&font, sizeof(font));

			if(font & KittyGG::Message::Color) {
				color = format.foreground().color().red();
				attr.append((char*)&color, sizeof(color));

				color = format.foreground().color().green();
				attr.append((char*)&color, sizeof(color));

				color = format.foreground().color().blue();
				attr.append((char*)&color, sizeof(color));
			}

			if(font & KittyGG::Message::Image) {
				//length
				attr.append(0x09);

				//type
				attr.append(0x01);

				quint32 size;
				quint32 crc_32;

				QFileInfo info(image.name());
				KittyGG::ImageUpload *img = KittyGG::UploadMgr::byFileName(info.fileName());
				if(img) {
					size = img->size;
					crc_32 = img->crc32;
				} else {
					QFile file(image.name());
					if(file.open(QFile::ReadOnly)) {
						size = file.size();
						crc_32 = crc32(0, (Bytef*)file.readAll().constData(), file.size());

						KittyGG::UploadMgr::append(new KittyGG::ImageUpload(info.fileName(), info.path(), crc_32, size));

						file.close();
					}
				}

				attr.append((char*)&size, sizeof(quint32));
				attr.append((char*)&crc_32, sizeof(quint32));
			}

			position += it.fragment().text().size();
		}
	}

	if(attr.size()) {
		quint16 size = attr.size();
		attr.prepend((char*)&size, sizeof(quint16));

		//type
		attr.prepend(0x02);
	}

	return attr;
}

QString Message::plainToHtml(const quint32 &sender, const QString &plain, const QByteArray &attr, MessageRecv *packet)
{
	QDataStream str(attr);
	str.setByteOrder(QDataStream::LittleEndian);

	QString text, fragment;

	bool opened = false;
	quint16 last_pos = 0;
	quint8 red = 0;
	quint8 green = 0;
	quint8 blue = 0;

	int attr_length = attr.size();
	while(attr_length > 0) {
		quint16 pos;
		str >> pos;
		attr_length -= sizeof(pos);

		quint8 font;
		str >> font;
		attr_length -= sizeof(font);

		pos++;
		fragment = plain.mid(last_pos, pos - last_pos);
		last_pos = pos;

		if(opened) {
			text.append("</span>");
			opened = false;
		}

		if(font & KittyGG::Message::Image) {
			quint8 length;
			str >> length;
			attr_length -= sizeof(length);

			quint8 type;
			str >> type;
			attr_length -= sizeof(type);

			quint32 size;
			str >> size;
			attr_length -= sizeof(size);

			quint32 crc32;
			str >> crc32;
			attr_length -= sizeof(crc32);

			packet->addImageRequest(new ImageDetails(size, crc32));
		} else {
			QString style;

			if(font & KittyGG::Message::Color) {
				str >> red;
				attr_length -= sizeof(red);

				str >> green;
				attr_length -= sizeof(green);

				str >> blue;
				attr_length -= sizeof(blue);
			}

			style.append(QString("color: #%1%2%3;").arg(QString::number(red, 16)).arg(QString::number(green, 16)).arg(QString::number(blue, 16)));

			if(font & KittyGG::Message::Bold) {
				style.append("font-weight: bold;");
			}

			if(font & KittyGG::Message::Italic) {
				style.append("font-style: italic;");
			}

			if(font & KittyGG::Message::Underline) {
				style.append("text-decoration: underline;");
			}

			QString code("<span");
			if(!style.isEmpty()) {
				code.append(QString(" style=\"%1\"").arg(style));
			}

			code.append(">");

			fragment.replace("&", "&amp;");
			fragment.replace("<", "&lt;");
			fragment.replace(">", "&gt;");
			fragment.replace("\"", "&quot;");

			text.append(code);
			text.append(fragment);
			opened = true;
		}
	}

	fragment = plain.mid(last_pos);
	fragment.replace("&", "&amp;");
	fragment.replace("<", "&lt;");
	fragment.replace(">", "&gt;");
	fragment.replace("\"", "&quot;");

	text.append(fragment);

	if(opened) {
		text.append("</span>");
	}

	text.replace("\n", "<br>");
	text.replace("\r", "");
	text.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");
	text.replace("  ", " &nbsp;");

	return text;
}

}
