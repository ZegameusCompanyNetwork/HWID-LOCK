#include <atlbase.h>
#include <atlstr.h>
#include <comutil.h>
#include <wbemidl.h>
#include <string>
#include <windows.h>
#include <wininet.h>
#include <tchar.h>
#include <winsock2.h>
#include <Urlmon.h>
#include "sha256.h"
#include <iostream>
#include <fstream>
#include <iphlpapi.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Iphlpapi.lib")

using namespace std;

string checkhash = "360f8950b73d566ff22ade0f23b43fac949f5726eff31bc018a9d5a519660851"; // Hash de prueba, necesario cambiar en programa final por el del usuario
string userhash;

bool CheckHash(string& userhash);
void PrintMACaddress(BYTE* addr);
void GetMACaddress();
string GetIP();
string GetHDDSerial();
void GetPhysicalDriveSerialNumber(UINT nDriveNumber IN, CString& strSerialNumber OUT);

int _tmain(int argc, _TCHAR* argv[])
{
	userhash = sha256(GetHDDSerial() + GetIP());
	cout << "Hash del Usuario:  " << userhash << endl;
	CheckHash(userhash);
	GetMACaddress();
	system("\npause");

	return 0;
}

bool CheckHash(string& userhash)
{
	if (checkhash.compare(userhash) == 0) { cout << "Coincide" << endl; }
	else
	{
		cout << "Hash de comprobaci¢n: " << checkhash << endl;//¢ es ó, en consola ó es 3/4
		cout << "No coincide!" << endl;
	}
	return false;
}

void PrintMACaddress(BYTE* addr) { for (int i = 0; i < 8; i++) { printf("%x ", *addr++); } }

void GetMACaddress()
{
	IP_ADAPTER_INFO AdapterInfo[16];
	DWORD dwBufLen = sizeof(AdapterInfo);

	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if (dwStatus != ERROR_SUCCESS)
	{
		printf("GetAdaptersInfo falló. err=%d\n", GetLastError());
		return;
	}

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	do
	{
		PrintMACaddress(pAdapterInfo->Address);
		pAdapterInfo = pAdapterInfo->Next;
	}
	while (pAdapterInfo);
}

string GetIP()
{
	string ip;

	TCHAR url[] = TEXT("http://zcnr.ddns.net/raw");

	TCHAR path[MAX_PATH];

	GetCurrentDirectory(MAX_PATH, path);

	wsprintf(path, TEXT("%s\\ip.txt"), path);

	auto res = URLDownloadToFile(nullptr, url, path, 0, nullptr);

	if (res == S_OK)
	{
		ifstream myfile("ip.txt");
		if (myfile.is_open())
		{
			if (myfile.good())
			{
				getline(myfile, ip);
			}
		}
		myfile.close();
		remove("ip.txt");
	}

	return ip;
}

string GetHDDSerial()
{
	CString strResult;
	try
	{
		auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		ATLENSURE_SUCCEEDED(hr);

		CString strSerialNumber;
		UINT nDriveNumber = 0;
		GetPhysicalDriveSerialNumber(nDriveNumber, strSerialNumber);
		strResult.Format(static_cast<const wchar_t*>(strSerialNumber));
	}
	catch (CAtlException& e)
	{
		strResult.Format(_T("Fallo al obtener número serial. Error code: 0x%08X"),
		                 static_cast<HRESULT>(e));
	}

	CoUninitialize();
	CT2CA pszConvertedAnsiString(strResult);
	string serial(pszConvertedAnsiString);
	return serial;
}

void GetPhysicalDriveSerialNumber(UINT nDriveNumber IN, CString& strSerialNumber OUT)
{
	strSerialNumber.Empty();

	CString strDrivePath;
	strDrivePath.Format(_T("\\\\.\\PhysicalDrive%u"), nDriveNumber);

	auto hr = CoInitializeSecurity(
		nullptr,
		-1,
		nullptr,
		nullptr,
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		nullptr,
		EOAC_NONE,
		nullptr);

	ATLENSURE_SUCCEEDED(hr);
	CComPtr<IWbemLocator> pIWbemLocator;
	hr = CoCreateInstance(CLSID_WbemLocator, nullptr,
	                      CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pIWbemLocator));

	ATLENSURE_SUCCEEDED(hr);

	CComPtr<IWbemServices> pIWbemServices;
	hr = pIWbemLocator->ConnectServer(L"ROOT\\CIMV2",
	                                  nullptr, nullptr, nullptr, NULL, nullptr, nullptr, &pIWbemServices);

	ATLENSURE_SUCCEEDED(hr);

	hr = CoSetProxyBlanket(
		pIWbemServices,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		nullptr,
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		nullptr,
		EOAC_NONE);

	ATLENSURE_SUCCEEDED(hr);

	const BSTR szQueryLanguage = L"WQL";
	const BSTR szQuery = L"SELECT Tag, SerialNumber FROM Win32_PhysicalMedia";
	CComPtr<IEnumWbemClassObject> pIEnumWbemClassObject;
	hr = pIWbemServices->ExecQuery(
		szQueryLanguage,
		szQuery,
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		nullptr,
		&pIEnumWbemClassObject);

	ATLENSURE_SUCCEEDED(hr);

	ULONG uReturn = 0;
	while (pIEnumWbemClassObject)
	{
		CComPtr<IWbemClassObject> pIWbemClassObject;
		hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &uReturn);
		if (0 == uReturn || FAILED(hr))
			break;

		variant_t vtTag;
		variant_t vtSerialNumber;

		hr = pIWbemClassObject->Get(L"Tag", 0, &vtTag, nullptr, nullptr);
		ATLENSURE_SUCCEEDED(hr);

		CString strTag(vtTag.bstrVal);
		if (!strTag.CompareNoCase(strDrivePath))
		{
			hr = pIWbemClassObject->Get(L"SerialNumber", 0, &vtSerialNumber, nullptr, nullptr);
			ATLENSURE_SUCCEEDED(hr);
			strSerialNumber = vtSerialNumber.bstrVal;
			break;
		}
	}
}
