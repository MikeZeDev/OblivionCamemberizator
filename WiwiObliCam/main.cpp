#include "obse/PluginAPI.h"
#include <tlhelp32.h>

IDebugLog		gLog("wiwiland_Oblivion_Camenberizator.log");

PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
OBSESerializationInterface	* g_serialization = NULL;
OBSEArrayVarInterface		* g_arrayIntfc = NULL;


unsigned long GetTargetProcessIdFromProcname(char *procName)
{
PROCESSENTRY32 pe;
HANDLE thSnapshot;
BOOL retval, ProcFound = false;

thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

if(thSnapshot == INVALID_HANDLE_VALUE)
{
	_MESSAGE("Erreur: impossible de lister les processus");
	return false;
}

pe.dwSize = sizeof(PROCESSENTRY32);

retval = Process32First(thSnapshot, &pe);

while(retval)
{
	if( lstrcmpi(pe.szExeFile, procName) == 0 )
{
	ProcFound = true;
	break;
}

retval = Process32Next(thSnapshot,&pe);
pe.dwSize = sizeof(PROCESSENTRY32);
}

if (!ProcFound )
{
	_MESSAGE("Impossible de trouver Oblivion.exe...");
}
return pe.th32ProcessID;
}

//INvocation: Nom de la créature
/*
ORIGINE
005A4D71                         8B4C24 28               MOV     ECX, DWORD PTR SS:[ESP+28]
005A4D75                         51                      PUSH    ECX
005A4D76                         8BC8                    MOV     ECX, EAX
005A4D78                         C64424 58 02            MOV     BYTE PTR SS:[ESP+58], 2
005A4D7D                         E8 1E55F3FF             CALL    004DA2A0
005A4D82                         50                      PUSH    EAX
005A4D83                         68 8CBEA600             PUSH    00A6BE8C     ; ASCII "%s's %s"         

PATCH
005A4D71                           8BC8                  MOV     ECX, EAX
005A4D73                           C6 44 24 54 02        MOV     BYTE PTR SS:[ESP+54], 2
005A4D78                           E8 23 55 F3 FF        CALL    004DA2A0
005A4D7D                           50                    PUSH    EAX
005A4D7E                           8B 4C 24 2C           MOV     ECX, DWORD PTR SS:[ESP+2C]
005A4D82                           51                    PUSH    ECX
005A4D83                           68 33 55 5A 00        PUSH    005A5533    ; ASCII "%s de %s"  

En 5A5533 on a des octets de bourrage 0xCC, en gros de la place pour mettre "%s de %s"\0
*/
DWORD addr_INvocation = 0x5A4D72;
DWORD addr_INvocationstring = 0x005A5533;
const char INvocationstring[10]={0x25,0x73,0x20,0x64,0x65,0x20,0x25,0x73,0x00}; //format of summoning's name
const char INvocation[22]={0xc8,0xc6,0x44,0x24,0x54,0x02,0xe8,0x23,0x55,0xf3,0xff,0x50,0x8b,0x4c,0x24,0x2c,0x51, 0x68, 0x33, 0x55, 0x5A, 00 };
const char OriginalSummonCode[22]={0x4c,0x24,0x28,0x51,0x8b,0xc8,0xc6,0x44,0x24,0x58,0x02,0xe8,0x1e,0x55,0xf3,0xff,0x50,0x68,0x8c,0xbe,0xa6,0x00};
const char OriginalSummonString2[11]={0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,0x00};


DWORD addr_AugmenteDE = 0xA73E6C;
const char OrigBy[3] = "by";
const char De[3] = "de";

DWORD addr_EffetEnchantement = 0xA6BCB0;
const char OrigEffetEnchantement[62] = "That effect has already been added.  Edit the effect instead.";
const char EffetEnchantement[62]="Cet effet a déjà été ajouté. Choissez en un autre à la place.";

DWORD addr_DateJournalFormat = 0xA3D170;
const char OrigDateJournalFormat[17] ="%d%s of %s, 3E%d";
const char DateJournalFormat[8] ="Le %d%s"; //On patche que ca


//th, rd, etc...
DWORD addr_Ordinaux = 0xA3D184;
const char OrigOrdinaux[16] = {0x72,0x64,0x00,0x00, 0x6E,0x64,0x00,0x00, 0x73,0x74,0x00,0x00, 0x74,0x68,0x00,0x00};
const char Ordinaux[16] =     {0x65,0x00,0x00,0x00, 0x65,0x00,0x00,0x00, 0x65,0x72,0x00,0x00, 0x65,0x00,0x00,0x00};


/*
//Date : jour/MOIs/An DANS l'inTERFACE
00402F19                       |.  E8 A2F95700           CALL    009828C0
00402F1E                       |.  0FBEC8                MOVSX   ECX, AL
00402F21                           57                    PUSH    EDI// on inverse les deux paramétres, tout simplement
00402F22                           51                    PUSH    ECX//
00402F23                           68 54F9A200           PUSH    00A2F954  ;  ASCII "%d %s, 3E%d"////INVERSER d et s
00402F28                           56                    PUSH    ESI
00402F29                           E8 02FFFFFF           CALL    00402E30*/
DWORD addr_DateMenu = 0xA2F954;
const char OrigDateMenu[12] = "%s %d, 3E%d"; 
const char DateMenu[12] = "%d %s, 3E%d"; 
DWORD addr_PUsDateMenu = 0x402f21;
const char OrigPUsDateMenu[2] = {0x51, 0x57};
const char PUsDateMenu[2] = {0x57, 0x51};


