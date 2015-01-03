/*
  Copyright (c) 2011-2012 - Tőkés Attila

  This file is part of SmtpClient for Qt.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.
*/

#include "mimeattachment.hpp"
#include <QFileInfo>

/* [1] Constructors and Destructors */

MimeAttachment::MimeAttachment(QFile *file)
    : MimeFile(file)
{
}
MimeAttachment::MimeAttachment(const QByteArray& stream, const QString& fileName): MimeFile(stream, fileName)
{

}

MimeAttachment::~MimeAttachment()
{
}

/* [1] --- */


/* [2] Protected methods */

void MimeAttachment::prepare()
{
    this->header += "Content-disposition: attachment";
	this->header.append("; filename*=UTF-8\'\'").append(cName.toUtf8().toPercentEncoding()).append("\r\n");

    mimeString = QString();

    /* === Header Prepare === */

    /* Content-Type */
    mimeString.append("Content-Type: ").append(cType);

	if (cName != "")
	{
        mimeString.append("; name*=UTF-8\'\'").append(cName.toUtf8().toPercentEncoding());
	}

    mimeString.append("\r\n");
    /* ------------ */

    /* Content-Transfer-Encoding */
    mimeString.append("Content-Transfer-Encoding: ");
    switch (cEncoding)
    {
    case _7Bit:
        mimeString.append("7bit\r\n");
        break;
    case _8Bit:
        mimeString.append("8bit\r\n");
        break;
    case Base64:
        mimeString.append("base64\r\n");
        break;
    case QuotedPrintable:
        mimeString.append("quoted-printable\r\n");
        break;
    }
    /* ------------------------ */

    /* Content-Id */
    if (cId != NULL)
        mimeString.append("Content-ID: <").append(cId).append(">\r\n");
    /* ---------- */

    /* Addition header lines */

    mimeString.append(header).append("\r\n");

    /* ------------------------- */

    /* === End of Header Prepare === */

    /* === Content === */
    switch (cEncoding)
    {
    case _7Bit:
        mimeString.append(QString(content).toLatin1());
        break;
    case _8Bit:
        mimeString.append(content);
        break;
    case Base64:
        mimeString.append(formatter.format(content.toBase64()));
        break;
    }
    mimeString.append("\r\n");
    /* === End of Content === */

    prepared = true;

}

QString MimeAttachment::toString()
{
	if(!prepared)
		prepare();
    return mimeString;
}


/* [2] --- */
