#include "src/FileHandler.h"
#include <tchar.h>
#include <conio.h>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <iostream>

int main1()
{
    std::cout << std::endl;
    std::cout << std::endl;
    
    system("dir ");
    
    std::cout << std::endl;
    std::cout << std::endl;

    const TCHAR* filename = _T("sharedfile.txt");

    // 1. YAZICI UYGULAMA (Paylaþým modu açýk)
    CFileHandler writer;
    if (writer.Open(filename, GENERIC_WRITE, OPEN_ALWAYS, FILE_SHARE_READ | FILE_SHARE_WRITE))
    {
        _tprintf(_T("Yazýcý: Dosya paylaþýmlý modda açýldý\n"));

        for (int i = 1; i <= 5; i++)
        {
            CHAR buffer[256];
            sprintf_s(buffer, "Mesaj %d - Zaman: %d\n", i, GetTickCount());

            writer.Append(buffer, strlen(buffer));
            _tprintf(_T("Yazýcý: %s"), buffer);
            Sleep(1000); // 1 saniye bekle
        }

        writer.Close();
    }
    else
    {
        _tprintf(_T("Yazýcý: Dosya açýlamadý! Hata: %d\n"), GetLastError());
    }

    // 2. OKUYUCU UYGULAMA (Paylaþým modu açýk)
    CFileHandler reader;
    if (reader.Open(filename, GENERIC_READ, OPEN_EXISTING, FILE_SHARE_READ | FILE_SHARE_WRITE))
    {
        _tprintf(_T("\nOkuyucu: Dosya paylaþýmlý modda açýldý\n"));

        CHAR buffer[1024];
        DWORD bytesRead;

        while (reader.ReadWithProgress())
        {
            reader.SetPosition(0, FILE_BEGIN); // Her seferinde baþtan oku
            Sleep(500); // 0.5 saniye bekle
        }

        reader.Close();
    }
    else
    {
        _tprintf(_T("Okuyucu: Dosya açýlamadý! Hata: %d\n"), GetLastError());
    }

    system("pause");

    return 0;
}


// Progress callback fonksiyonlarý
void ShowProgress(DWORD current, DWORD total)
{
    float percent = (float)current / total * 100.0f;
    _tprintf(_T("Isleniyor: %%%.2f (%d/%d byte)\n"), percent, current, total);
}
void ReadProgressCallback(DWORD current, DWORD total)
{
    float percent = (float)current / total * 100.0f;
    _tprintf(_T("Okuma ilerleme: %%%.2f (%d/%d byte)\n"), percent, current, total);
}
void WriteProgressCallback(DWORD current, DWORD total)
{
    float percent = (float)current / total * 100.0f;    
    _tprintf(_T("Yazma ilerleme: %%%.2f (%d/%d byte)\n"), percent, current, total);
}