//HEURE
DWORD addr_HeureAMPM = 0xA6CD04;
const char OrigHeureAMPM[8] = {0x61, 0x6D, 00, 00, 0x70, 0x6D, 00, 00};//"am..pm.."
const char HeureAMPM[8]= {0,0,0,0,0,0,0,0};

DWORD addr_HeureInterface = 0x5B90E5;
const char OrigHeureInterface[28] = {0x83 ,0xFE ,0x01 ,0x8B ,0xF8 ,0x7D ,0x07 ,0xBE ,0x0C ,0x00 ,0x00 ,0x00,0xEB ,0x08 ,0x83 ,0xFE ,0x0C ,0x7E ,0x03 ,0x83 ,0xEE ,0x0C ,0x55 ,0xE8 ,0x1F ,0x8E ,0xE4 ,0xFF};
const char HeureInterface[28] =     {0x83 ,0xFE ,0x01 ,0x8B ,0xF8 ,0xEB ,0x07 ,0xBE ,0x0C ,0x00 ,0x00 ,0x00,0xEB ,0x08 ,0x83 ,0xFE ,0x0C ,0x7E ,0x03 ,0x90 ,0x90 ,0x90 ,0x55 ,0xE8 ,0x1F ,0x8E ,0xE4 ,0xFF};

DWORD addr_HeureReposUn = 0x5D72EB;
const char OrigHeureReposUn[27] = {0xF6, 0xC4, 0x41, 0x75, 0x04, 0xD9, 0xC0, 0xEB, 0x0D, 0xD8, 0xD1, 0xDF, 0xE0, 0xD9, 0xC1, 0xF6, 0xC4, 0x05, 0x7A, 0x02, 0xD8, 0xE1, 0xE8, 0xBA, 0xB5, 0x3A, 0x00};
const char HeureReposUn[27]=      {0xF6, 0xC4, 0x41, 0x75, 0x04, 0xD9, 0xC9, 0xEB, 0x0D, 0xD8, 0xD1, 0xDF, 0xE0, 0xD9, 0xC1, 0xF6, 0xC4, 0x05, 0x7A, 0x02, 0x90, 0x90, 0xE8, 0xBA, 0xB5, 0x3A, 0x00};

DWORD addr_HeureReposDeux = 0x05D6F84;
const char OrigHeureReposDeux[27] = {0xF6, 0xC4, 0x41, 0x75, 0x04, 0xD9, 0xC0, 0xEB, 0x0D, 0xD8, 0xD1, 0xDF, 0xE0, 0xD9, 0xC1, 0xF6, 0xC4, 0x05, 0x7A, 0x02, 0xD8, 0xE1, 0xE8, 0x21, 0xB9, 0x3A, 0x00};
const char HeureReposDeux[27]=      {0xF6, 0xC4, 0x41, 0x75, 0x04, 0xD9, 0xC9, 0xEB, 0x0D, 0xD8, 0xD1, 0xDF, 0xE0, 0xD9, 0xC1, 0xF6, 0xC4, 0x05, 0x7A, 0x02, 0x90, 0x90, 0xE8, 0x21, 0xB9, 0x3A, 0x00};

/*
//DATE ECRAN TITRE

004612C2        0FB75424 2A     MOVZX EDX, WORD PTR SS:[ESP+2A]
004612C7        51              PUSH    ECX
004612C8        0FB74C24 2A     MOVZX ECX, WORD PTR SS:[ESP+2A]


004612C2        0FB75424 26     MOVZX EDX, WORD PTR SS:[ESP+26]
004612C7        51              PUSH ECX
004612C8        0FB74C24 2E     MOVZX ECX, WORD PTR SS:[ESP+2E]

*/


DWORD addr_DateEcranTitre = 0x004612C2;
const char OrigDateEcranTitre[12]  = {0x0F,0xB7,0x54,0x24,0x2A, 0x51, 0x0F,0xB7,0x4C,0x24,0x2A,0x52} ;
const char DateEcranTitre[12]  =     {0x0F,0xB7,0x54,0x24,0x26, 0x51, 0x0F,0xB7,0x4C,0x24,0x2E,0x52} ;


