/***************************************************************************
    Connect plugin for NWNX  - hooks implementation
    (c) 2012 virusman (virusman@virusman.ru)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#include <stdint.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <stdarg.h>

#include <limits.h>		/* for PAGESIZE */
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

#include "NWNXApi.h"
#include "ConnectHooks.h"
#include "NWNXConnect.h"

extern CNWNXConnect plugin;

unsigned long lastRet;
char scriptRun = 0;
char runLock = 0;

int (*CNWSMessage__HandlePlayerToServerMessage)(CNWSMessage *pMessage, int nPlayerID, char *pData, int nLen);
int (*CNWSMessage__HandlePlayerToServerServerStatusMessage)(CNWSMessage *pMessage, CNWSPlayer *pPlayer, int nSubtype);
void SendHakList(CNWSMessage *pMessage, int nPlayerID);


int CNWSMessage__HandlePlayerToServerMessage_Hook(CNWSMessage *pMessage, int nPlayerID, char *pData, int nLen)
{
    int nType = pData[1];
    int nSubtype = pData[2];
    plugin.Log(0, "Message: PID %d, type %x, subtype %x\n", nPlayerID, nType, nSubtype);
    if (nType == 1) {
        SendHakList(pMessage, nPlayerID);
    }
    return CNWSMessage__HandlePlayerToServerMessage(pMessage, nPlayerID, pData, nLen);
}

void SendHakList(CNWSMessage *pMessage, int nPlayerID)
{
    unsigned char *pData;
    long unsigned int nSize;

    CNWSModule *pModule = (CNWSModule *) g_pAppManager->ServerExoApp->GetModule();
    if (pModule) {
        plugin.Log(0, "Sending hak list...\n");
        pMessage->CreateWriteMessage(80, -1, 1);
        pMessage->WriteINT(pModule->HakList.Length, 32);
        for (int i = pModule->HakList.Length - 1; i >= 0; --i) {
            pMessage->WriteCExoString(pModule->HakList[i], 32);
            plugin.Log(0, "%s\n", pModule->HakList[i].Text);
        }
        pMessage->WriteCExoString(pModule->m_sCustomTLK, 32);
        plugin.Log(0, "%s\n", pModule->m_sCustomTLK.Text);
        pMessage->GetWriteMessage(&pData, &nSize);
        pMessage->SendServerToPlayerMessage(nPlayerID, 100, 1, pData, nSize);
    }
}

int HookFunctions()
{
    *(void**)&CNWSMessage__HandlePlayerToServerMessage = nx_hook_function((void *) 0x08196544, (void *) CNWSMessage__HandlePlayerToServerMessage_Hook, 5, NX_HOOK_DIRECT | NX_HOOK_RETCODE);
}