int runBinaryFileCopyExample()
{
    const TCHAR* srcFileName = _T("../input_binary.rar");
    const TCHAR* dstFilename = _T("../output_binary.rar");

    _tprintf(_T("Dosya kopyalama islemi baslatiliyor...\n\n"));
    _tprintf(_T("Kaynak : %s -> Hedef : %s\n\n"), srcFileName, dstFilename);

    CFileHandler reader;
    BOOL successRd = reader.Open(srcFileName, GENERIC_READ, OPEN_EXISTING, FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (!successRd) {
        _tprintf(_T("Kaynak dosya acilamadi! Hata: %d\n"), GetLastError());
        return 1;
    }

    CFileHandler writer;
    BOOL successWr = writer.Open(dstFilename, GENERIC_WRITE, OPEN_ALWAYS, 0, FALSE);
    if (!successWr) {
        _tprintf(_T("Hedef dosya acilamadi! Hata: %d\n"), GetLastError());
        reader.Close();
        return 1;
    }

    const DWORD bufferSize = 4096;
    CHAR buffer[bufferSize];
    DWORD bytesRead, bytesWritten;
    DWORD totalBytes = reader.GetFileSize();
    DWORD totalCopied = 0;
    DWORD totalBytesShowProgress = 0;
    DWORD loopCounter = 0;

    _tprintf(_T("Toplam kopyalanacak boyut : %d byte\n\n"), totalBytes);

    while (true)
    {
        bool statusRead = reader.Read(buffer, bufferSize, &bytesRead);
        if (!statusRead || bytesRead == 0)
        {
            break;
        }

        bool statusWrite = writer.Write(buffer, bytesRead, &bytesWritten);
        if (!statusWrite || bytesWritten != bytesRead)
        {
            _tprintf(_T("Yazma hatasi! Hata: %d\n"), GetLastError());
            break;
        }

        totalCopied += bytesRead;
        loopCounter++;
        if (loopCounter % 100 == 0)
        {
            totalBytesShowProgress = totalCopied;
            ShowProgress(totalCopied, totalBytes);
        }

        if (_kbhit() && _getch() == 27) // ESC Tusu
        {
            _tprintf(_T("Kullanici tarafindan iptal edildi\n"));
            break;
        }
    }

    if (totalBytesShowProgress != totalBytes) 
    {
        totalBytesShowProgress = totalBytes;
        ShowProgress(totalCopied, totalBytes);
    }   

    _tprintf(_T("\n\nIslem tamamlandi. Toplam kopyalanan: %d byte\n\n"), totalCopied);

    writer.Close();

    reader.Close();

    _tprintf(_T("Cikmak icin bir tusa basin..."));
    
    int val = _getch();

    return 0;
}




// Satýr iþleme callback fonksiyon tipi
typedef void (*LineCallback)(const std::string& line, DWORD lineNumber);

// Satýr yazdýrma callback'i
void PrintLine(const std::string& line, DWORD lineNumber)
{
    _tprintf(_T("%4d: %hs\n"), lineNumber, line.c_str());
}

void ExtractLinesFromBuffer(const std::string& buffer, std::vector<std::string>& lines, std::string& remaining)
{
    size_t start = 0;
    size_t end = buffer.find('\n');

    while (end != std::string::npos)
    {
        std::string line = buffer.substr(start, end - start);

        // Windows'taki \r\n desteði
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        lines.push_back(line);
        start = end + 1;
        end = buffer.find('\n', start);
    }

    // Kalan kismi sakla (tam satir olmayan veri)
    remaining = buffer.substr(start);

}

int runTextFileCopyExample()
{
    const TCHAR* srcFileName = _T("../input_text.txt");
    const TCHAR* dstFilename = _T("../output_text.txt");

    _tprintf(_T("Text dosyasý satýr satýr kopyalanýyor...\n"));
    _tprintf(_T("Kaynak: %s -> Hedef: %s\n\n"), srcFileName, dstFilename);

    CFileHandler reader;
    BOOL successRd = reader.Open(srcFileName, GENERIC_READ, OPEN_EXISTING);
    if (!successRd) {
        _tprintf(_T("Kaynak dosya acilamadi! Hata: %d\n"), GetLastError());
        return 1;
    }

    CFileHandler writer;
    BOOL successWr = writer.Open(dstFilename, GENERIC_WRITE, OPEN_ALWAYS, FILE_SHARE_READ, FALSE);
    if (!successWr) {
        _tprintf(_T("Hedef dosya acilamadi! Hata: %d\n"), GetLastError());
        reader.Close();
        return 1;
    }

    const DWORD bufferSize = 4096;
    CHAR buffer[bufferSize];
    DWORD bytesRead, bytesWritten;
    DWORD totalBytes = reader.GetFileSize();
    DWORD totalCopied = 0;
    DWORD totalBytesShowProgress = 0;
    DWORD lineCount = 0;

    std::string remainingData;
    std::vector<std::string>lines;

    _tprintf(_T("Toplam kopyalanacak boyut : %d byte\n\n"), totalBytes);

    while (true)
    {
        bool statusRead = reader.Read(buffer, bufferSize-1, &bytesRead);
        if (!statusRead || bytesRead == 0)
        {
            break;
        }

        buffer[bytesRead] = '\0'; // NULL-terminating string

        std::string currentData = remainingData + buffer;
        remainingData.clear();
        ExtractLinesFromBuffer(currentData, lines, remainingData);
        for (const auto& line : lines) {
            std::string lineToWrite = line + "\r\n";
            bool statusWrite = writer.Write((LPVOID)lineToWrite.c_str(), lineToWrite.size(), &bytesWritten);
            if (!statusWrite)
            {
                _tprintf(_T("Yazma hatasi! Hata: %d\n"), GetLastError());
                reader.Close();
                writer.Close();
                return 1;
            }

            totalCopied += bytesWritten;
            lineCount++;
            if (lineCount % 100 == 0)
            {
                totalBytesShowProgress = totalCopied;
                ShowProgress(totalCopied, totalBytes);
                //PrintLine(line, lineCount);
            }
        }
        lines.clear();

        if (_kbhit() && _getch() == 27) // ESC Tusu
        {
            _tprintf(_T("Kullanici tarafindan iptal edildi\n"));
            break;
        }
    }

    // Kalan veriyi yaz (son satýr \n ile bitmiyorsa)
    if (!remainingData.empty())
    {
        bool statusWrite = writer.Write((LPVOID)remainingData.c_str(), remainingData.size(), &bytesWritten);
        if (!statusWrite)
        {
            _tprintf(_T("Son satýr yazma hatasi! Hata: %d\n"), GetLastError());
        }
        else
        {
            totalCopied += bytesWritten;
            lineCount++;
        }
    }

    if (totalBytesShowProgress != totalBytes)
    {
        totalBytesShowProgress = totalBytes;
        ShowProgress(totalCopied, totalBytes);
        //PrintLine(remainingData, lineCount);
    }

    _tprintf(_T("\n\nIslem tamamlandi. Toplam %d satir kopyalandi, %d byte\n\n"), lineCount, totalCopied);

    writer.Close();

    reader.Close();

    _tprintf(_T("Cikmak icin bir tusa basin..."));

    int val = _getch();

    return 0;
}

using namespace std;
atomic<bool> g_bRunnig(false);
std::wstring g_sharedFilePath;

void WriterThread();
void ReaderThread();
void BinaryFileCopy();
void TextFileCopy();

void ThreadedFileIO()
{
    g_sharedFilePath = L"../shared_txt_file.txt";
    std::wcout << L"Paylasimli Dosya Yolu : " << g_sharedFilePath << std::endl;

    g_bRunnig = true;

    thread writerThread(WriterThread);
    thread readerThread(ReaderThread);

    std::wcout << L"Calisiyor...(ESC ile durdur)\n";

    while (g_bRunnig)
    {
        if (_kbhit() && _getch() == 27) // ESC Tusu
        {
            g_bRunnig = false;
            break;
        }
        Sleep(100);
    }

    writerThread.join();
    readerThread.join();

    std::wcout << L"Threadli islem sonlandirildi.\n";
}


void WriterThread()
{
    CFileHandler file;
    if (!file.Open(g_sharedFilePath.c_str(), GENERIC_WRITE, OPEN_ALWAYS, FILE_SHARE_READ | FILE_SHARE_WRITE, TRUE))
    {
        // error
        return;
    }

    std::wcout << L"Yazici thread basladi...\n";

    while(g_bRunnig)
    {
        if (_kbhit())
        {
            int ch = _getch();
            if (ch == 27) continue;

            DWORD bytesWritten;
            char c = static_cast<char>(ch);
            if (!file.Write(&c, 1, &bytesWritten))
            {
                // error
                break;
            }
            wcout << L"[Yazildi : '" << c << L"'] ";
        }
        Sleep(50);

        if (_kbhit() && _getch() == 27) // ESC Tusu
        {
            g_bRunnig = false;
            break;
        }
        Sleep(100);
    }
    file.Close();
}

void ReaderThread()
{
    CFileHandler file;
    if (!file.Open(g_sharedFilePath.c_str(), GENERIC_READ, OPEN_EXISTING, FILE_SHARE_READ | FILE_SHARE_WRITE))
    {
        // error
        return;
    }

    std::wcout << L"Okuucu thread basladi...\n";

    DWORD lastSize = 0;
    DWORD currentPos = 0;

    while (g_bRunnig)
    {
        DWORD fileSize = file.GetFileSize();
        if (fileSize > lastSize)
        {
            file.SetPosition(currentPos, FILE_BEGIN);

            DWORD bytesToRead = fileSize - currentPos;
            vector<char> buffer(bytesToRead + 1);
            DWORD bytesRead;

            if (file.Read(buffer.data(), bytesToRead, &bytesRead) && bytesRead > 0)
            {
                buffer[bytesRead] = '\0';
                wcout << L"\n[Okunan : \"" << buffer.data() << L"\"] ";
                currentPos += bytesRead;
            } 
        }
        lastSize = fileSize;
        Sleep(200);
    }
    file.Close();
}

void BinaryFileCopy()
{

}

void TextFileCopy()
{

}

int main()
{
    while (true)
    {
        std::wcout << L"\nDosya Kopyalama\n";
        std::wcout << L"======================\n";
        std::wcout << L"  1. BinaryFileCopy\n";
        std::wcout << L"  2. TextFileCopy\n";
        std::wcout << L"  3. ThreadedFileIO\n";
        std::wcout << L"ESC. Cikis\n\n";
        std::wcout << L"Seciminiz :";

        int choice = _getch();

        if (choice == 27) // ESC
        {
            std::wcout << L"Uygulamadan cikiliyor...\n\n";
            break;
        }
        else if (choice == '1')
        {
            runBinaryFileCopyExample();
            std::wcout << L"\n";
        }
        else if (choice == '2')
        {
            runTextFileCopyExample();
            std::wcout << L"\n";
        }
        else if (choice == '3')
        {
            ThreadedFileIO();
            std::wcout << L"\n";
        }
        else
        {
            std::wcout << L"\nGecersiz secim! Tekrar deneyin.\n";
        }
    }

    return 0;
}