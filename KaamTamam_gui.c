#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Simple Win32 GUI version of the KaamTamam game.
// Controls: Arrow keys to move, K to kill adjacent enemy.
// This is a single-file C program using the Win32 API (no C++).

#define GRID_ROWS 20
#define GRID_COLS 60

typedef struct {
    int x, y;
    int dir;       // 0=Up,1=Right,2=Down,3=Left
    int patrolDir; // +1 or -1
    int isRow;     // 1 for horizontal, 0 for vertical
    int isAlive;   // 1 alive
} Entity;

static char grid[GRID_ROWS][GRID_COLS];
static char displayBuf[GRID_ROWS][GRID_COLS];
static Entity enemies[10];
static int enemyCount = 0;
static int playerX = 1, playerY = 1;
static int playerDir = 1; // right
static int gameOver = 0; // 0 running, 1 lost, 2 won

// GUI metrics
static const int cellW = 10;
static const int cellH = 14;
static HFONT hFont = NULL;

// forward
void initializeGridGUI();
void initializeEnemiesGUI();
void updateGameStep();
int getAdjacentEnemyIndex();

void initializeGridGUI() {
    // fill with spaces
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            grid[r][c] = ' ';

    // borders
    for (int c = 0; c < GRID_COLS; c++) {
        grid[0][c] = '-';
        grid[GRID_ROWS-1][c] = '-';
    }
    for (int r = 0; r < GRID_ROWS; r++) {
        grid[r][0] = '|';
        grid[r][GRID_COLS-1] = '|';
    }

    // some horizontal walls
    for (int c = 2; c < GRID_COLS-4; c++) grid[3][c] = '-';
    for (int c = 4; c < GRID_COLS-10; c++) grid[7][c] = '-';
    for (int c = 6; c < GRID_COLS-8; c++) grid[11][c] = '-';
    for (int c = 10; c < GRID_COLS-6; c++) grid[15][c] = '-';

    // vertical walls
    for (int r = 2; r < 9; r++) grid[r][8] = '|';
    for (int r = 6; r < 13; r++) grid[r][18] = '|';
    for (int r = 10; r < 18; r++) grid[r][30] = '|';
    for (int r = 4; r < GRID_ROWS-2; r++) grid[r][44] = '|';

    // openings
    grid[3][5] = ' ';
    grid[7][9] = ' ';
    grid[11][14] = ' ';
    grid[15][20] = ' ';
    grid[6][8] = ' ';
    grid[12][18] = ' ';
    grid[14][30] = ' ';
    grid[10][44] = ' ';
}

void addEnemyGUI(int x, int y, int dir, int patrolDir, int isRow) {
    if (enemyCount < (int)(sizeof(enemies)/sizeof(enemies[0]))) {
        enemies[enemyCount].x = x;
        enemies[enemyCount].y = y;
        enemies[enemyCount].dir = dir;
        enemies[enemyCount].patrolDir = patrolDir;
        enemies[enemyCount].isRow = isRow;
        enemies[enemyCount].isAlive = 1;
        enemyCount++;
    }
}

void initializeEnemiesGUI() {
    enemyCount = 0;
    addEnemyGUI(5, 2, 1, 1, 1);
    addEnemyGUI(20, 6, 2, 1, 0);
    addEnemyGUI(12, 10, 3, -1, 1);
    addEnemyGUI(35, 11, 0, -1, 0);
    addEnemyGUI(48, 8, 1, 1, 1);
    addEnemyGUI(40, 15, 2, -1, 0);
}

int getAdjacentEnemyIndex() {
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].isAlive) continue;
        if ((abs(enemies[i].x - playerX) == 1 && enemies[i].y == playerY) ||
            (abs(enemies[i].y - playerY) == 1 && enemies[i].x == playerX))
            return i;
    }
    return -1;
}