bool PatchDateEcranTitre(HANDLE proc)
{
	UInt32	bytesWritten;
	unsigned char buffer[80] ={0};
	bool result = false;



	_MESSAGE("Traduction de la date dans l'écran titre : test de contrôle...");
	ReadProcessMemory(proc,(LPVOID) addr_DateEcranTitre,(LPVOID) buffer,sizeof(OrigDateEcranTitre),&bytesWritten);
	if (memcmp(buffer,OrigDateEcranTitre,sizeof(OrigDateEcranTitre)) == 0)
	{
		_MESSAGE("Traduction de la date dans l'écran titre : test de contrôle OK :)");
		_MESSAGE("Traduction de la date dans l'écran titre : PATCH..... !");
		result = WriteProcessMemory(proc,(LPVOID) addr_DateEcranTitre,(LPVOID) DateEcranTitre,sizeof(DateEcranTitre),&bytesWritten);

		if (result) 
		{_MESSAGE("Traduction de la date dans l'écran titre : PATCH OK !");}
		else 
		{_MESSAGE("Traduction de la date dans l'écran titre : impossible de patcher !"); }
			
		return result;
	}
	else 
	{
		_MESSAGE("Traduction de la date dans l'écran titre : test de contrôle NON OK :(");
		return false;
	}
}
bool PatchHeure(HANDLE proc)
{
	UInt32	bytesWritten;
	unsigned char buffer[60] ={0};
	bool result = false;
	bool Controle = false;
	DWORD oldprotect;

	//Il faut que tout les octets correspondent pour appliquer les patchs

/* On teste d'abord am et "pm"	

005B9124                       |.  BD 08CDA600           MOV     EBP, 00A6CD08       ;  ASCII "pm"//METTRE DES 00
005D7316                       |.  B8 04CDA600           MOV     EAX, 00A6CD04       ;  ASCII "am"//IDEM
*/

	_MESSAGE("Traduction de l'Heure : tests de contrôle...");
	ReadProcessMemory(proc, (LPCVOID)addr_HeureAMPM,(LPVOID) buffer,sizeof(OrigHeureAMPM),&bytesWritten);
	if (memcmp(buffer,OrigHeureAMPM, sizeof(OrigHeureAMPM)) != 0)
	{
		_ERROR("Traduction de l'Heure : contrôle n°1 non ok... Rien ne sera modifié.");
		return false;
	}


	if (VirtualProtectEx(proc,(LPVOID)addr_HeureAMPM,8,PAGE_EXECUTE_READWRITE,&oldprotect) == 0 )
	{
		_ERROR("Traduction de l'Heure : Impossible de déprotéger 'am/pm'... Rien ne sera modifié.");
		return false;
	}

/*

//HEURE FR : INTERFACE

ORIGINAL
005B90E5                       |.  83FE 01               CMP     ESI, 1
005B90E8                       |.  8BF8                  MOV     EDI, EAX
005B90EA                       |.  7D 07                 JGE     SHORT 005B90F3
005B90EC                       |.  BE 0C000000           MOV     ESI, 0C
005B90F1                       |.  EB 08                 JMP     SHORT 005B90FB
005B90F3                       |>  83FE 0C               CMP     ESI, 0C
005B90F6                       |.  7E 03                 JLE     SHORT 005B90FB
005B90F8                       |.  83EE 0C               SUB     ESI, 0C
005B90FB                       |>  55                    PUSH    EBP
005B90FC                       |.  E8 1F8EE4FF           CALL    00401F20

PATCHE
005B90E5                       |.  83FE 01               CMP     ESI, 1
005B90E8                       |.  8BF8                  MOV     EDI, EAX
005B90EA                           EB 07                 JMP     SHORT 005B90F3 //on force le saut
005B90EC                       |.  BE 0C000000           MOV     ESI, 0C
005B90F1                       |.  EB 08                 JMP     SHORT 005B90FB
005B90F3                       |>  83FE 0C               CMP     ESI, 0C
005B90F6                       |.  7E 03                 JLE     SHORT 005B90FB
005B90F8                           90                    NOP//  on annihile la suppression de 12 heures
005B90F9                           90                    NOP//
005B90FA                           90                    NOP//
005B90FB                       |>  55                    PUSH    EBP
005B90FC                       |.  E8 1F8EE4FF           CALL    00401F20
005B9101                       |.  83C4 04               ADD     ESP, 4
*/

	ReadProcessMemory(proc, (LPCVOID)addr_HeureInterface,(LPVOID) buffer,sizeof(OrigHeureInterface),&bytesWritten);
	if (memcmp(buffer,OrigHeureInterface, sizeof(OrigHeureInterface)) != 0)
	{
		_ERROR("Traduction de l'Heure : contrôle n°2 non ok... Rien ne sera modifié.");
		return false;
	}

/*
//HEURE Fr : MENU REPOS

ORIGINAL
005D72EB                       |.  F6C4 41               TEST    AH, 41
005D72EE                       |.  75 04                 JNZ     SHORT 005D72F4
005D72F0                       |.  D9C0                  FLD     ST
005D72F2                       |.  EB 0D                 JMP     SHORT 005D7301
005D72F4                       |>  D8D1                  FCOM    ST(1)
005D72F6                       |.  DFE0                  FSTSW   AX
005D72F8                       |.  D9C1                  FLD     ST(1)
005D72FA                       |.  F6C4 05               TEST    AH, 5
005D72FD                       |.  7A 02                 JPE     SHORT 005D7301
005D72FF                       |.  D8E1                  FSUB    ST, ST(1)
005D7301                       |>  E8 BAB53A00           CALL    009828C0

PATCHE
005D72EB                       |.  F6C4 41               TEST    AH, 41
005D72EE                       |.  75 04                 JNZ     SHORT 005D72F4
005D72F0                           D9C9                  FXCH    ST(1)// Exception: on passe ici pour Minuit  
005D72F2                       |.  EB 0D                 JMP     SHORT 005D7301
005D72F4                       |>  D8D1                  FCOM    ST(1)
005D72F6                       |.  DFE0                  FSTSW   AX
005D72F8                       |.  D9C1                  FLD     ST(1)
005D72FA                       |.  F6C4 05               TEST    AH, 5
005D72FD                       |.  7A 02                 JPE     SHORT 005D7301
005D72FF                           90                    NOP//   on annihile la suppression de 12 heures
005D7300                           90                    NOP//
005D7301                       |>  E8 BAB53A00           CALL    009828C0

*/
	ReadProcessMemory(proc, (LPCVOID)addr_HeureReposUn,(LPVOID) buffer,sizeof(OrigHeureReposUn),&bytesWritten);
	if (memcmp(buffer,OrigHeureReposUn, sizeof(OrigHeureReposUn)) != 0)
	{
		_ERROR("Traduction de l'Heure : contrôle n°3 non ok... Rien ne sera modifié.");
		return false;
	}

/*
//HEURE Fr : REPOS 2

ORIGINAL
005D6F84                        .  F6C4 41               TEST    AH, 41
005D6F87                        .  75 04                 JNZ     SHORT 005D6F8D
005D6F89                        .  D9C0                  FLD     ST
005D6F8B                        .  EB 0D                 JMP     SHORT 005D6F9A
005D6F8D                        >  D8D1                  FCOM    ST(1)
005D6F8F                        .  DFE0                  FSTSW   AX
005D6F91                        .  D9C1                  FLD     ST(1)
005D6F93                        .  F6C4 05               TEST    AH, 5
005D6F96                        .  7A 02                 JPE     SHORT 005D6F9A
005D6F98                        .  D8E1                  FSUB    ST, ST(1)
005D6F9A                        >  E8 21B93A00           CALL    009828C0

PATCHE
005D6F84                        .  F6C4 41               TEST    AH, 41
005D6F87                        .  75 04                 JNZ     SHORT 005D6F8D
005D6F89                           D9C9                  FXCH    ST(1)// Exception: on passe ici pour Minuit  
005D6F8B                        .  EB 0D                 JMP     SHORT 005D6F9A
005D6F8D                        >  D8D1                  FCOM    ST(1)
005D6F8F                        .  DFE0                  FSTSW   AX
005D6F91                        .  D9C1                  FLD     ST(1)
005D6F93                        .  F6C4 05               TEST    AH, 5
005D6F96                        .  7A 02                 JPE     SHORT 005D6F9A
005D6F98                           90                    NOP//	 on annihile la suppression de 12 heures
005D6F99                           90                    NOP//
005D6F9A                        >  E8 21B93A00           CALL    009828C0
*/
	ReadProcessMemory(proc, (LPCVOID)addr_HeureReposDeux,(LPVOID) buffer,sizeof(OrigHeureReposDeux),&bytesWritten);
	if (memcmp(buffer,OrigHeureReposDeux, sizeof(OrigHeureReposDeux)) != 0)
	{
		_ERROR("Traduction de l'Heure : contrôle n°4 non ok... Rien ne sera modifié.");
		return false;
	}

	//ICI TOUT ROULE YAPLUKA PATCHER

	_MESSAGE("Traduction de l'Heure : contrôles OK... PATCH...");
	result = WriteProcessMemory(proc,(LPVOID) addr_HeureAMPM,(LPVOID) HeureAMPM,sizeof(HeureAMPM),&bytesWritten);
	VirtualProtectEx(proc,(LPVOID)addr_HeureAMPM,2,oldprotect,&oldprotect) ;


	if (result)
	{
		_MESSAGE("....PATCH AMPM OK");
		result = WriteProcessMemory(proc,(LPVOID) addr_HeureInterface,(LPVOID) HeureInterface,sizeof(HeureInterface),&bytesWritten);
	}
	
	if (result)
	{
		_MESSAGE("....PATCH HEURE INTERFACE OK");
		result = WriteProcessMemory(proc,(LPVOID) addr_HeureReposUn,(LPVOID) HeureReposUn,sizeof(HeureReposUn),&bytesWritten);	
	}

	if (result)
	{
		_MESSAGE("....PATCH HEURE (MENU REPOS 1) OK");
		result = WriteProcessMemory(proc,(LPVOID) addr_HeureReposDeux,(LPVOID) HeureReposDeux,sizeof(HeureReposDeux),&bytesWritten);	
	}
	if (result)
		_MESSAGE("....PATCH HEURE (MENU REPOS 2) OK");


	return result;
}

