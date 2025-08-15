/*    Copyright (c) 2025 Sushant kr. Ray
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a copy
 *    of this software and associated documentation files (the "Software"), to deal
 *    in the Software without restriction, including without limitation the rights
 *    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the Software is
 *    furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in all
 *    copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *    SOFTWARE.
 */

#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

#define BOARD_SIZE 3
#define CELL_W 5
#define CELL_H 3
#define ANIM_DELAY 60000

typedef enum { EMPTY = ' ', HUMAN = 'X', BOT = 'O' } Piece;

Piece board[BOARD_SIZE][BOARD_SIZE];
int cursor_r = 0, cursor_c = 0;

void init_board();
void draw_board();
void draw_cell(int r, int c, int highlight);
int check_winner(Piece *winner, int *pos, int *wdir_r, int *wdir_c);
int is_board_full();
void bot_move();
void animate_win_line(int pos, int dir_r, int dir_c);

int main() {
    initscr();
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        attron(COLOR_PAIR(1));
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(-1);

    init_board();
    draw_board();

    Piece winner = EMPTY;
    int pos=0, wdr=0, wdc=0;
    int ch = 0;

    while (1) {
        if(!ch)
            ch = getch();
        bool redraw = false;

        if (ch == KEY_UP || ch == 'k' || ch == 'w') {
            cursor_r = (cursor_r + BOARD_SIZE - 1) % BOARD_SIZE;
            redraw = true;
        } else if (ch == KEY_DOWN || ch == 'j' || ch == 's') {
            cursor_r = (cursor_r + 1) % BOARD_SIZE;
            redraw = true;
        } else if (ch == KEY_LEFT || ch == 'h' || ch == 'a') {
            cursor_c = (cursor_c + BOARD_SIZE - 1) % BOARD_SIZE;
            redraw = true;
        } else if (ch == KEY_RIGHT || ch == 'l' || ch == 'd') {
            cursor_c = (cursor_c + 1) % BOARD_SIZE;
            redraw = true;
        } else if (ch == 'q') {
            break;
        } else if (ch == 'r') {
            init_board();
            winner = EMPTY;
            redraw = true;
        } else if (ch == ' ' || ch == '\n') {
            if (board[cursor_r][cursor_c] == EMPTY && winner == EMPTY) {
                board[cursor_r][cursor_c] = HUMAN;

                if (check_winner(&winner, &pos, &wdr, &wdc)) {
                    draw_board();
                    animate_win_line(pos, wdr, wdc);
                } else if (!is_board_full()) {
                    bot_move();

                    if (check_winner(&winner, &pos, &wdr, &wdc)) {
                        draw_board();
                        animate_win_line(pos, wdr, wdc);
                    } else redraw = true;
                } else redraw = true;
            }
        }

        if (redraw) draw_board();

        if (winner != EMPTY || is_board_full()) {
            if (winner == EMPTY) {
                Piece w = EMPTY;
                check_winner(&winner, &pos, &wdr, &wdc);
            }

            mvprintw(BOARD_SIZE * (CELL_H + 1) + 3, 0, "Game over. ");
            if (winner == HUMAN)
                printw("You (X) win! Press 'r' to restart or 'q' to quit.");
            else if (winner == BOT)
                printw("Bot (O) wins. Press 'r' to restart or 'q' to quit.");
            else
                printw("Draw. Press 'r' to restart or 'q' to quit.");

            refresh();
            ch = getch();
            if (ch == 'r') {
                init_board();
                winner = EMPTY;
                draw_board();
            } else if (ch == 'q') break;
        } else
            ch = 0;
    }

    attroff(COLOR_PAIR(1));
    endwin();
    return 0;
}

void init_board() {
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            board[r][c] = EMPTY;

    cursor_r = BOARD_SIZE / 2;
    cursor_c = BOARD_SIZE / 2;
}

