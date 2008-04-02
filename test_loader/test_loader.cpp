#include <tchar.h>
#include <windows.h>
#include <stdio.h>

typedef int (__stdcall *STDCALL_TEST_PROC)(char* name);
typedef void (__cdecl *CDECL_TEST_PROC)(char* name);

#define TRACE(msg)	printf("TEST-LOADER: "); printf msg

int main(int argc, const char *argv[])
{
	HMODULE *hLib = new HMODULE[argc-1];

	for (int i = 1; i < argc; i++) {
		hLib[i] = LoadLibraryA(argv[i]);
	}

	for (int i = 1; i < argc; i++) {
		if (hLib[i] == 0) {
			TRACE(("error opening module: %s\n", argv[i]));
			continue;
		} else {
			TRACE(("loaded at %08X: %s\n", hLib[i], argv[i]));
		}

		TRACE(("calling: %s\n", argv[i]));

		STDCALL_TEST_PROC testStdcall = (STDCALL_TEST_PROC) GetProcAddress(hLib[i], "TestHello");
		if (testStdcall) {
			int x = testStdcall("Sir");
			TRACE(("result = %d\n", x));
		} else {
			TRACE(("no such proc: TestHello\n"));
		}

		CDECL_TEST_PROC testCdecl = (CDECL_TEST_PROC) GetProcAddress(hLib[i], "TestGoodbye");
		if (testCdecl) {
			testCdecl("Artisan");
		} else {
			TRACE(("no such proc: TestGoodbye\n"));
		}

		// one of these breaks all other DLLs (because of Py_Finalize() at DLL_PROCESS_DETACH)
		//FreeLibrary(hLib[i]); 
	}

	return 0;
}