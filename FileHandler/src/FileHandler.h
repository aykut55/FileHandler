#ifndef FileHandlerH
#define FileHandlerH

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

// Progress callback fonksiyon tipi
typedef void (*OnProgressCallback)(DWORD currentByte, DWORD totalBytes);

class CFileHandler
{
public:
    virtual ~CFileHandler();
             CFileHandler();

    // Dosya açma/kapama
    BOOL Open(const TCHAR* filename, DWORD dwDesiredAccess, DWORD dwCreationDisposition, DWORD dwShareMode = 0, BOOL bAppendMode = FALSE);
    BOOL Close();

    // Dosya boyutu iþlemleri
    DWORD GetFileSize();
    static DWORD GetFileSize(const TCHAR* filename);

    // Dosya konum iþaretçisi iþlemleri
    DWORD GetPosition();
    BOOL SetPosition(DWORD position, DWORD moveMethod = FILE_BEGIN);

    // Okuma/yazma iþlemleri
    BOOL ReadWithProgress(OnProgressCallback callback = NULL, DWORD bufferSize = 1024);
    BOOL WriteWithProgress(const CHAR* data, DWORD dataSize, OnProgressCallback callback = NULL);
    BOOL Append(const CHAR* data, DWORD dataSize, OnProgressCallback callback = NULL);

    // Statik yardýmcý fonksiyonlar
    static BOOL ReadFileWithProgress(const TCHAR* filename, OnProgressCallback callback = NULL);
    static BOOL WriteFileWithProgress(const TCHAR* filename, const CHAR* data, DWORD dataSize,
        OnProgressCallback callback = NULL, DWORD position = 0xFFFFFFFF,
        DWORD moveMethod = FILE_BEGIN);
    static BOOL AppendToFile(const TCHAR* filename, const CHAR* data, OnProgressCallback callback = NULL);

    BOOL Read(LPVOID buffer, DWORD bytesToRead, LPDWORD bytesRead);
    BOOL Write(LPVOID buffer, DWORD bytesToWrite, LPDWORD bytesWritten);

protected:

private:
    HANDLE m_hFile;
    TCHAR m_filename[MAX_PATH];
};

#endif // FileHandlerH
