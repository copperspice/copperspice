/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

//! [0]
connect(buttonBack, SIGNAL(clicked()), webBrowser, SLOT(GoBack()));
//! [0]


//! [1]
activeX->setProperty("text", "some text");
int value = activeX->property("value");
//! [1]


//! [2]
webBrowser->dynamicCall("GoHome()");
//! [2]


//! [3]
connect(webBrowser, SIGNAL(TitleChanged(QString)),
	this, SLOT(setCaption(QString)));
//! [3]


//! [4]
dispinterface IControl
{
properties:
    [id(1)] BSTR text;
    [id(2)] IFontDisp *font;

methods:
    [id(6)] void showColumn([in] int i);
    [id(3)] bool addColumn([in] BSTR t);
    [id(4)] int fillList([in, out] SAFEARRAY(VARIANT) *list);
    [id(5)] IDispatch *item([in] int i);
};
//! [4]


//! [5]
QAxObject object("<CLSID>");

QString text = object.property("text").toString();
object.setProperty("font", QFont("Times New Roman", 12));

connect(this, SIGNAL(clicked(int)), &object, SLOT(showColumn(int)));
bool ok = object.dynamicCall("addColumn(const QString&)", "Column 1").toBool();

QList<QVariant> varlist;
QList<QVariant> parameters;
parameters << QVariant(varlist);
int n = object.dynamicCall("fillList(QList<QVariant>&)", parameters).toInt();

QAxObject *item = object.querySubItem("item(int)", 5);
//! [5]


//! [6]
IUnknown *iface = 0;
activeX->queryInterface(IID_IUnknown, (void**)&iface);
if (iface) {
    // use the interface
    iface->Release();
}
//! [6]


//! [7]
ctrl->setControl("{8E27C92B-1264-101C-8A2F-040224009C02}");
//! [7]


//! [8]
ctrl->setControl("MSCal.Calendar");
//! [8]


//! [9]
ctrl->setControl("Calendar Control 9.0");
//! [9]


//! [10]
ctrl->setControl("c:/files/file.doc");
//! [10]


//! [11]
<domain/username>:<password>@server/{8E27C92B-1264-101C-8A2F-040224009C02}
//! [11]


//! [12]
{8E27C92B-1264-101C-8A2F-040224009C02}:<LicenseKey>
//! [12]


//! [13]
{8E27C92B-1264-101C-8A2F-040224009C02}&
//! [13]


//! [14]
ctrl->setControl("DOMAIN/user:password@server/{8E27C92B-1264-101C-8A2F-040224009C02}:LicenseKey");
//! [14]


//! [15]
activeX->dynamicCall("Navigate(const QString&)", "qt.nokia.com");
//! [15]


//! [16]
activeX->dynamicCall("Navigate(\"qt.nokia.com\")");
//! [16]


//! [17]
activeX->dynamicCall("Value", 5);
QString text = activeX->dynamicCall("Text").toString();
//! [17]


//! [18]
IWebBrowser2 *webBrowser = 0;
activeX->queryInterface(IID_IWebBrowser2, (void **)&webBrowser);
if (webBrowser) {
    webBrowser->Navigate2(pvarURL);
    webBrowser->Release();
}
//! [18]


//! [19]
QAxWidget outlook("Outlook.Application");
QAxObject *session = outlook.querySubObject("Session");
if (session) {
    QAxObject *defFolder = session->querySubObject(
			    "GetDefaultFolder(OlDefaultFolders)",
			    "olFolderContacts");
    //...
}
//! [19]


//! [20]
void Receiver::slot(const QString &name, int argc, void *argv)
{
    VARIANTARG *params = (VARIANTARG*)argv;
    if (name.startsWith("BeforeNavigate2(")) {
	IDispatch *pDisp = params[argc-1].pdispVal;
	VARIANTARG URL = *params[argc-2].pvarVal;
	VARIANTARG Flags = *params[argc-3].pvarVal;
	VARIANTARG TargetFrameName = *params[argc-4].pvarVal;
	VARIANTARG PostData = *params[argc-5].pvarVal;
	VARIANTARG Headers = *params[argc-6].pvarVal;
	bool *Cancel = params[argc-7].pboolVal;
    }
}
//! [20]