bool PatchDateMenu(HANDLE proc)
{
	UInt32	bytesWritten;
	unsigned char buffer[60] ={0};
	bool result = false;
	bool Controle = false;
	DWORD oldprotect;

	_MESSAGE("Traduction de la date (MENU) : test de contrôle...");
	ReadProcessMemory(proc,(LPVOID) addr_DateMenu,(LPVOID) buffer,sizeof(OrigDateMenu),&bytesWritten);
	Controle = lstrcmp((char*)buffer,OrigDateMenu) == 0;
	if (Controle)
	{
		FillMemory(&buffer, 60,0);
		ReadProcessMemory(proc,(LPVOID) addr_PUsDateMenu,(LPVOID) buffer,sizeof(PUsDateMenu),&bytesWritten);
		if (memcmp(buffer,OrigPUsDateMenu, sizeof(OrigPUsDateMenu)) == 0) {Controle = true;}
	}

	if (Controle)
	{
		_MESSAGE("Traduction de la date (MENU) : test de contrôle OK :)");
		_MESSAGE("Traduction de la date (MENU) : PATCH..... !");

		if (VirtualProtectEx(proc,(LPVOID)addr_DateMenu,2,PAGE_EXECUTE_READWRITE,&oldprotect) != 0 )
		{
			result = WriteProcessMemory(proc,(LPVOID) addr_PUsDateMenu,(LPVOID) PUsDateMenu,sizeof(PUsDateMenu),&bytesWritten);
			if (result)
			{
				result &&  WriteProcessMemory(proc,(LPVOID) addr_DateMenu,(LPVOID) DateMenu,sizeof(DateMenu),&bytesWritten);
			}
			VirtualProtectEx(proc,(LPVOID)addr_DateMenu,2,oldprotect,&oldprotect) ;
		}
		
		if (result) 
		{_MESSAGE("Traduction de la date (MENU) : PATCH OK !");}
		else 
		{_MESSAGE("Traduction de la date (MENU) : impossible de patcher !"); }

		return result;
	}
	else 
	{
		_MESSAGE("Traduction de la date (MENU) : test de contrôle NON OK :(");
		return false;
	}
}