void draw_board() {
    clear();
    int start_y = 1, start_x = 1;
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            int y = start_y + r * (CELL_H + 1);
            int x = start_x + c * (CELL_W + 1);

            if(r) {
                move(y, x);
                for (int i = 0; i < CELL_W; i++)
                    addch('-');

                if(c != BOARD_SIZE - 1)
                    addch('+');
            }

            if(c){
                for (int i = 0; i < CELL_H; i++)
                    mvaddch(y + i + 1, x - 1, '|');
            }

            int highlight = (r == cursor_r && c == cursor_c);
            draw_cell(r, c, highlight);
        }
    }

    mvprintw(0, 0, "Use arrow keys or h/j/k/l or WASD. Space/Enter to place. 'r' restart, 'q' quit.");
    refresh();
}

void draw_cell(int r, int c, int highlight) {
    int start_y = 2 + r * (CELL_H + 1);
    int start_x = 1 + c * (CELL_W + 1);
    Piece p = board[r][c];

    if (highlight)
        attron(A_REVERSE);

    int cy = start_y + CELL_H/2;
    int cx = start_x + CELL_W/2;

    for(int i = start_y; i < cy; i++)
    {
        move(i, start_x);
        for(int j = 0; j < CELL_W; j++)
            addch(' ');
    }

    move(cy, start_x);
    for(int j = 0; j < CELL_W/2; j++)
        addch(' ');

    printw("%c", (char)p);

    for(int j = cx + 1 - CELL_W; j < start_x; j++)
        addch(' ');

    for(int i = cy + 1; i < start_y + CELL_H; i++) {
        move(i, start_x);
        for(int j = 0; j < CELL_W; j++)
            addch(' ');
    }

    if (highlight) attroff(A_REVERSE);
}

int check_winner(Piece *winner, int *pos, int *wdir_r, int *wdir_c) {
    for (int r = 0; r < BOARD_SIZE; r++) {
        if (board[r][0] != EMPTY &&
            board[r][0] == board[r][1] &&
            board[r][1] == board[r][2]) {
            *winner = board[r][0];
            *pos = r;
            *wdir_r = 0;
            *wdir_c = 1;
            return 1;
        }
    }

    for (int c = 0; c < BOARD_SIZE; c++) {
        if (board[0][c] != EMPTY &&
            board[0][c] == board[1][c] &&
            board[1][c] == board[2][c]) {
            *winner = board[0][c];
            *pos = c;
            *wdir_r = 1;
            *wdir_c = 0;
            return 1;
        }
    }

    if (board[0][0] != EMPTY &&
        board[0][0] == board[1][1] &&
        board[1][1] == board[2][2]) {
        *winner = board[0][0];
        *pos = -1;
        *wdir_r = 1;
        *wdir_c = 1;
        return 1;
    }

    if (board[0][2] != EMPTY &&
        board[0][2] == board[1][1] &&
        board[1][1] == board[2][0]) {
        *winner = board[0][2];
        *pos = -2;
        *wdir_r = 1;
        *wdir_c = -1;
        return 1;
    }

    *winner = EMPTY;
    return 0;
}

int is_board_full() {
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            if (board[r][c] == EMPTY)
                return 0;

    return 1;
}

int check_possible_win(char PLAYER)
{
    for(int i = 0; i < BOARD_SIZE; i++) {
        int num_player = 0;
        int num_empty = 0;
        for(int j = 0; j < BOARD_SIZE; j++)
        {
            num_player += (board[i][j] == PLAYER);
            num_empty  += (board[i][j] == EMPTY);
        }

        if(num_player == BOARD_SIZE - 1 && num_empty)
        {
            for(int j = 0; j < BOARD_SIZE; j++)
                if(board[i][j] == EMPTY)
                    board[i][j] = BOT;
            return 1;
        }
    }

    for(int j = 0; j < BOARD_SIZE; j++) {
        int num_player = 0;
        int num_empty = 0;
        for(int i = 0; i < BOARD_SIZE; i++)
        {
            num_player += (board[i][j] == PLAYER);
            num_empty  += (board[i][j] == EMPTY);
        }

        if(num_player == BOARD_SIZE - 1 && num_empty)
        {
            for(int i = 0; i < BOARD_SIZE; i++)
                if(board[i][j] == EMPTY)
                    board[i][j] = BOT;
            return 1;
        }
    }

    {
        int num_player = 0;
        int num_empty = 0;
        for(int i = 0; i < BOARD_SIZE; i++)
        {
            num_player += (board[i][i] == PLAYER);
            num_empty  += (board[i][i] == EMPTY);
        }

        if(num_player == BOARD_SIZE - 1 && num_empty)
        {
            for(int i = 0; i < BOARD_SIZE; i++)
                if(board[i][i] == EMPTY)
                    board[i][i] = BOT;
            return 1;
        }
    }

    {
        int num_player = 0;
        int num_empty = 0;
        for(int i = 0; i < BOARD_SIZE; i++)
        {
            num_player += (board[i][BOARD_SIZE - i - 1] == PLAYER);
            num_empty  += (board[i][BOARD_SIZE - i - 1] == EMPTY);
        }

        if(num_player == BOARD_SIZE - 1 && num_empty)
        {
            for(int i = 0; i < BOARD_SIZE; i++)
                if(board[i][BOARD_SIZE - i - 1] == EMPTY)
                    board[i][BOARD_SIZE - i - 1] = BOT;
            return 1;
        }
    }

    return 0;
}

