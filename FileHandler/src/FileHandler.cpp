#include "FileHandler.h"

CFileHandler::~CFileHandler()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
        Close();
}

CFileHandler::CFileHandler() : 
    m_hFile(INVALID_HANDLE_VALUE)
{
    m_filename[0] = _T('\0');
}

BOOL CFileHandler::Open(const TCHAR* filename, DWORD dwDesiredAccess, DWORD dwCreationDisposition, DWORD dwShareMode, BOOL bAppendMode)
{
    if (m_hFile != INVALID_HANDLE_VALUE)
        Close();

    _tcscpy_s(m_filename, MAX_PATH, filename);

    // Append Modu Etkinse Ayarla
    if (bAppendMode)
    {
        dwDesiredAccess = GENERIC_WRITE;
        dwCreationDisposition = OPEN_ALWAYS;
    }

    m_hFile = CreateFile(
        filename,
        dwDesiredAccess,
        dwShareMode,
        NULL,
        dwCreationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    // Append Modu Etkinse Ayarla
    if (bAppendMode && m_hFile != INVALID_HANDLE_VALUE)
    {
        SetFilePointer(m_hFile, 0, NULL, FILE_END);
    }

    return (m_hFile != INVALID_HANDLE_VALUE);
}

BOOL CFileHandler::Close()
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return TRUE;

    BOOL result = CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;
    return result;
}

DWORD CFileHandler::GetFileSize()
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return 0;

    return ::GetFileSize(m_hFile, NULL);
}

DWORD CFileHandler::GetFileSize(const TCHAR* filename)
{
    CFileHandler temp;
    if (!temp.Open(filename, GENERIC_READ, OPEN_EXISTING))
        return 0;

    DWORD size = temp.GetFileSize();
    temp.Close();
    return size;
}

DWORD CFileHandler::GetPosition()
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return 0;

    return SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
}

BOOL CFileHandler::SetPosition(DWORD position, DWORD moveMethod)
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    DWORD newPosition = SetFilePointer(m_hFile, position, NULL, moveMethod);
    if (newPosition == INVALID_SET_FILE_POINTER) {
        DWORD error = GetLastError();
        if (error != NO_ERROR) {
            _tprintf(_T("Dosya konumlandýrma hatasý: %d\n"), error);
            return FALSE;
        }
    }
    return TRUE;
}

BOOL CFileHandler::ReadWithProgress(OnProgressCallback callback, DWORD bufferSize)
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    DWORD fileSize = GetFileSize();
    DWORD totalRead = 0;
    CHAR* buffer = new CHAR[bufferSize];
    DWORD bytesRead;

    SetPosition(0, FILE_BEGIN);

    while (::ReadFile(m_hFile, buffer, bufferSize - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';

        // Burada okunan veri iþlenebilir
        // Örneðin: printf("%s", buffer);

        totalRead += bytesRead;

        if (callback != NULL) {
            callback(totalRead, fileSize);
        }
    }

    delete[] buffer;
    return TRUE;
}

BOOL CFileHandler::WriteWithProgress(const CHAR* data, DWORD dataSize, OnProgressCallback callback)
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    //if (m_bEnableAppendMode)
    {
        SetFilePointer(m_hFile, 0, NULL, FILE_END);
    }

    DWORD totalWritten = 0;
    const DWORD chunkSize = 1024;
    BOOL result = TRUE;

    while (totalWritten < dataSize && result) {
        DWORD toWrite = (dataSize - totalWritten) > chunkSize ? chunkSize : (dataSize - totalWritten);
        DWORD bytesWritten;

        result = ::WriteFile(
            m_hFile,
            data + totalWritten,
            toWrite,
            &bytesWritten,
            NULL);

        if (!result || bytesWritten != toWrite) {
            _tprintf(_T("Yazma hatasý! Hata: %d\n"), GetLastError());
            break;
        }

        totalWritten += bytesWritten;

        if (callback != NULL) {
            callback(totalWritten, dataSize);
        }

        //if (m_bEnableAppendMode)
        {
            SetFilePointer(m_hFile, 0, NULL, FILE_END);
        }
    }

    return result;
}

BOOL CFileHandler::Append(const CHAR* data, DWORD dataSize, OnProgressCallback callback)
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    SetPosition(0, FILE_END);
    return WriteWithProgress(data, dataSize, callback);
}

// Statik fonksiyonlar
BOOL CFileHandler::ReadFileWithProgress(const TCHAR* filename, OnProgressCallback callback)
{
    CFileHandler file;
    if (!file.Open(filename, GENERIC_READ, OPEN_EXISTING))
        return FALSE;

    BOOL result = file.ReadWithProgress(callback);
    file.Close();
    return result;
}

BOOL CFileHandler::WriteFileWithProgress(const TCHAR* filename, const CHAR* data, DWORD dataSize,
    OnProgressCallback callback, DWORD position, DWORD moveMethod)
{
    CFileHandler file;
    if (!file.Open(filename, GENERIC_WRITE, OPEN_ALWAYS))
        return FALSE;

    if (position != 0xFFFFFFFF) {
        if (!file.SetPosition(position, moveMethod)) {
            file.Close();
            return FALSE;
        }
    }

    BOOL result = file.WriteWithProgress(data, dataSize, callback);
    file.Close();
    return result;
}

BOOL CFileHandler::AppendToFile(const TCHAR* filename, const CHAR* data, OnProgressCallback callback)
{
    return WriteFileWithProgress(filename, data, strlen(data), callback, 0xFFFFFFFF, FILE_END);
}

BOOL CFileHandler::Read(LPVOID buffer, DWORD bytesToRead, LPDWORD bytesRead)
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    return ::ReadFile(m_hFile, buffer, bytesToRead, bytesRead, NULL);
}

BOOL CFileHandler::Write(LPVOID buffer, DWORD bytesToWrite, LPDWORD bytesWritten)
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    return ::WriteFile(m_hFile, buffer, bytesToWrite, bytesWritten, NULL);
}