bool PatchOrdinaux(HANDLE proc)
{
	UInt32	bytesWritten;
	unsigned char buffer[20] ={0};
	bool result = false;
	DWORD oldprotect;

	_MESSAGE("Traduction des Ordinaux : test de contrôle...");
	ReadProcessMemory(proc,(LPVOID) addr_Ordinaux,(LPVOID) buffer,16,&bytesWritten);
	if (memcmp(buffer,OrigOrdinaux,16) == 0)
	{
		_MESSAGE("Traduction des Ordinaux : test de contrôle OK :)");
		_MESSAGE("Traduction des Ordinaux : PATCH..... !");

		if (VirtualProtectEx(proc,(LPVOID)addr_Ordinaux,16,PAGE_EXECUTE_READWRITE,&oldprotect) != 0 )
		{
			result = WriteProcessMemory(proc,(LPVOID) addr_Ordinaux,(LPVOID) Ordinaux,16,&bytesWritten);
			VirtualProtectEx(proc,(LPVOID)addr_Ordinaux,16,oldprotect,&oldprotect) ;
		}

		if (result) 
		{_MESSAGE("Traduction des Ordinaux : PATCH OK !");}
		else 
		{_MESSAGE("Traduction des Ordinaux : impossible de patcher !"); }
	
		
		return result;
	}
	else 
	{
		_MESSAGE("Traduction du format de date du journal : test de contrôle NON OK :(");
		return false;
	}
}

bool PatchDateJournalFormat(HANDLE proc)
{
	UInt32	bytesWritten;
	unsigned char buffer[20] ={0};
	bool result = false;
	DWORD oldprotect;

	_MESSAGE("Traduction du format de date du journal : test de contrôle...");
	ReadProcessMemory(proc,(LPVOID) addr_DateJournalFormat,(LPVOID) buffer,17,&bytesWritten);
	if (memcmp(buffer,OrigDateJournalFormat,17) == 0)
	{
		_MESSAGE("Traduction du format de date du journal : test de contrôle OK :)");
		_MESSAGE("Traduction du format de date du journal : PATCH..... !");

		if (VirtualProtectEx(proc,(LPVOID)addr_DateJournalFormat,9,PAGE_EXECUTE_READWRITE,&oldprotect) != 0 )
		{
			result = WriteProcessMemory(proc,(LPVOID) addr_DateJournalFormat,(LPVOID) DateJournalFormat,7,&bytesWritten);
			VirtualProtectEx(proc,(LPVOID)addr_DateJournalFormat,9,oldprotect,&oldprotect) ;
		}

		if (result) 
		{_MESSAGE("Traduction du format de date du journal : PATCH OK !");}
		else 
		{_MESSAGE("Traduction du format de date du journal : impossible de patcher !"); }
	
		
		return result;
	}
	else 
	{
		_MESSAGE("Traduction du format de date du journal : test de contrôle NON OK :(");
		return false;
	}
}

bool PatchAugmenteBY(HANDLE proc)
{
	UInt32	bytesWritten;
	unsigned char buffer[3] ={0};
	bool result = false;
	DWORD oldprotect;

	_MESSAGE("Traduction du 'by' : test de contrôle...");
	ReadProcessMemory(proc,(LPVOID) addr_AugmenteDE,(LPVOID) buffer,2,&bytesWritten);
	if (memcmp(buffer,OrigBy,2) == 0)
	{
		_MESSAGE("Traduction du 'by' : test de contrôle OK :)");
		_MESSAGE("Traduction du 'by' : PATCH..... !");

		if (VirtualProtectEx(proc,(LPVOID)addr_AugmenteDE,2,PAGE_EXECUTE_READWRITE,&oldprotect) != 0 )
		{
			result = WriteProcessMemory(proc,(LPVOID) addr_AugmenteDE,(LPVOID) De,2,&bytesWritten);
			VirtualProtectEx(proc,(LPVOID)addr_AugmenteDE,2,oldprotect,&oldprotect) ;
		}

		if (result) 
		{_MESSAGE("Traduction du 'by' : PATCH OK !");}
		else 
		{_MESSAGE("Traduction du 'by' : impossible de patcher !"); }
	
		
		return result;
	}
	else 
	{
		_MESSAGE("Traduction du 'by' : test de contrôle NON OK :(");
		return false;
	}
}



