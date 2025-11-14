#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memcpy
#include <conio.h> // for _getch and _kbhit (Windows-specific)
#include <windows.h> // for Sleep
#include <math.h> // for abs()

#define GRID_ROWS 28
#define GRID_COLS 100
#define RESET "\033[0m"
#define BLUE "\033[34m"
#define RED "\033[31m"
#define YELLOW "\033[33m"

// Structure to represent an enemy entity
typedef struct {
    int x, y;
    int dir;       // 0=Up, 1=Right, 2=Down, 3=Left
    int patrolDir; // +1 or -1
    int isRow;     // 1 for horizontal (row) movement, 0 for vertical (column)
    int isAlive;   // 1 if enemy is alive, 0 if killed
} Entity;

char grid[GRID_ROWS][GRID_COLS];
Entity enemies[10]; // Fixed size array instead of vector (can hold up to 10 enemies)
int enemyCount = 0;
int playerX = 1, playerY = 1; // Starting position
int playerDir = 1; // Player direction: 0=Up, 1=Right, 2=Down, 3=Left

void initializeGrid() {
    // Initialize grid with empty spaces
    memset(grid, ' ', sizeof(grid));
    
    // Create a bigger maze-like pattern with horizontal and vertical walls
    // Borders
    // Borders: top/bottom across columns, left/right across rows
    for (int col = 0; col < GRID_COLS; col++) {
        grid[0][col] = '-';
        grid[GRID_ROWS-1][col] = '-';
    }
    for (int row = 0; row < GRID_ROWS; row++) {
        grid[row][0] = '|';
        grid[row][GRID_COLS-1] = '|';
    }

    // Several horizontal wall bands with gaps (row indices must be < GRID_ROWS)
    if (GRID_ROWS > 4)  for (int c = 2; c < GRID_COLS-2; c++) grid[4][c] = '-';
    if (GRID_ROWS > 9)  for (int c = 2; c < GRID_COLS-6; c++) grid[9][c] = '-';
    if (GRID_ROWS > 14) for (int c = 6; c < GRID_COLS-4; c++) grid[14][c] = '-';
    if (GRID_ROWS > 20) for (int c = 8; c < GRID_COLS-2; c++) grid[20][c] = '-';
    if (GRID_ROWS > 26) for (int c = 10; c < GRID_COLS-3; c++) grid[26][c] = '-';

    // Several vertical wall bands with gaps (column indices must be < GRID_COLS)
    if (GRID_COLS > 6)  for (int r = 2; r < 12 && r < GRID_ROWS; r++) grid[r][6] = '|';
    if (GRID_COLS > 12) for (int r = 8; r < 22 && r < GRID_ROWS; r++) grid[r][12] = '|';
    if (GRID_COLS > 18) for (int r = 12; r < 30 && r < GRID_ROWS; r++) grid[r][18] = '|';
    if (GRID_COLS > 24) for (int r = 16; r < GRID_ROWS-2; r++) grid[r][24] = '|';
    if (GRID_COLS > 30) for (int r = 4; r < GRID_ROWS-6; r++) grid[r][30] = '|';

    // Openings / passages in walls (gaps) -- ensure indexes are in range
    if (GRID_ROWS > 4 && GRID_COLS > 5)  grid[4][5] = ' ';
    if (GRID_ROWS > 9 && GRID_COLS > 7)  grid[9][7] = ' ';
    if (GRID_ROWS > 14 && GRID_COLS > 10) grid[14][10] = ' ';
    if (GRID_ROWS > 20 && GRID_COLS > 12) grid[20][12] = ' ';
    if (GRID_ROWS > 26 && GRID_COLS > 14) grid[26][14] = ' ';

    if (GRID_ROWS > 6 && GRID_COLS > 6)  grid[6][6] = ' ';
    if (GRID_ROWS > 12 && GRID_COLS > 12) grid[12][12] = ' ';
    if (GRID_ROWS > 18 && GRID_COLS > 18) grid[18][18] = ' ';
    if (GRID_ROWS > 22 && GRID_COLS > 24) grid[22][24] = ' ';
    if (GRID_ROWS > 8 && GRID_COLS > 30)  grid[8][30] = ' ';
}

