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

#ifndef USBHOSTCOMM_H
#define USBHOSTCOMM_H

// Based on the official usbostrouter header, modified to remove dependancy on
// the client DLL

#include <e32base.h>

typedef int TOstProtIds;

class RUsbOstComm : public RSessionBase
{
public:
    RUsbOstComm();
    TInt Connect();
    TInt Disconnect();
    TInt Open();
    TInt Close();
    TInt RegisterProtocolID(TOstProtIds aId, TBool aNeedHeader);
    void ReadMessage(TRequestStatus& aStatus, TDes8& aDes);
    TInt ReadCancel();
    void WriteMessage(TRequestStatus& aStatus, const TDesC8& aDes, TBool aHasHeader=EFalse);
    TVersion Version() const;

private:
    enum TUsbOstCmdCode
    {
        EUsbOstCmdCodeFirst,
        EUsbOstCmdConnect,
        EUsbOstCmdDisconnect,
        EUsbOstCmdCodeGetAcmConfig,
        EUsbOstCmdCodeSetAcmConfig,
        EUsbOstCmdCodeOpen,
        EUsbOstCmdCodeClose,
        EUsbOstCmdCodeRegisterId,
        EUsbOstCmdCodeRegisterIds,
        EUsbOstCmdCodeUnRegisterId,
        EUsbOstCmdCodeUnRegisterIds,
        EUsbOstCmdCodeReadMsg,
        EUsbOstCmdCodeReadCancel,
        EUsbOstCmdCodeWriteMsg,
        EUsbOstCmdCodeWriteCancel,
        EUsbOstCmdCodeLast
    };
};

RUsbOstComm::RUsbOstComm()
{
}

TInt RUsbOstComm::Connect()
{
    _LIT(KUsbOstServerName, "!UsbOstRouter");
    _LIT(KUsbOstServerImageName, "usbostrouter");
    const TUid KUsbOstServerUid = { 0x200170BE };
    TInt startupAttempts = 2;
    for(;;) {
        TInt ret = CreateSession(KUsbOstServerName, TVersion(1,0,0));
        if (ret != KErrNotFound && ret != KErrServerTerminated) {
            return ret;
        }

        if (startupAttempts-- == 0) {
            return ret;
        }

        RProcess server;
        ret = server.Create(KUsbOstServerImageName, KNullDesC, KUsbOstServerUid);
        if (ret != KErrNone)
            return ret;

        TRequestStatus serverDiedRequestStatus;
        server.Rendezvous(serverDiedRequestStatus);

        if (serverDiedRequestStatus != KRequestPending) {
            // Abort startup
            server.Kill(KErrNone);
        } else {
            // Logon OK - start the server
            server.Resume();
        }
        User::WaitForRequest(serverDiedRequestStatus);
        ret = (server.ExitType() == EExitPanic) ? KErrGeneral : serverDiedRequestStatus.Int();
        server.Close();

        if (ret != KErrNone && ret != KErrAlreadyExists) {
            return ret;
        }
    }
}

TInt RUsbOstComm::Disconnect()
{
    return SendReceive(EUsbOstCmdDisconnect);
}

TInt RUsbOstComm::Open()
{
    return SendReceive(EUsbOstCmdCodeOpen);
}

TInt RUsbOstComm::Close()
{
    TInt err = SendReceive(EUsbOstCmdCodeClose);
    RHandleBase::Close();
    return err;
}

TInt RUsbOstComm::RegisterProtocolID(const TOstProtIds aId, TBool aNeedHeader)
{
    TIpcArgs args(aId, aNeedHeader);
    return SendReceive(EUsbOstCmdCodeRegisterId, args);
}

void RUsbOstComm::ReadMessage(TRequestStatus& aStatus, TDes8& aDes)
{
    TIpcArgs args(aDes.MaxLength(), &aDes);
    SendReceive(EUsbOstCmdCodeReadMsg, args, aStatus);
}

TInt RUsbOstComm::ReadCancel()
{
    return SendReceive(EUsbOstCmdCodeReadCancel);
}

void RUsbOstComm::WriteMessage(TRequestStatus& aStatus, const TDesC8& aDes, TBool aHasHeader)
{
    TIpcArgs args(aHasHeader, aDes.Length(), &aDes);
    SendReceive(EUsbOstCmdCodeWriteMsg, args, aStatus);
}

typedef TVersion (*TVersionFunction)(const RUsbOstComm*);
const TInt KVersionOrdinal = 17;

TVersion RUsbOstComm::Version() const
{
    // This function has to go to the DLL, unfortunately
    TVersion result; // Return 0.0.0 on any error
    RLibrary lib;
    TInt err = lib.Load(_L("usbostcomm"));
    if (err) return result;

    TLibraryFunction fn = lib.Lookup(KVersionOrdinal);
    if (fn)
        result = ((TVersionFunction)fn)(this);
    lib.Close();
    return result;
}

#endif //USBHOSTCOMM_H
