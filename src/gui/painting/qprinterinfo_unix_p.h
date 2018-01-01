/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPRINTERINFO_UNIX_P_H
#define QPRINTERINFO_UNIX_P_H

#ifndef QT_NO_NIS
#  ifndef BOOL_DEFINED
#    define BOOL_DEFINED
#  endif

#  include <sys/types.h>
#  include <rpc/rpc.h>
#  include <rpcsvc/ypclnt.h>
#  include <rpcsvc/yp_prot.h>
#endif

#ifdef Success
#  undef Success
#endif

#include <ctype.h>

QT_BEGIN_NAMESPACE


#ifndef QT_NO_PRINTER

struct QPrinterDescription {
   QPrinterDescription(const QString &n, const QString &h, const QString &c, const QStringList &a)
      : name(n), host(h), comment(c), aliases(a) {}
   QString name;
   QString host;
   QString comment;
   QStringList aliases;
   bool samePrinter(const QString &printer) const {
      return name == printer || aliases.contains(printer);
   }
};

enum { Success = 's', Unavail = 'u', NotFound = 'n', TryAgain = 't' };
enum { Continue = 'c', Return = 'r' };

void qt_perhapsAddPrinter(QList<QPrinterDescription> *printers, const QString &name,
                          QString host, QString comment,
                          QStringList aliases = QStringList());
void qt_parsePrinterDesc(QString printerDesc, QList<QPrinterDescription> *printers);

int qt_parsePrintcap(QList<QPrinterDescription> *printers, const QString &fileName);
QString qt_getDefaultFromHomePrinters();
void qt_parseEtcLpPrinters(QList<QPrinterDescription> *printers);
char *qt_parsePrintersConf(QList<QPrinterDescription> *printers, bool *found = 0);

#ifndef QT_NO_NIS
#if defined(Q_C_CALLBACKS)
extern "C" {
#endif
   int qt_pd_foreach(int /*status */, char * /*key */, int /*keyLen */,
                     char *val, int valLen, char *data);

#if defined(Q_C_CALLBACKS)
}
#endif
int qt_retrieveNisPrinters(QList<QPrinterDescription> *printers);
#endif // QT_NO_NIS
char *qt_parseNsswitchPrintersEntry(QList<QPrinterDescription> *printers, char *line);
char *qt_parseNsswitchConf(QList<QPrinterDescription> *printers);
void qt_parseEtcLpMember(QList<QPrinterDescription> *printers);
void qt_parseSpoolInterface(QList<QPrinterDescription> *printers);
void qt_parseQconfig(QList<QPrinterDescription> *printers);
int qt_getLprPrinters(QList<QPrinterDescription> &printers);

#endif // QT_NO_PRINTER

QT_END_NAMESPACE

#endif // QPRINTERINFO_UNIX_P_H