void addEnemy(int x, int y, int dir, int patrolDir, int isRow) {
    if (enemyCount < 10) {
        enemies[enemyCount].x = x;
        enemies[enemyCount].y = y;
        enemies[enemyCount].dir = dir;
        enemies[enemyCount].patrolDir = patrolDir;
        enemies[enemyCount].isRow = isRow;
        enemies[enemyCount].isAlive = 1; // Initialize as alive
        enemyCount++;
    }
}

// Check if player is adjacent to an enemy (returns enemy index or -1 if none)
int getAdjacentEnemy() {
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].isAlive) continue; // Skip dead enemies
        
        // Check all adjacent positions (up, down, left, right)
        if ((abs(enemies[i].x - playerX) == 1 && enemies[i].y == playerY) ||
            (abs(enemies[i].y - playerY) == 1 && enemies[i].x == playerX)) {
            return i;
        }
    }
    return -1;
}

void initializeEnemies() {
    // Add more enemies across the expanded map
    addEnemy(3, 3, 1, 1, 1);     // top-left horizontal patrol
    addEnemy(8, 8, 2, 1, 0);     // small vertical patrol
    addEnemy(13, 20, 3, -1, 1);  // mid horizontal patrol
    addEnemy(18, 10, 2, 1, 0);   // vertical in center-left
    addEnemy(50, 30, 1, -1, 1);  // bottom-right horizontal (moved right)
    addEnemy(80, 6, 3, 1, 1);    // near far right-side horizontal
    addEnemy(70, 24, 0, -1, 0);  // lower vertical patrol (moved right)
    // leave room for up to 10 enemies; unused slots stay empty
}

