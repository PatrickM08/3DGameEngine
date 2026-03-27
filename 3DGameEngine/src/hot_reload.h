#pragma once
#include <windows.h>
#include <cstdio>

struct ECS;
struct CameraComponent;

typedef void (*GameUpdateFn)(ECS&, CameraComponent&);

struct GameDLL {
    HMODULE handle = nullptr;
    GameUpdateFn update = nullptr;
    FILETIME lastWriteTime = {};
};

inline FILETIME getFileWriteTime(const char* path) {
    FILETIME ft = {};
    WIN32_FIND_DATAA findData;
    HANDLE handle = FindFirstFileA(path, &findData);
    if (handle != INVALID_HANDLE_VALUE) {
        ft = findData.ftLastWriteTime;
        FindClose(handle);
    }
    return ft;
}

inline bool loadGameDLL(GameDLL& dll, const char* sourcePath, const char* tempPath) {
    if (dll.handle) {
        FreeLibrary(dll.handle);
        dll.handle = nullptr;
        dll.update = nullptr;
    }

    BOOL copyResult = CopyFileA(sourcePath, tempPath, FALSE);
    printf("CopyFile: %d, error: %lu\n", copyResult, GetLastError());

    dll.handle = LoadLibraryA(tempPath);
    printf("LoadLibrary: %p, error: %lu\n", dll.handle, GetLastError());

    if (dll.handle) {
        dll.update = (GameUpdateFn)GetProcAddress(dll.handle, "game_update");
        printf("GetProcAddress: %p\n", dll.update);
    }
    dll.lastWriteTime = getFileWriteTime(sourcePath);
    return dll.update != nullptr;
}

inline bool checkForReload(GameDLL& dll, const char* sourcePath, const char* tempPath) {
    FILETIME current = getFileWriteTime(sourcePath);
    if (current.dwLowDateTime == 0 && current.dwHighDateTime == 0)
        return false;
    if (CompareFileTime(&current, &dll.lastWriteTime) != 0) {
        return loadGameDLL(dll, sourcePath, tempPath);
    }
    return false;
}