void updateGameStep() {
    if (gameOver) return;

    // move enemies and update direction according to movement
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].isAlive) continue;
        if (enemies[i].isRow) {
            int nx = enemies[i].x + enemies[i].patrolDir;
            if (nx < 0 || nx >= GRID_COLS || grid[enemies[i].y][nx] == '-' || grid[enemies[i].y][nx] == '|') {
                enemies[i].patrolDir = -enemies[i].patrolDir;
                nx = enemies[i].x + enemies[i].patrolDir;
            }
            enemies[i].x = nx;
            enemies[i].dir = (enemies[i].patrolDir > 0) ? 1 : 3;
        } else {
            int ny = enemies[i].y + enemies[i].patrolDir;
            if (ny < 0 || ny >= GRID_ROWS || grid[ny][enemies[i].x] == '-' || grid[ny][enemies[i].x] == '|') {
                enemies[i].patrolDir = -enemies[i].patrolDir;
                ny = enemies[i].y + enemies[i].patrolDir;
            }
            enemies[i].y = ny;
            enemies[i].dir = (enemies[i].patrolDir > 0) ? 2 : 0;
        }
    }

    // build display buffer from grid
    memcpy(displayBuf, grid, sizeof(grid));

    // place enemies and their vision
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].isAlive) continue;
        displayBuf[enemies[i].y][enemies[i].x] = 'E';
        int dx = 0, dy = 0; char arrow = '^';
        if (enemies[i].dir == 0) { dy = -1; arrow = '^'; }
        else if (enemies[i].dir == 1) { dx = 1; arrow = '>'; }
        else if (enemies[i].dir == 2) { dy = 1; arrow = 'v'; }
        else { dx = -1; arrow = '<'; }
        for (int step = 1; step <= 3; step++) {
            int nx = enemies[i].x + dx*step;
            int ny = enemies[i].y + dy*step;
            if (nx < 0 || nx >= GRID_COLS || ny < 0 || ny >= GRID_ROWS) break;
            if (grid[ny][nx] == '-' || grid[ny][nx] == '|') break;
            displayBuf[ny][nx] = arrow;
        }
    }

    // place player char
    char pch = (playerDir==0? '^' : playerDir==1? '>' : playerDir==2? 'v' : '<');
    displayBuf[playerY][playerX] = pch;

    // detection
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].isAlive) continue;
        int dx = 0, dy = 0;
        if (enemies[i].dir == 0) dy = -1;
        else if (enemies[i].dir == 1) dx = 1;
        else if (enemies[i].dir == 2) dy = 1;
        else dx = -1;
        for (int step = 1; step <= 3; step++) {
            int nx = enemies[i].x + dx*step;
            int ny = enemies[i].y + dy*step;
            if (nx < 0 || nx >= GRID_COLS || ny < 0 || ny >= GRID_ROWS) break;
            if (grid[ny][nx] == '-' || grid[ny][nx] == '|') break;
            if (nx == playerX && ny == playerY) {
                gameOver = 1; // lost
                return;
            }
        }
    }

    // win check
    int allDead = 1;
    for (int i = 0; i < enemyCount; i++) if (enemies[i].isAlive) { allDead = 0; break; }
    if (allDead) { gameOver = 2; }
}

// Win32 app
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        // create font
        hFont = CreateFontA(cellH - 2, cellW, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            FIXED_PITCH | FF_DONTCARE, "Consolas");
        SetTimer(hWnd, 1, 120, NULL); // 120ms tick
        return 0;

    case WM_TIMER:
        updateGameStep();
        InvalidateRect(hWnd, NULL, TRUE);
        if (gameOver) {
            KillTimer(hWnd, 1);
            if (gameOver == 1) MessageBoxA(hWnd, "You were spotted! Game Over!", "KaamTamam", MB_OK|MB_ICONERROR);
            else MessageBoxA(hWnd, "You win! All enemies eliminated.", "KaamTamam", MB_OK|MB_ICONINFORMATION);
            PostQuitMessage(0);
        }
        return 0;

    case WM_KEYDOWN: {
        int vk = (int)wParam;
        if (vk == 'K') {
            int idx = getAdjacentEnemyIndex();
            if (idx != -1) {
                enemies[idx].isAlive = 0;
            }
            return 0;
        }
        if (vk == VK_UP) { playerDir = 0; if (playerY-1 >=0 && grid[playerY-1][playerX] != '-' && grid[playerY-1][playerX] != '|') playerY--; }
        else if (vk == VK_DOWN) { playerDir = 2; if (playerY+1 < GRID_ROWS && grid[playerY+1][playerX] != '-' && grid[playerY+1][playerX] != '|') playerY++; }
        else if (vk == VK_LEFT) { playerDir = 3; if (playerX-1 >=0 && grid[playerY][playerX-1] != '-' && grid[playerY][playerX-1] != '|') playerX--; }
        else if (vk == VK_RIGHT) { playerDir = 1; if (playerX+1 < GRID_COLS && grid[playerY][playerX+1] != '-' && grid[playerY][playerX+1] != '|') playerX++; }
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        HFONT old = (HFONT)SelectObject(hdc, hFont);

        // background
        RECT rc; GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW+1));

        // draw grid characters
        for (int r = 0; r < GRID_ROWS; r++) {
            for (int c = 0; c < GRID_COLS; c++) {
                char ch = displayBuf[r][c];
                COLORREF col = RGB(0,0,0);
                if (ch == 'E') col = RGB(200,0,0);
                else if (ch == '^' || ch == '>' || ch == 'v' || ch == '<') col = RGB(200,160,0);
                else if (ch == '|' || ch == '-') col = RGB(100,100,100);
                else if (r == playerY && c == playerX) col = RGB(0,120,200);
                else col = RGB(30,30,30);

                SetTextColor(hdc, col);
                char s[2] = { ch, 0 };
                int x = c * cellW + 4;
                int y = r * cellH + 4;
                TextOutA(hdc, x, y, s, 1);
            }
        }

        // status line
        SetTextColor(hdc, RGB(0,0,0));
        char status[128];
        int alive = 0; for (int i = 0; i < enemyCount; i++) if (enemies[i].isAlive) alive++;
        sprintf(status, "Enemies remaining: %d   Use arrow keys to move, K to kill adjacent enemy.", alive);
        TextOutA(hdc, 8, GRID_ROWS*cellH + 6, status, (int)strlen(status));

        SelectObject(hdc, old);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        if (hFont) DeleteObject(hFont);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    initializeGridGUI();
    initializeEnemiesGUI();
    memset(displayBuf, ' ', sizeof(displayBuf));

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KaamTamamClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    int winW = GRID_COLS * cellW + 24;
    int winH = GRID_ROWS * cellH + 80;
    HWND hwnd = CreateWindowExA(0, wc.lpszClassName, "KaamTamam - GUI", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, winW, winH, NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
