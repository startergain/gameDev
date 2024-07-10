// VideoTeris.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <vector>
using namespace std;

std::wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;

int nScreenWidth = 80;
int nScreenHeight = 30;

int Rotate(int px, int py, int r) {

    switch (r % 4) {
    case 0: return py * 4 + px;         // 0 degrees
    case 1: return 12 + py - (px * 4);  // 90 degrees
    case 2: return 15 - (py * 4) - px;  // 180 degrees
    case 3: return 3 - py + (px * 4);   // 270 degrees
    }
    return 0;
}

bool DosePieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
    
    for (int px = 0; px < 4; px++)
    {
        for (int py = 0; py < 4; py++)
        {
            // Get index into piece
            int pi = Rotate(px, py, nRotation);

            // Get index into Field
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

            if (nPosX + px >=0 && nPosX + px < nFieldWidth)
            {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
                    if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0) {

                        return false; // Fail on first hit  
                    }
                }
            }
        }
    }

    return true;
}

int main()
{
    // Create assets
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    
    tetromino[1].append(L".XX.");
    tetromino[1].append(L"..X.");
    tetromino[1].append(L"..X.");
    tetromino[1].append(L"....");

    tetromino[2].append(L"..X.");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L".X..");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"..X.");
    tetromino[3].append(L"....");

    tetromino[4].append(L"....");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"....");

    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"....");

    tetromino[6].append(L"....");
    tetromino[6].append(L"..XX");
    tetromino[6].append(L"..X.");
    tetromino[6].append(L"..X.");

    pField = new unsigned char[nFieldWidth * nFieldHeight]; // create play Field
    for (int x = 0; x < nFieldWidth; x++)
    {
        for (int y = 0; y < nFieldHeight; y++)
        {
            pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
        }
    }
    
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth*nScreenHeight; i++)
    {
        screen[i] = L' ';
    }
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwByteWritten = 0;

    // Game logic stuff
    bool bGameOver = false;

    int nCurrentPiece = 1;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;

    bool bKey[4];
    bool bRotateHold = false;
    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCount = 0;
    int nScore =0;

    vector<int> vLines;
    while (!bGameOver)
    {

        // GAME TIMING ===========================================
        this_thread::sleep_for(50ms);
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed);
        // INPUT =================================================
        for (int k = 0; k < 4; k++)
        {
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x25\x27\x28Z"[k]))) != 0;
        }
        // GAME LOGIC ============================================
        nCurrentX -= (bKey[0] && (DosePieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY))) ? 1 : 0; ;
            
        nCurrentX += (bKey[1] && (DosePieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY))) ?1:0;
            
        nCurrentY += (bKey[2] && (DosePieceFit(nCurrentPiece, nCurrentRotation, nCurrentX , nCurrentY +1))) ? 1 : 0;
            
        if (bKey[3])
        {
            nCurrentRotation += (!bRotateHold && (DosePieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY))) ? 1 : 0;
            bRotateHold = true;
        }
        else
            bRotateHold = false;

        if (bForceDown)
        {
            if (DosePieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
                nCurrentY++;
            else
            {
                // Lock the current piece in the field
                for (int px = 0; px < 4; px++)
                {
                    for (int py = 0; py < 4; py++)
                    {
                        if (tetromino[nCurrentPiece][Rotate(px,py,nCurrentRotation)] == L'X')
                        {
                            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
                        }
                    }
                }
                nPieceCount++;
                if (nPieceCount % 10 == 0)
                {
                    if (nSpeed >= 10) nSpeed--;
                }
                // Check have we got any lines
                for (int py = 0; py < 4; py++)
                {
                    if (nCurrentY + py < nFieldHeight -1)
                    {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1; px++)
                        {
                            bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
                        }
                        if (bLine)
                        {
                            // Remove line set to =
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8; 
           
                            vLines.push_back(nCurrentY + py);
                        }
                    }
                }

                nScore += 25;
                if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;
                // Choose next piece
                nCurrentPiece = rand()%7;
                nCurrentRotation = 0;
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                // If the piece dose not fit
                bGameOver = !DosePieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
            nSpeedCounter = 0;
        }
        // RENDER　OUTPUT ========================================



        // Draw Field
        for (int x = 0; x < nFieldWidth; x++)
        {
            for (int y = 0; y < nFieldHeight; y++)
            {
                screen[(y +2) * nScreenWidth + (x +2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
            }
        }

        // Draw current piece
        for (int px = 0; px < 4; px++)
        {
            for (int py = 0; py < 4; py++)
            {
                if (tetromino[nCurrentPiece][Rotate(px,py,nCurrentRotation)] == L'X')
                {
                    screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
                }
            }
        }
        // Draw score
        swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"Score:%8d", nScore); 

        if (!vLines.empty()) {
            // Display Frame (checkily to draw the lines)
            WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwByteWritten);
            this_thread::sleep_for(400ms); // Delay a bit

            for (auto &v :vLines)
            {
                for (int px = 1; px < nFieldWidth -1; px++)
                {
                    for (int py = v; py >0; py--)
                    {
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];

                    }
                    pField[px] = 0;
                }
            }
            vLines.clear();
        } 
        
        // Display Frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwByteWritten);

    }

    // oh dear
    CloseHandle(hConsole);
    cout << "Game over!! Score:" << nScore << endl;
    system("pause");
    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