bool PatchInvocation(HANDLE proc)
{
	UInt32	bytesWritten;
	unsigned char buffer[60] ={0};
	bool result = false;
	bool Controle = false;
	_MESSAGE("Traduction des invocations : test de contrôle...");
	ReadProcessMemory(proc,(LPVOID) addr_INvocation,(LPVOID) buffer,sizeof(INvocation),&bytesWritten);
	Controle = memcmp(buffer,OriginalSummonCode,sizeof(OriginalSummonCode)) == 0;
	if (Controle)
	{
		FillMemory(&buffer, 60,0);
		ReadProcessMemory(proc,(LPVOID) addr_INvocationstring,(LPVOID) buffer,10,&bytesWritten);
		if (lstrcmp((char*)buffer,OriginalSummonString2) == 0) {Controle = true;}
	}

	if (Controle)
	{
		_MESSAGE("Traduction des invocations : test de contrôle OK :)");
		_MESSAGE("Traduction des invocations : PATCH..... !");
		result = WriteProcessMemory(proc,(LPVOID) addr_INvocation,(LPVOID) INvocation,sizeof(INvocation),&bytesWritten);
		result = result && WriteProcessMemory(proc,(LPVOID) addr_INvocationstring,(LPVOID) INvocationstring,sizeof(INvocationstring),&bytesWritten);
		
		if (result) 
		{_MESSAGE("Traduction des invocations : PATCH OK !");}
		else 
		{_MESSAGE("Traduction des invocations : impossible de patcher !"); }

		return result;
	}
	else 
	{
		_MESSAGE("Traduction des invocations : test de contrôle NON OK :(");
		return false;
	}
}


bool PatchDialogueEffetEnchantement(HANDLE proc)
{
	UInt32	bytesWritten;
	unsigned char buffer[65] ={0};
	bool result = false;
	DWORD oldprotect;

	_MESSAGE("Traduction de la boite de dialogue lors d'une redondance d'effet pendant l'enchantement : test de contrôle...");
	ReadProcessMemory(proc,(LPVOID) addr_EffetEnchantement,(LPVOID) buffer,62,&bytesWritten);
	if (memcmp(buffer,OrigEffetEnchantement,62) == 0)
	{
		_MESSAGE("Traduction de la boite de dialogue lors d'une redondance d'effet pendant l'enchantement : test de contrôle OK :)");
		_MESSAGE("Traduction de la boite de dialogue lors d'une redondance d'effet pendant l'enchantement : PATCH..... !");

		if (VirtualProtectEx(proc,(LPVOID)addr_EffetEnchantement,62,PAGE_EXECUTE_READWRITE,&oldprotect) != 0 )
		{
			result = WriteProcessMemory(proc,(LPVOID) addr_EffetEnchantement,(LPVOID) EffetEnchantement,62,&bytesWritten);
			VirtualProtectEx(proc,(LPVOID)addr_EffetEnchantement,62,oldprotect,&oldprotect) ;
		}

		if (result) 
		{_MESSAGE("Traduction de la boite de dialogue lors d'une redondance d'effet pendant l'enchantement : PATCH OK !");}
		else 
		{_MESSAGE("Traduction de la boite de dialogue lors d'une redondance d'effet pendant l'enchantement : impossible de patcher !"); }
	
		
		return result;
	}
	else 
	{
		_MESSAGE("Traduction de la boite de dialogue lors d'une redondance d'effet pendant l'enchantement : test de contrôle NON OK :(");
		return false;
	}
}





bool PatchProcess()
{
	
	_MESSAGE("Recherche du processus d'Oblivon...");
	bool	result = false;
	DWORD  oldprotect;
	
	HANDLE	process = OpenProcess(
		PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, GetTargetProcessIdFromProcname("Oblivion.exe"));
	if(process)
	{
		_MESSAGE("Processus Oblivion = [%08X]", process);
   
		if (VirtualProtectEx(process,(LPVOID)0x401000,0x12000,PAGE_EXECUTE_READWRITE,&oldprotect) != 0 )
		{
			result = PatchInvocation(process);
			result = result && PatchAugmenteBY(process);
			result = result && PatchDialogueEffetEnchantement(process);
			result = result && PatchDateJournalFormat(process);
			result = result && PatchOrdinaux(process);
			result = result && PatchDateMenu(process);
			result = result && PatchHeure(process);
			result = result && PatchDateEcranTitre(process);

			
			VirtualProtectEx(process,(LPVOID)0x401000,0x12000,oldprotect,&oldprotect);
		}
		else _ERROR("Impossible de déprotéger la mémoire :(");

		CloseHandle(process);
	}		else _ERROR("Impossible d'ouvrir Oblivion :(");

	return result;

}






#pragma region INUTILE



