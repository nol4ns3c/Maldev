#include <Windows.h>
#include <winternl.h>
#include <stdio.h>

#define NEW_STREAM L":Teze"
extern PPEB GetPEB(void);
extern BYTE CheckDebugger(void);

BOOL DeleteSelf() {
    WCHAR szPath[MAX_PATH * 2] = { 0 };
    FILE_DISPOSITION_INFO Delete = { 0 };
    HANDLE hFile = INVALID_HANDLE_VALUE;
    PFILE_RENAME_INFO pRename = NULL;
    const wchar_t* NewStream = (const wchar_t*)NEW_STREAM;
    SIZE_T StreamLength = wcslen(NewStream) * sizeof(wchar_t);
    SIZE_T sRename = sizeof(FILE_RENAME_INFO) + StreamLength;

    // Allocating enough buffer for the 'FILE_RENAME_INFO' structure
    pRename = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sRename);
    if (!pRename) {
        printf("[!] HeapAlloc Failed With Error: %d \n", GetLastError());
        return FALSE;
    }

    ZeroMemory(szPath, sizeof(szPath));
    ZeroMemory(&Delete, sizeof(FILE_DISPOSITION_INFO));

    Delete.DeleteFile = TRUE;
    pRename->FileNameLength = StreamLength;
    RtlCopyMemory(pRename->FileName, NewStream, StreamLength);

    if (GetModuleFileNameW(NULL, szPath, MAX_PATH * 2) == 0) {
        printf("[!] GetModuleFileNameW Failed With Error: %d \n", GetLastError());
        return FALSE;
    }

    hFile = CreateFileW(szPath, DELETE | SYNCHRONIZE, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("[!] CreateFileW [R] Failed With Error: %d \n", GetLastError());
        return FALSE;
    }

    wprintf(L"[i] Renaming :$DATA to %s  ...", NEW_STREAM);

    if (!SetFileInformationByHandle(hFile, FileRenameInfo, pRename, sRename)) {
        printf("[!] SetFileInformationByHandle [R] Failed With Error: %d \n", GetLastError());
        return FALSE;
    }
    wprintf(L"[+] DONE \n");

    CloseHandle(hFile);

    hFile = CreateFileW(szPath, DELETE | SYNCHRONIZE, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("[!] CreateFileW [D] Failed With Error: %d \n", GetLastError());
        return FALSE;
    }

    wprintf(L"[i] DELETING ...");

    if (!SetFileInformationByHandle(hFile, FileDispositionInfo, &Delete, sizeof(Delete))) {
        printf("[!] SetFileInformationByHandle [D] Failed With Error: %d \n", GetLastError());
        return FALSE;
    }
    wprintf(L"[+] DONE \n");

    CloseHandle(hFile);
    HeapFree(GetProcessHeap(), 0, pRename);

    return TRUE;
}

int main(void) {
    printf("Getting PEB...\n");
    PPEB pPEB = GetPEB();

    if (CheckDebugger() != 0) {
        printf("Debugger detected\n");
        DeleteSelf();
        return 1; 
    }

    printf("There is no debugger\n");
    return 0; 
}

