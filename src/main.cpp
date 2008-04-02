#include <windows.h>
#include <shlwapi.h>
#include "epylib.h"

#pragma comment(linker, "/merge:.rdata=.text")
#pragma comment(linker, "/section:.edata,rw")

#define TRACE(msg)	printf("EPY: "); printf msg

epy::Interpreter inter;
PBYTE thisModule;

// these must be deleted at unload time
DWORD *addressRVA;
DWORD *nameRVA;
WORD *ordRVA;
epy::String *names;

bool Startup();
bool Startup2();
void Shutdown();

BOOL WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hModule);
			thisModule = (PBYTE) hModule;
			return Startup2();
		case DLL_PROCESS_DETACH:
			Shutdown();
	}
	return TRUE;
}

/*
bool Reroute(const char *apiName, void *newAddr)
{
	DWORD *names = (DWORD *) (thisModule + expDir->AddressOfNames);
	DWORD *addrs = (DWORD *) (thisModule + expDir->AddressOfFunctions);
	bool found = false;
	for (DWORD i = 0; i < expDir->NumberOfNames; i++) {
		if (_stricmp((char*) thisModule + names[i], apiName) == 0) {
			PBYTE addr = thisModule + addrs[i];
			printf("[%s] = %p, redirect to %p, rel %08X\n", thisModule + names[i], addr, newAddr, (PBYTE) newAddr - addr);
			*addr = 0xE9;
			*(DWORD *) (addr+1) = (DWORD) ((PBYTE) newAddr - addr - 5);
			found = true;
			break;
		}
	}

	if (!found) {
		//expDir->NumberOfNames += 1;
		//names[expDir->NumberOfNames-1] = (PBYTE) _strdup(apiName) - thisModule;
		//addrs[expDir->NumberOfNames-1] = (PBYTE) newAddr - thisModule;
	}

	return true;
}
*/

//epy::String *names;

#define RVA(addr, base)	((DWORD) ((PBYTE) addr - (PBYTE) base))

bool Startup2()
{
	using namespace epy;
	char szFileName[MAX_PATH];
	char *scriptName;

	// extract file name from the DLL path in order to use as script name
	GetModuleFileNameA((HMODULE) thisModule, szFileName, sizeof(szFileName));
	PathRemoveExtensionA(scriptName = PathFindFileNameA(szFileName));
	TRACE(("importing: %s ...\n", scriptName));

	// also extract the DLL location to add it to sys.path
	PathRemoveFileSpecA(szFileName);
	TRACE(("script dir: %s\n", szFileName));

	if (!inter.Startup()) {
		return false;
	}

	// quick-n-dirty: add the DLL dir to sys.path so "import <module>" does not fail
	char cmd[1024];
	PyOS_snprintf(cmd, 1024, "import sys\nif r'%s' not in sys.path: sys.path.append(r'%s')\n", szFileName, szFileName);
	inter.RunString(cmd);

	// safe to import the module now
	Module mod(inter.Import(scriptName));

	if (mod == 0) {
		PyRun_SimpleString("import __builtin__"); // easy way to print stack trace :P
		if (Error::Check()) {
			Object exType, exValue, exTrace;
			Error::Fetch(exType, exValue, exTrace);
			TRACE(("EPY: error: %s: %s\n", String(exType.Str()).AsString(), String(exValue.Str()).AsString()));
			Error::Clear();
		}
		return false;
	}

	TRACE(("import ok!\n"));

	Sequence exportList(mod.GetAttr("EPY_EXPORT_LIST"));
	DWORD nFuncs = exportList.Size();

	addressRVA = new DWORD[nFuncs];
	nameRVA = new DWORD[nFuncs];
	ordRVA = new WORD[nFuncs];
	names = new String[nFuncs];

	for (int i = 0; i < exportList.Size(); i++) {
		Object func(exportList.Get(i));
		names[i].Keep(func.GetAttr("_cfuncname"));
		PyObject *address = func.GetAttr("_cfuncptr");
		const char *n = names[i].AsString();
		void *a = PyLong_AsVoidPtr(address);

		ordRVA[i] = i;
		addressRVA[i] = RVA(a, thisModule);
		nameRVA[i] = RVA(n, thisModule);
		TRACE(("n = %s, a = %p\n", n, a));
		//Reroute(n, a);
	}

	// patch PE export directory

	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER) thisModule;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS) (thisModule+ pDos->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY expDir = (PIMAGE_EXPORT_DIRECTORY) (thisModule + pNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	expDir->NumberOfNames = expDir->NumberOfFunctions = nFuncs;
	expDir->AddressOfFunctions = RVA(addressRVA, thisModule);
	expDir->AddressOfNameOrdinals = RVA(ordRVA, thisModule);
	expDir->AddressOfNames = RVA(nameRVA, thisModule);

	return true;
}

/*
bool Startup()
{
	using namespace epy;
	char szFileName[MAX_PATH];
	char *scriptName;

	GetModuleFileNameA((HMODULE) thisModule, szFileName, sizeof(szFileName));
	PathRemoveExtensionA(scriptName = PathFindFileNameA(szFileName));
	printf("EPY: running: %s\n", scriptName);
	


	if (!inter.Startup()) {
		return false;
	}

	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER) thisModule;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS) (thisModule+ pDos->e_lfanew);
	expDir = (PIMAGE_EXPORT_DIRECTORY) (thisModule + pNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	Module mod = inter.Import(scriptName);

	if (mod == 0) {
		PyRun_SimpleString("import __builtin__");
		if (PyErr_Occurred()) {
			PyObject *t, *v, *tr;
			PyErr_Fetch(&t, &v, &tr);
			
			Object ot = PyObject_Str(t);
			Object ov = PyObject_Str(v);
			printf("EPY: error: %s: %s\n", PyString_AsString(ot), PyString_AsString(ov));
			PyErr_Clear();
		}
		return true;
	}

	pyplug = new Module(inter.Import("epy"));

	Sequence exportList = pyplug->GetAttr("exportList");
	for (int i = 0; i < exportList.Size(); i++) {
		Object func = exportList.Get(i);
		Object name = func.GetAttr("_cfuncname");
		Object address = func.GetAttr("_cfuncptr");
		char *n = PyString_AsString(name);
		void *a = PyLong_AsVoidPtr(address);
		//printf("n = %s, a = %p\n", n, a);
		Reroute(n, a);
	}

	return true;
}
*/
void Shutdown()
{
	delete[] names;
	delete[] ordRVA;
	delete[] nameRVA;
	delete[] addressRVA;
	inter.Shutdown();
}

extern "C" void __declspec(dllexport) __declspec(naked) dummy()
{
}