/*
bool Cmd_PluginTest_Execute(COMMAND_ARGS)
{
	_MESSAGE("plugintest");

	*result = 42;

	Console_Print("plugintest running");

	return true;
}

static CommandInfo kPluginTestCommand =
{
	"plugintest",
	"",
	0,
	"test command for obse plugin",
	0,		// requires parent obj
	0,		// doesn't have params
	NULL,	// no param table

	Cmd_PluginTest_Execute
};

std::string	g_strData;

//static void ResetData(void)
//{
//	g_strData.clear();
//}

static void ExamplePlugin_SaveCallback(void * reserved)
{
	// write out the string
	g_serialization->OpenRecord('STR ', 0);
	g_serialization->WriteRecordData(g_strData.c_str(), g_strData.length());

	// write out some other data
	g_serialization->WriteRecord('ASDF', 1234, "hello world", 11);
}

static void ExamplePlugin_LoadCallback(void * reserved)
{
	UInt32	type, version, length;

//	ResetData();

	char	buf[512];

	while(g_serialization->GetNextRecordInfo(&type, &version, &length))
	{
		_MESSAGE("record %08X (%.4s) %08X %08X", type, &type, version, length);

		switch(type)
		{
			case 'STR ':
				g_serialization->ReadRecordData(buf, length);
				buf[length] = 0;

				_MESSAGE("got string %s", buf);

				g_strData = buf;
				break;

			case 'ASDF':
				g_serialization->ReadRecordData(buf, length);
				buf[length] = 0;

				_MESSAGE("ASDF chunk = %s", buf);
				break;
		}
	}
}

static void ExamplePlugin_NewGameCallback(void * reserved)
{
//	ResetData();
}

bool Cmd_ExamplePlugin_SetString_Execute(COMMAND_ARGS)
{
	char	data[512];

	if(ExtractArgs(PASS_EXTRACT_ARGS, &data))
	{
		g_strData = data;
		Console_Print("Set string %s in script %08x", data, scriptObj->refID);
	}

	ExtractFormattedString(ScriptFormatStringArgs(0, 0, 0, 0), data);
	return true;
}

DEFINE_COMMAND_PLUGIN(ExamplePlugin_SetString, "sets a string", 0, 1, kParams_OneString)

bool Cmd_ExamplePlugin_PrintString_Execute(COMMAND_ARGS)
{
	Console_Print("PrintString: %s", g_strData.c_str());

	return true;
}

DEFINE_COMMAND_PLUGIN(ExamplePlugin_PrintString, "prints a string", 0, 0, NULL)


typedef OBSEArrayVarInterface::Array	OBSEArray;
typedef OBSEArrayVarInterface::Element	OBSEElement;

// helper function for creating an OBSE StringMap from a std::map<std::string, OBSEElement>
OBSEArray* StringMapFromStdMap(const std::map<std::string, OBSEElement>& data, Script* callingScript)
{
	// create empty string map
	OBSEArray* arr = g_arrayIntfc->CreateStringMap(NULL, NULL, 0, callingScript);

	// add each key-value pair
	for (std::map<std::string, OBSEElement>::const_iterator iter = data.begin(); iter != data.end(); ++iter) {
		g_arrayIntfc->SetElement(arr, iter->first.c_str(), iter->second);
	}

	return arr;
}

// helper function for creating an OBSE Map from a std::map<double, OBSEElement>
OBSEArray* MapFromStdMap(const std::map<double, OBSEElement>& data, Script* callingScript)
{
	OBSEArray* arr = g_arrayIntfc->CreateMap(NULL, NULL, 0, callingScript);
	for (std::map<double, OBSEElement>::const_iterator iter = data.begin(); iter != data.end(); ++iter) {
		g_arrayIntfc->SetElement(arr, iter->first, iter->second);
	}

	return arr;
}

// helper function for creating OBSE Array from std::vector<OBSEElement>
OBSEArray* ArrayFromStdVector(const std::vector<OBSEElement>& data, Script* callingScript)
{
	OBSEArray* arr = g_arrayIntfc->CreateArray(&data[0], data.size(), callingScript);
	return arr;
}

bool Cmd_ExamplePlugin_MakeArray_Execute(COMMAND_ARGS)
{
	// Create an array of the format
	// { 
	//	 0:"Zero"
	//	 1:1.0
	//	 2:PlayerRef
	//	 3:StringMap { "A":"a", "B":123.456, "C":"manually set" }
	//	 4:"Appended"
	//	}

	// create the inner StringMap array
	std::map<std::string, OBSEElement> stringMap;
	stringMap["A"] = "a";
	stringMap["B"] = 123.456;

	// create the outer array
	std::vector<OBSEElement> vec;
	vec.push_back("Zero");
	vec.push_back(1.0);
	vec.push_back(*g_thePlayer);
	
	// convert our map to an OBSE StringMap and store in outer array
	OBSEArray* stringMapArr = StringMapFromStdMap(stringMap, scriptObj);
	vec.push_back(stringMapArr);

	// manually set another element in stringmap
	g_arrayIntfc->SetElement(stringMapArr, "C", "manually set");

	// convert outer array
	OBSEArray* arr = ArrayFromStdVector(vec, scriptObj);

	// append another element to array
	g_arrayIntfc->AppendElement(arr, "appended");

	if (!arr)
		Console_Print("Couldn't create array");

	// return the array
	if (!g_arrayIntfc->AssignCommandResult(arr, result))
		Console_Print("Couldn't assign array to command result.");

	// result contains the new ArrayID; print it
	Console_Print("Returned array ID %.0f", *result);

	return true;
}

DEFINE_COMMAND_PLUGIN(ExamplePlugin_MakeArray, test, 0, 0, NULL);


void MessageHandler(OBSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case OBSEMessagingInterface::kMessage_ExitGame:
//		_MESSAGE("Plugin Example received ExitGame message");
		break;
	case OBSEMessagingInterface::kMessage_ExitToMainMenu:
//		_MESSAGE("Plugin Example received ExitToMainMenu message");
		break;
	case OBSEMessagingInterface::kMessage_PostLoad:
//		_MESSAGE("Plugin Example received PostLoad mesage");
		break;
	case OBSEMessagingInterface::kMessage_LoadGame:
	case OBSEMessagingInterface::kMessage_SaveGame:
//		_MESSAGE("Plugin Example received save/load message with file path %s", msg->data);
		break;
	case OBSEMessagingInterface::kMessage_Precompile: 
		{
//			ScriptBuffer* buffer = (ScriptBuffer*)msg->data;
			//_MESSAGE("Plugin Example received precompile message. Script Text:\n%s", buffer->scriptText);
			break;
		}
	case OBSEMessagingInterface::kMessage_PreLoadGame:
//		_MESSAGE("Plugin Example received pre-loadgame message with file path %s", msg->data);
		break;
	default:
//		_MESSAGE("Plugin Example received unknown message");
		break;
	}
}

*/
#pragma endregion 

