#include "PDBInfo.h"
#include <Shlwapi.h>

#include "KernelModules.h"


bool PDBInfo::DownLoadNtos()
{
	KernelModules _KernelModules;
	_KernelModules.GetKernelModuleListR3();
	for (auto x : _KernelModules._KernelModuleListR3)
	{
		if (std::string((char*)x.Name) == "ntoskrnl.exe")
		{
			_BaseAddr = x.Addr;
			break;
		}
	}
	

	std::string symbolpath = std::getenv("_NT_SYMBOL_PATH");
	if (symbolpath.find("SRV") != std::string::npos)
	{
		std::vector<std::string> temp_vector = Split(symbolpath, "*");
		for (auto x : temp_vector)
		{
			if (PathFileExistsA(x.c_str()))
			{
				symbolpath = x;
				break;
			}
		}
	}
	std::string ntos_pdb_path = symbolpath + "\\" + GetPdbPath(std::string(std::getenv("systemroot")) + "\\System32\\ntoskrnl.exe");
	//ntos_pdb_path = Replace_ALL(ntos_pdb_path, "/", "\\");
	if (PathFileExistsA(ntos_pdb_path.c_str()))
	{
		if (EzPdbLoad(ntos_pdb_path, &_Pdb))
		{
			return true;
		}
	}
	else
	{
		if (symbolpath.length() > 0)
		{
			std::string temp_path = EzPdbDownload2(std::string(std::getenv("systemroot")) + "\\System32\\ntoskrnl.exe", symbolpath);
			if (EzPdbLoad(ntos_pdb_path, &_Pdb))
			{
				return true;
			}
		}
		else
		{
			std::string temp_path = EzPdbDownload(std::string(std::getenv("systemroot")) + "\\System32\\ntoskrnl.exe");
			if (EzPdbLoad(ntos_pdb_path, &_Pdb))
			{
				return true;
			}
		}
	}
	return false;
}


struct MyStruct
{
	ULONG64 _BaseAddr;
	std::vector<NTOSSYMBOL>* temp_vector;
};

BOOL PsymEnumeratesymbolsCallback(
	PSYMBOL_INFO pSymInfo,
	ULONG SymbolSize,
	PVOID UserContext
)
{
	MyStruct* _MyStruct = (MyStruct*)UserContext;
	NTOSSYMBOL temp_list;
	RtlZeroMemory(&temp_list, sizeof(NTOSSYMBOL));
	temp_list.Addr = pSymInfo->Address - pSymInfo->ModBase + _MyStruct->_BaseAddr;
	temp_list.Name = pSymInfo->Name;
	temp_list.RVA = pSymInfo->Address - pSymInfo->ModBase;
	_MyStruct->temp_vector->push_back(temp_list);
	return TRUE;
}

bool PDBInfo::GetALL()
{
	MyStruct _MyStruct;
	_MyStruct._BaseAddr = _BaseAddr;
	_MyStruct.temp_vector = &_Symbol;
	return SymEnumSymbols(GetCurrentProcess(), EZ_PDB_BASE_OF_DLL, "*!*", &PsymEnumeratesymbolsCallback, &_MyStruct);
}

std::vector<SYMBOLSTRUCT> PDBInfo::PdbGetStruct(IN PEZPDB Pdb, IN std::string StructName)
{
	std::vector<SYMBOLSTRUCT> temp_vector;

	ULONG SymInfoSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR);
	SYMBOL_INFO* SymInfo = (SYMBOL_INFO*)malloc(SymInfoSize);
	if (!SymInfo)
	{
		return  temp_vector;
	}
	ZeroMemory(SymInfo, SymInfoSize);
	SymInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
	SymInfo->MaxNameLen = MAX_SYM_NAME;
	if (!SymGetTypeFromName(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, StructName.c_str(), SymInfo))
	{
		return  temp_vector;
	}

	TI_FINDCHILDREN_PARAMS TempFp = { 0 };
	if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, SymInfo->TypeIndex, TI_GET_CHILDRENCOUNT, &TempFp))
	{
		free(SymInfo);
		return  temp_vector;
	}

	ULONG ChildParamsSize = sizeof(TI_FINDCHILDREN_PARAMS) + TempFp.Count * sizeof(ULONG);
	TI_FINDCHILDREN_PARAMS* ChildParams = (TI_FINDCHILDREN_PARAMS*)malloc(ChildParamsSize);
	if (ChildParams == NULL)
	{
		free(SymInfo);
		return  temp_vector;
	}
	ZeroMemory(ChildParams, ChildParamsSize);
	memcpy(ChildParams, &TempFp, sizeof(TI_FINDCHILDREN_PARAMS));
	if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, SymInfo->TypeIndex, TI_FINDCHILDREN, ChildParams))
	{
		goto failed;
	}
	for (ULONG i = ChildParams->Start; i < ChildParams->Count; i++)
	{
		WCHAR* pSymName = NULL;
		ULONG Offset = 0;
		if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, ChildParams->ChildId[i], TI_GET_OFFSET, &Offset))
		{
			goto failed;
		}
		if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, ChildParams->ChildId[i], TI_GET_SYMNAME, &pSymName))
		{
			goto failed;
		}
		if (pSymName)
		{
			SYMBOLSTRUCT temp_list;
			RtlZeroMemory(&temp_list, sizeof(SYMBOLSTRUCT));
			temp_list.Name = W_TO_C(pSymName);
			temp_list.Offset = Offset;
			LocalFree(pSymName);
			temp_vector.push_back(temp_list);
		}
	}
failed:
	free(ChildParams);
	free(SymInfo);
	return  temp_vector;
}


void PDBInfo::UnLoad()
{
	EzPdbUnload(&_Pdb);
}