int main() {
    initializeGrid();
    initializeEnemies();
    
    int timeCounter = 0;
    
    while (1) {
        // Handle player input (arrow keys)
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 'k' || ch == 'K') {
                int enemyIdx = getAdjacentEnemy();
                if (enemyIdx != -1) {
                    enemies[enemyIdx].isAlive = 0;
                    printf("\a"); // Make a beep sound for feedback
                }
            }
            else if (ch == 224) { // Arrow key prefix
                ch = _getch();
                int dx = 0, dy = 0;
                if (ch == 72) { dy = -1; playerDir = 0; } // Up arrow
                else if (ch == 80) { dy = 1; playerDir = 2; } // Down arrow
                else if (ch == 75) { dx = -1; playerDir = 3; } // Left arrow
                else if (ch == 77) { dx = 1; playerDir = 1; } // Right arrow
                
                int nx = playerX + dx;
                int ny = playerY + dy;
                if (nx >= 0 && nx < GRID_COLS && ny >= 0 && ny < GRID_ROWS && 
                    grid[ny][nx] != '-' && grid[ny][nx] != '|') {
                    playerX = nx;
                    playerY = ny;
                }
            }
        }
        
        // Move enemies (patrol logic) and update their direction
        for (int i = 0; i < enemyCount; i++) {
            if (!enemies[i].isAlive) continue; // Skip dead enemies
            
            if (enemies[i].isRow) {
                int newX = enemies[i].x + enemies[i].patrolDir;
                if (newX < 0 || newX >= GRID_COLS || grid[enemies[i].y][newX] == '-' || grid[enemies[i].y][newX] == '|') {
                    enemies[i].patrolDir = -enemies[i].patrolDir;
                    newX = enemies[i].x + enemies[i].patrolDir;
                }
                enemies[i].x = newX;
                // Update direction based on horizontal movement
                enemies[i].dir = (enemies[i].patrolDir > 0) ? 1 : 3; // 1 for right, 3 for left
            } else {
                int newY = enemies[i].y + enemies[i].patrolDir;
                if (newY < 0 || newY >= GRID_ROWS || grid[newY][enemies[i].x] == '-' || grid[newY][enemies[i].x] == '|') {
                    enemies[i].patrolDir = -enemies[i].patrolDir;
                    newY = enemies[i].y + enemies[i].patrolDir;
                }
                enemies[i].y = newY;
                // Update direction based on vertical movement
                enemies[i].dir = (enemies[i].patrolDir > 0) ? 2 : 0; // 2 for down, 0 for up
            }
        }
        
    // Create display grid
    char display[GRID_ROWS][GRID_COLS];
    memcpy(display, grid, sizeof(grid));
        
        // Place enemies
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].isAlive) {
                display[enemies[i].y][enemies[i].x] = 'E';
            }
        }
        
        // Place player with directional arrow
        char playerChar;
        switch(playerDir) {
            case 0: playerChar = '^'; break; // Up
            case 1: playerChar = '>'; break; // Right
            case 2: playerChar = 'v'; break; // Down
            case 3: playerChar = '<'; break; // Left
            default: playerChar = '^';
        }
        display[playerY][playerX] = playerChar;
        
        // Draw enemy vision (arrows)
        for (int i = 0; i < enemyCount; i++) {
            if (!enemies[i].isAlive) continue; // Skip dead enemies
            
            int dx = 0, dy = 0;
            char arrow;
            if (enemies[i].dir == 0) { dy = -1; arrow = '^'; }
            else if (enemies[i].dir == 1) { dx = 1; arrow = '>'; }
            else if (enemies[i].dir == 2) { dy = 1; arrow = 'v'; }
            else { dx = -1; arrow = '<'; }
            
            for (int j = 1; j <= 3; j++) {
                int nx = enemies[i].x + dx * j;
                int ny = enemies[i].y + dy * j;
                if (nx < 0 || nx >= GRID_COLS || ny < 0 || ny >= GRID_ROWS || 
                    grid[ny][nx] == '-' || grid[ny][nx] == '|') break;
                display[ny][nx] = arrow;
            }
        }
        
        // Check if all enemies are dead
        int allEnemiesDead = 1;
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].isAlive) {
                allEnemiesDead = 0;
                break;
            }
        }
        
        // If all enemies are dead, player wins
        if (allEnemiesDead) {
            system("cls");
            printf("\n\n\t🎉 Congratulations! You have eliminated all enemies and won the game! 🎉\n");
            break;
        }
        
        // Check for detection
        int detected = 0;
        for (int i = 0; i < enemyCount; i++) {
            if (!enemies[i].isAlive) continue; // Dead enemies can't detect player
            
            int dx = 0, dy = 0;
            if (enemies[i].dir == 0) dy = -1;
            else if (enemies[i].dir == 1) dx = 1;
            else if (enemies[i].dir == 2) dy = 1;
            else dx = -1;
            
            for (int j = 1; j <= 3; j++) {
                int nx = enemies[i].x + dx * j;
                int ny = enemies[i].y + dy * j;
                if (nx < 0 || nx >= GRID_COLS || ny < 0 || ny >= GRID_ROWS || 
                    grid[ny][nx] == '-' || grid[ny][nx] == '|') break;
                if (nx == playerX && ny == playerY) {
                    detected = 1;
                    break;
                }
            }
            if (detected) break;
        }
        
        if (detected) {
            printf("💀 You were spotted! Game Over!\n");
            break;
        }
        
        // Redraw the grid
        system("cls");
        for (int i = 0; i < GRID_ROWS; i++) {
            for (int j = 0; j < GRID_COLS; j++) {
                char c = display[i][j];
                if (j == playerX && i == playerY) printf("%s%c%s", BLUE, c, RESET);
                else if (c == 'E') printf("%s%c%s", RED, c, RESET);
                else if (c == '^' || c == '>' || c == 'v' || c == '<') printf("%s%c%s", YELLOW, c, RESET);
                else printf("%c", c);
            }
            printf("\n");
        }
        
        // Wait a bit for smoother movement
        Sleep(100);
        timeCounter++;
    }
    
    return 0;
}