extern "C" {

bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
{

	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Wiwiland_Oblivion_Camemberizator";
	info->version = 1;

	// version checks
	if(!obse->isEditor)
	{
		if(obse->obseVersion < OBSE_VERSION_INTEGER)
		{
			_ERROR("OBSE version too old (got %08X expected at least %08X)", obse->obseVersion, OBSE_VERSION_INTEGER);
			return false;
		}

		if(obse->oblivionVersion != OBLIVION_VERSION)
		{
			_ERROR("incorrect Oblivion version (got %08X need %08X)", obse->oblivionVersion, OBLIVION_VERSION);
			return false;
		}

		g_serialization = (OBSESerializationInterface *)obse->QueryInterface(kInterface_Serialization);
		if(!g_serialization)
		{
			_ERROR("serialization interface not found");
			return false;
		}

		if(g_serialization->version < OBSESerializationInterface::kVersion)
		{
			_ERROR("incorrect serialization version found (got %08X need %08X)", g_serialization->version, OBSESerializationInterface::kVersion);
			return false;
		}

		g_arrayIntfc = (OBSEArrayVarInterface*)obse->QueryInterface(kInterface_ArrayVar);
		if (!g_arrayIntfc)
		{
			_ERROR("Array interface not found");
			return false;
		}

		_MESSAGE("->Wiwiland Oblivion Camemberizator pour OBSE");
		_MESSAGE("-> Version 1.0");
		_MESSAGE("-> par la PNOO team le 30 Octobre 2010");
		_MESSAGE(" ");
		_MESSAGE("Credits : ");
		_MESSAGE(" * Recherche : Shadow-She-Wolf, MagikMike");
		_MESSAGE(" * Code : MagikMike ");
		_MESSAGE(" ");
		_MESSAGE("Remerciements : les auteurs d'obse (greet work), Shadow et ses coups de martifouet, les testeurs(euses) de ce truc, les membres de Wiwiland et la communauté des fans des TEs en général :) ");
		_MESSAGE(" ");		
		if (PatchProcess())
		{
			_MESSAGE("Patchs Terminés. Aucun problèmes.");
		}

	}
	else
	{
		// no version checks needed for editor
	}

	// version checks pass

	return true;
}

bool OBSEPlugin_Load(const OBSEInterface * obse)
{
	//_MESSAGE("Chargement......");

	g_pluginHandle = obse->GetPluginHandle();

	/***************************************************************************
	 *	
	 *	READ THIS!
	 *	
	 *	Before releasing your plugin, you need to request an opcode range from
	 *	the OBSE team and set it in your first SetOpcodeBase call. If you do not
	 *	do this, your plugin will create major compatibility issues with other
	 *	plugins, and may not load in future versions of OBSE. See
	 *	obse_readme.txt for more information.
	 *	
	 **************************************************************************/

	// register commands
//	obse->SetOpcodeBase(0x2000);
//	obse->RegisterCommand(&kPluginTestCommand);

//	obse->RegisterCommand(&kCommandInfo_ExamplePlugin_SetString);
	//obse->RegisterCommand(&kCommandInfo_ExamplePlugin_PrintString);

	// commands returning array must specify return type; type is optional for other commands
//	obse->RegisterTypedCommand(&kCommandInfo_ExamplePlugin_MakeArray, kRetnType_Array);

	// set up serialization callbacks when running in the runtime
/*	if(!obse->isEditor)
	{
		// NOTE: SERIALIZATION DOES NOT WORK USING THE DEFAULT OPCODE BASE IN RELEASE BUILDS OF OBSE
		// it works in debug builds
		g_serialization->SetSaveCallback(g_pluginHandle, ExamplePlugin_SaveCallback);
		g_serialization->SetLoadCallback(g_pluginHandle, ExamplePlugin_LoadCallback);
		g_serialization->SetNewGameCallback(g_pluginHandle, ExamplePlugin_NewGameCallback);

		// register to use string var interface
		// this allows plugin commands to support '%z' format specifier in format string arguments
		OBSEStringVarInterface* g_Str = (OBSEStringVarInterface*)obse->QueryInterface(kInterface_StringVar);
		g_Str->Register(g_Str);
	}

	// register to receive messages from OBSE
	OBSEMessagingInterface* msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
	msgIntfc->RegisterListener(g_pluginHandle, "OBSE", MessageHandler);*/

	return true;
}

};
