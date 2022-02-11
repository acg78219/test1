#include "pch.h"
#include "GetHttp.h"
#include <vector>
using namespace std;

typedef struct MacIp
{
    CString Description;
    CString ip;
    CString mac;
}MacIp;

typedef std::vector<MacIp> VECTOR_DATA;

BOOL GetLocoalIPandMac(vector<MacIp>& macip)
{
    CString myIP, myMAC, myNetworkName;
    bool bNetReady = false;
    ULONG outBufLen = 0;
    DWORD dwRetVal = 0;

    PIP_ADAPTER_INFO pAadpterInfo;
    PIP_ADAPTER_INFO pAadpterInfoTmp = NULL;

    MacIp tmpmacip;

    pAadpterInfo = (IP_ADAPTER_INFO*)GlobalAlloc(GMEM_ZEROINIT, sizeof(IP_ADAPTER_INFO));

    if (GetAdaptersInfo(pAadpterInfo, &outBufLen) != ERROR_SUCCESS)
    {
        GlobalFree(pAadpterInfo);
        pAadpterInfo = (IP_ADAPTER_INFO*)GlobalAlloc(GMEM_ZEROINIT, outBufLen);
    }

    if ((dwRetVal = GetAdaptersInfo(pAadpterInfo, &outBufLen)) == NO_ERROR)
    {
        pAadpterInfoTmp = pAadpterInfo;
        myIP = "";

        while (pAadpterInfoTmp)
        {
            if (pAadpterInfoTmp->Type == MIB_IF_TYPE_ETHERNET)
            {
                if (pAadpterInfoTmp->CurrentIpAddress)
                {
                    myIP = pAadpterInfoTmp->CurrentIpAddress->IpAddress.String;
                }
                else
                {
                    myIP = pAadpterInfoTmp->IpAddressList.IpAddress.String;
                }
            }

            myNetworkName = pAadpterInfoTmp->Description;

            if (!myIP.IsEmpty() && (myIP.CompareNoCase(_T("0.0.0.0")) != 0))
            {
                bNetReady = true;
                myMAC.Format(_T("%02X-%02X-%02X-%02X-%02X-%02X"),
                    pAadpterInfoTmp->Address[0],
                    pAadpterInfoTmp->Address[1],
                    pAadpterInfoTmp->Address[2],
                    pAadpterInfoTmp->Address[3],
                    pAadpterInfoTmp->Address[4],
                    pAadpterInfoTmp->Address[5]);
                tmpmacip.Description = myNetworkName;
                tmpmacip.ip = myIP;
                tmpmacip.mac = myMAC;
                macip.push_back(tmpmacip);
                if (pAadpterInfoTmp->IpAddressList.Next)
                {
                    tmpmacip.Description = myNetworkName;
                    tmpmacip.ip = pAadpterInfoTmp->IpAddressList.Next->IpAddress.String;
                    tmpmacip.mac = myMAC;
                    macip.push_back(tmpmacip);
                }
                //break;  
            }
            pAadpterInfoTmp = pAadpterInfoTmp->Next;
        }
    }
    GlobalFree(pAadpterInfo);
    GlobalFree(pAadpterInfoTmp);

    return bNetReady;
}

GetHttp::GetHttp() :mac(_T(""))
{

}
void GetHttp::GetMac()
{
	//获取IP 和MAC 地址
	VECTOR_DATA v;

	GetLocoalIPandMac(v);

	for (int x = 0; x < v.size(); x++)
	{
        mac = v[x].mac;
		//MyMac = v[x].mac;
	}
}


