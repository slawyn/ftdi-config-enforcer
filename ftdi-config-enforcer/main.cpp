#include <Windows.h>
#include <WinReg.h>
#include <iostream>
#include <string>
#include <locale>
#include <vector>
#include <iomanip>

#define FTDIPATH "SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS\\"
#define STR1 "VID_0403+PID_6001+AXZ001MIA\\0000\\Device Parameters"


#define PATH FTDIPATH STR1
#define LATENCYTIMER "LatencyTimer"
#define PORTNAME "PortName"

#define MAX_KEY_LENGTH 50
#define MAX_VALUE_NAME 50


// Global Variables
DWORD dwSubkeyCountPrevious = 0;
DWORD cdwLatencyTimerParameter = 0x00000001;
DWORD cdwParameterSize = sizeof(cdwLatencyTimerParameter);



LSTATUS MyRegSetKeyValueW(
                        HKEY    hKey,
                        LPCSTR lpSubKey,
                        LPCSTR lpValueName,
                        DWORD   dwType,
                        LPCVOID lpData,
                        DWORD   cbData
                        )
{
    LSTATUS s;

    if (lpSubKey && *lpSubKey)
    {
        s = RegCreateKeyExA(hKey, lpSubKey, 0, 0, 0, KEY_SET_VALUE, 0, &hKey, 0);

        if (s != NOERROR)
        {
            return s;
        }
    }

    s = RegSetValueExA(hKey, lpValueName, 0, dwType, 
        static_cast<PBYTE>(const_cast<void*>(lpData)), cbData);

    if (lpSubKey && *lpSubKey)
    {
        RegCloseKey(hKey);
    }

    return s;
}


VOID vQueryKeys(HKEY hKey) { 


	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    dwSubkeyCount = 0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    dwValuesCount;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode; 

	TCHAR  achValue[MAX_VALUE_NAME]; 
	DWORD cchValue = MAX_VALUE_NAME; 
	CHAR arrcBuffer[2048];
	
	CHAR * arrcPortName[50];
		for(DWORD i = 0; i<50;i++){
			arrcPortName[i] = 0x00;
	}
							
	// Read latency
	DWORD dwLatencyTime;
	DWORD arraysize= sizeof(arrcPortName);
	DWORD dataSize = sizeof(dwLatencyTime);
	DWORD dwSavedIntoBuffer = 0;
	DWORD dwShouldPrint = 0;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&dwSubkeyCount,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&dwValuesCount,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	// Enumerate the subkeys, until RegEnumKeyEx fails.

	if (dwSubkeyCount)
	{	


		if (dwSubkeyCountPrevious != dwSubkeyCount){
			dwSavedIntoBuffer += sprintf(arrcBuffer, "## New Subkeys found \n");
			dwShouldPrint = 1;
		}
		

		dwSubkeyCountPrevious  = dwSubkeyCount;
		for (i=0; i<dwSubkeyCount; i++) 
		{ 
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, i,
					 achKey, 
					 &cbName, 
					 NULL, 
					 NULL, 
					 NULL, 
					 &ftLastWriteTime); 
			if (retCode == ERROR_SUCCESS) 
			{

				// If there is a key
				CHAR  arrtcFullPath[MAX_PATH];
				for(DWORD i = 0; i<MAX_PATH;i++){
					arrtcFullPath[i] = 0x0000;
				}
				
		
				strcat(arrtcFullPath, FTDIPATH);
				strcat(arrtcFullPath, achKey);
				strcat(arrtcFullPath, "\\0000\\Device Parameters");

				
				dwSavedIntoBuffer += sprintf(arrcBuffer+dwSavedIntoBuffer,"(%d)\t-> (%s, %s)\t\t", i+1, arrtcFullPath, PORTNAME);
				if (ERROR_SUCCESS == RegGetValueA(HKEY_LOCAL_MACHINE, arrtcFullPath, PORTNAME, RRF_RT_REG_SZ, nullptr /*type not required*/, &arrcPortName, &arraysize)) {

					dwSavedIntoBuffer += sprintf(arrcBuffer+dwSavedIntoBuffer,"## Portname is %s\n", arrcPortName);
				}else{
						dwSavedIntoBuffer += sprintf(arrcBuffer+dwSavedIntoBuffer,"## Error: Could read portname\n");
				}

				dwSavedIntoBuffer += sprintf(arrcBuffer+dwSavedIntoBuffer,"\t-> (%s, %s)\t",  arrtcFullPath, LATENCYTIMER);
				if (ERROR_SUCCESS == RegGetValueA(HKEY_LOCAL_MACHINE, arrtcFullPath, LATENCYTIMER, RRF_RT_DWORD, nullptr /*type not required*/, &dwLatencyTime, &dataSize)) {
					dwSavedIntoBuffer += sprintf(arrcBuffer+dwSavedIntoBuffer,"## Latency is %i ms ", dwLatencyTime);
						
					// if the value is not equal to 1 ms
					if(dwLatencyTime != cdwLatencyTimerParameter){
							if (ERROR_SUCCESS == MyRegSetKeyValueW(HKEY_LOCAL_MACHINE, arrtcFullPath, LATENCYTIMER, REG_DWORD,&cdwLatencyTimerParameter, cdwParameterSize)){
								dwShouldPrint = 1;
								dwSavedIntoBuffer += sprintf(arrcBuffer+dwSavedIntoBuffer,"## Success! Latency changed to %d ms\n", cdwLatencyTimerParameter);
							}else{
								dwShouldPrint = 1;
								dwSavedIntoBuffer += sprintf(arrcBuffer+dwSavedIntoBuffer,"## Error: Could not change the latency\n");
							}
					}
					else{
							dwSavedIntoBuffer += sprintf(arrcBuffer+dwSavedIntoBuffer,"\n");
					}
					// no CloseKey needed because it is a predefined registry key
				}else {
					dwSavedIntoBuffer += sprintf(arrcBuffer+dwSavedIntoBuffer,"## Error: Couldn't read latency\n");
				}
			}
			
			
			if(dwShouldPrint){
				printf("%s", arrcBuffer);
			}
			
			
			dwSavedIntoBuffer = 0;
			
		}
	} 
	else{
			printf("## Error: No values found\n");                          
	}
	
// Enumerate the key values. 

	if (dwValuesCount) 
	{
		printf( "\nNumber of values: %d\n", dwValuesCount);

		for (i=0, retCode=ERROR_SUCCESS; i<dwValuesCount; i++) 
		{ 
			cchValue = MAX_VALUE_NAME; 
			achValue[0] = '\0'; 
			retCode = RegEnumValue(hKey, i, 
				achValue, 
				&cchValue, 
				NULL, 
				NULL,
				NULL,
				NULL);

			if (retCode == ERROR_SUCCESS ) 
			{ 
				printf("(%d) %s\n", i+1, achValue); 
			} 
		}
	}

}





int main()
{
	DWORD val;
	HKEY hkeyTest;
	
	
	// Run print all
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT(FTDIPATH), 0, KEY_READ, &hkeyTest) == ERROR_SUCCESS)
	{	
		vQueryKeys(hkeyTest);
	}
	RegCloseKey(hkeyTest);
	
	
	/*	*/
	while(1){
		// Try to open the key
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT(FTDIPATH), 0, KEY_READ, &hkeyTest) == ERROR_SUCCESS)
		{	
			vQueryKeys(hkeyTest);
		}
		RegCloseKey(hkeyTest);
		
		Sleep(100);

	}
	


	return 0;
}