void bot_move()
{
    if(check_possible_win(BOT)) return;
    if(check_possible_win(HUMAN)) return;

    if(board[BOARD_SIZE/2][BOARD_SIZE/2] == EMPTY)
    {
        board[BOARD_SIZE/2][BOARD_SIZE/2] = BOT;
        return;
    }

    int corners[2] = {0, BOARD_SIZE - 1};

    if(board[corners[0]][corners[0]] == HUMAN &&
       board[corners[0]][corners[0]] == board[corners[1]][corners[1]] ||
       board[corners[0]][corners[1]] == HUMAN &&
       board[corners[0]][corners[1]] == board[corners[1]][corners[0]]){
        for(int i = 0; i < 2; i++){
            if(board[corners[i]][BOARD_SIZE/2] == EMPTY) {
               board[corners[i]][BOARD_SIZE/2] = BOT;
                return;
            }

            if(board[BOARD_SIZE/2][corners[i]] == EMPTY) {
               board[BOARD_SIZE/2][corners[i]] = BOT;
                return;
            }
        }
    }

    for(int i = 0; i < 2; i++)
        for(int j = 0; j < 2; j++) {
            if(board[corners[i]][corners[j]] == EMPTY) {
                board[corners[i]][corners[j]] = BOT;
                return;
            }
        }

    for(int i = 0; i < BOARD_SIZE; i++)
        for(int j = 0; j < BOARD_SIZE; j++){
            if(board[i][j] == EMPTY)
               board[i][j] = BOT;
                return;
            }
}

void animate_win_line(int pos, int dir_r, int dir_c) {
    char ch;
    int start_x = 1, start_y = 2;
    int num_lines = 0;

    if(!dir_c) {
        ch = '|';
        start_x += pos * (CELL_W + 1) + CELL_W / 2;
        num_lines = BOARD_SIZE * (CELL_H + 1) - 1;
    }

    else if(!dir_r){
        ch = '-';
        start_y += pos * (CELL_H + 1) + CELL_H / 2;
        num_lines = BOARD_SIZE * (CELL_W + 1) - 1;
    }

    else{
        int a = CELL_W < CELL_H ? CELL_W : CELL_H;
        num_lines = BOARD_SIZE * (a + 1) - 1;

        if(dir_c == -1){
            ch = '/';
            start_x += (CELL_W + 1)*(BOARD_SIZE/2) + CELL_W / 2 + num_lines / 2;
        }

        else {
            ch = '\\';
            start_x += (CELL_W + 1)*(BOARD_SIZE/2) + CELL_W / 2 - num_lines / 2;
        }
    }

    if (has_colors()) {
        attroff(COLOR_PAIR(1));
        attron(COLOR_PAIR(2));
    }

    for(int i = 0; i < num_lines; i++)
    {
        mvaddch(start_y, start_x, ch);
        usleep(ANIM_DELAY);
        refresh();
        start_x += dir_c;
        start_y += dir_r;
    }

    if (has_colors()) {
        attroff(COLOR_PAIR(2));
        attron(COLOR_PAIR(1));
    }

    refresh();
}
