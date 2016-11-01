/*****************************************************************************
 * Copyright (C) Manasi Anil Ladkat manasiladkat@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL/SDL.h"
#include <limits.h>
#include "SDL/SDL_ttf.h"
#define MAX 128

#define SCREEN_WIDTH 1040
#define SCREEN_HEIGHT 570

#define BLOCK_SCREEN_BUFFER 40

#define MAX_BLOCKS 80 

#define BLUE_X           80
#define BLUE_Y           20

int BLOCK_WIDTH, BLOCK_HEIGHT, NUM_ROWS, NUM_COLS, cnt = 0, MINE_CNT, starttime,result = 0, gameover;
SDL_Rect menu;
SDL_Surface *surface, *mine, *block, *mines, *text, *textr, *textq, *flag;
FILE *fp;

void Countzero(int, int);
void Gameone(int, int);
void writetext(char *str, int fontsize, int x, int y, int w, int h, int r, int g, int b);
void flagtext(int i);
void topfive();
void printscore(int score, int x, int y);
void Startgame();
void playgame() ;
void makerect(SDL_Rect *rect, int x, int y, int w, int h);

typedef struct rect {
	SDL_Rect screen_location;
	int mine;
	int clk;
	int rtclk;
	int count;
}tile;

typedef struct mouse{
	int x;
	int y;
}mouse;

tile blocks[32][32];
int tileno = 0;

/* makes an array of 0s and 1s with a "mines" number of ones */  	
void makearray(int ar[], int tiles, int mines) {
	int r, i;
		for(i = 0; i < tiles; i++)
			ar[i] = 0;
		for(i = 0; i < mines; i++) {
			r = random() % tiles;
			if(ar[r] == 1)
				i--;
			else
				ar[r] = 1;
		}
}

/* searches for the highscore in given file */
int highscore(int score) {
	rewind(fp);
	int temp, flag = 0, hscore = score;
	
	while(fscanf(fp, "%d", &temp) != -1) {
		if(score == temp)
			flag = 1;		
		if(hscore > temp)
			hscore = temp;
	}

	if(!flag)
		fprintf(fp, "%d\n", score);
	
	return hscore;
}

/* makes the above bar for new, retry and exit buttons */ 	
void makemenu(int mines) {
	SDL_Surface *smile;
	menu.x = 0 ;
	menu.y = 0;
        menu.w = SCREEN_WIDTH;
        menu.h = 32;
	smile = SDL_LoadBMP("smile.bmp"); 
	SDL_FillRect(surface, &menu, SDL_MapRGB(surface->format, 128, 128, 128));
	SDL_SetColorKey(smile, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 0, 0, 0));	
	if(SDL_BlitSurface(smile, NULL, surface, &menu)!=0)
		printf("Unsuccessful blit%s", SDL_GetError());	
	writetext("New", 20, 50, 5, 50, 32, 0, 0, 0);
	writetext("Retry", 20, 140, 5, 50, 32, 0, 0, 0);
	writetext("Exit", 20, 255, 5, 50, 32, 0, 0, 0);
	writetext("Flags Marked:", 20, 450, 5, 50, 50, 0, 0, 0);
	flagtext(0);
	writetext("Time:   ", 20, 700, 5, 50, 50, 0, 0, 0);
}

/* displays the number of flags marksed on the menu bar */	
void flagtext(int i) {
	SDL_Rect f;
	f.x = 600;
	f.y = 5;
	f.w = 50;
 	f.h = 27;
	int mine;
	char number[6];
	SDL_FillRect(surface, &f, SDL_MapRGB(surface->format, 128, 128, 128));
	mine = i / 10;
	number[0] = mine + '0';
	mine = i % 10;
	number[1] = mine + '0';
	number[2] = '/';
	mine = MINE_CNT / 10;
	number[3] = mine + '0';
	mine = MINE_CNT % 10;
	number[4] = mine + '0';
	number[5] = '\0';
	writetext(number, 20, 600, 5, 50, 50, 0, 0, 0);
}

/* converts integer to array */
void inttostr(int n, char arr[]) {
	int count, i = 0, j;
	char temp;	
	do {
		arr[i++] = (n % 10) + 48;
		n /= 10;
	}while(n != 0);
	arr[i] = '\0';
	i--;
	for(j = 0; j <= i/2; j++) {
		temp = arr[j];
		arr[j] = arr[i - j];
		arr[i - j] = temp; 
	}
}

/* converts integer into a string such that it displays the integer in to digits */
void twotostr(int n, char arr[]) {
	int mine = 0;
	mine = n / 10;
	arr[0] = mine + '0';
	mine = n % 10;
	arr[1] = mine + '0';
	arr[2] = '\0';
}	
	
/* makes the grid of tiles and returns number of tiles */
int InitBlocks() {
	int i = 0, k = 0, row, col, mines, tiles, r;
	starttime = SDL_GetTicks();
	cnt = 0;
	time_t tt;
	srandom(time(&tt));
	tiles = NUM_ROWS * NUM_COLS;
	int arr[tiles];
	makemenu(mines);	
	switch(NUM_COLS) {
		case 8: mines = 10;
			break;
		case 16: mines = 40;	
			break;
		case 30: mines = 99;
			break;
		default: break;
	}
	makearray(arr, tiles, mines);
	i = 0;
	for (row=0; row < NUM_ROWS; row++) {
	        for (col=0; col < NUM_COLS; col++) {
				blocks[row][col].screen_location.x = (col)*BLOCK_WIDTH + BLOCK_SCREEN_BUFFER;
		                blocks[row][col].screen_location.y = (row)*BLOCK_HEIGHT + BLOCK_SCREEN_BUFFER;
		                blocks[row][col].screen_location.w = BLOCK_WIDTH;
		                blocks[row][col].screen_location.h = BLOCK_HEIGHT;
				blocks[row][col].mine = arr[i];
				blocks[row][col].rtclk = 0;
				blocks[row][col].clk = 0;
				i++;
		}
	}
	for (row=0; row < NUM_ROWS; row++) {
	        for (col=0; col < NUM_COLS; col++) {
		    	if(SDL_BlitSurface(mine, NULL, surface, &blocks[row][col].screen_location)!=0)
				printf("Unsuccessful blit%s", SDL_GetError());	
			blocks[row][col].count = minecount(row, col);	
		}
	}
	SDL_UpdateRect(surface, 0, 0, 0, 0);
	return i;
}

/* returns the coordinates of mouse clicked */ 
mouse mouseclk() {
	mouse m;
	SDL_Event event;
	SDL_PollEvent(&event);
	int x, y, i, row, col;
	x = y = i = 0;
	while (1) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			
				case SDL_QUIT:
					exit(0);
				break;
				
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							exit(0);
						break;
						
						default:
						break;
					}
				break;
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) 
                		if (event.button.button == SDL_BUTTON_LEFT) {	
					m.x = event.button.x;
					m.y = event.button.y;
					return m;
				}
		}
	}
}

void chooselevel() {
	mouse temp;
	SDL_Surface *begin, *inter, *exp;
	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));	
	begin = SDL_LoadBMP("beginner.bmp"); 
	SDL_SetColorKey(begin, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 255, 255, 255));
	SDL_Rect beginnerlocation = {SCREEN_WIDTH / 4, (SCREEN_HEIGHT / 4), 200, 100};	
	if(SDL_BlitSurface(begin, NULL, surface, &beginnerlocation) != 0)
		printf("Unsuccessful Blit%s", SDL_GetError());
	inter = SDL_LoadBMP("intermediate.bmp"); 
	SDL_SetColorKey(inter, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 255, 255, 255));
	SDL_Rect intermediatelocation = {SCREEN_WIDTH / 4, (SCREEN_HEIGHT / 4) + 150, 200, 100};	
	if(SDL_BlitSurface(inter, NULL, surface, &intermediatelocation) != 0)
		printf("Unsuccessful Blit%s", SDL_GetError());
	exp = SDL_LoadBMP("expert.bmp"); 
	SDL_SetColorKey(exp, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 255, 255, 255));
	SDL_Rect expertlocation = {SCREEN_WIDTH / 4, (SCREEN_HEIGHT / 2) + 150, 200, 100};	
	if(SDL_BlitSurface(exp, NULL, surface, &expertlocation) != 0)
		printf("Unsuccessful Blit%s", SDL_GetError());
	SDL_UpdateRect(surface, 0, 0, 0, 0);
	while(1) {			
		temp = mouseclk();
		/* according to level chosen the following tasks are carried out */
		if(temp.x > SCREEN_WIDTH / 4 && temp.x < (SCREEN_WIDTH / 4 + 200) && temp.y > (SCREEN_HEIGHT / 4) && temp.y < 			(SCREEN_HEIGHT / 4) + 100) { 
			MINE_CNT = 10; 
			mine = SDL_LoadBMP("mine1.bmp"); 
			block = SDL_LoadBMP("grey1.bmp");
			mines = SDL_LoadBMP("mines1.bmp");
			fp = fopen("scores1.txt", "a+");		
			SDL_SetColorKey(mines, SDL_SRCCOLORKEY, SDL_MapRGB(mines->format,255,255,255));
			flag = SDL_LoadBMP("flag1.bmp");
			BLOCK_WIDTH = 64;
			BLOCK_HEIGHT = 64;
			NUM_COLS = NUM_ROWS = 8;
			SDL_UpdateRect(surface, 0, 0, 0, 0);			
			break;
		}
		if(temp.x > SCREEN_WIDTH / 4 && temp.x < SCREEN_WIDTH / 4 + 200 && temp.y > (SCREEN_HEIGHT / 4) + 150 && temp.y < 			(SCREEN_HEIGHT / 4) + 250) { 
			MINE_CNT = 40;
			mine = SDL_LoadBMP("mine2.bmp"); 
			block = SDL_LoadBMP("grey2.bmp");
			mines = SDL_LoadBMP("mines2.bmp");
			fp = fopen("scores2.txt", "a+");			
			SDL_SetColorKey(mines, SDL_SRCCOLORKEY, SDL_MapRGB(mines->format,255,255,255));
			flag = SDL_LoadBMP("flag2.bmp");
			BLOCK_WIDTH = 32;
			BLOCK_HEIGHT = 32;
			NUM_COLS = NUM_ROWS = 16;
			SDL_UpdateRect(surface, 0, 0, 0, 0);			
			break;
		}
		if(temp.x > SCREEN_WIDTH / 4 && temp.x < (SCREEN_WIDTH / 4 + 200) && temp.y > (SCREEN_HEIGHT / 2) + 150 && temp.y < 			(SCREEN_HEIGHT / 2) + 250) { 
			MINE_CNT = 99;
			mine = SDL_LoadBMP("mine2.bmp"); 
			block = SDL_LoadBMP("grey2.bmp");
			mines = SDL_LoadBMP("mines2.bmp");	
			fp = fopen("scores3.txt", "a+");			
			SDL_SetColorKey(mines, SDL_SRCCOLORKEY, SDL_MapRGB(mines->format,255,255,255));
			flag = SDL_LoadBMP("flag2.bmp");
			BLOCK_WIDTH = 32;
			BLOCK_HEIGHT = 32;
			NUM_COLS = 30;
			NUM_ROWS = 16;
			SDL_UpdateRect(surface, 0, 0, 0, 0);			
			break;
		}
	}	
	playgame();
	SDL_FreeSurface(begin);
	SDL_FreeSurface(inter);
	SDL_FreeSurface(exp);

}

/* makes the start menu */
void Startgame() {
	int y = 110, direction = 1;
	double i = 1.25, k;
	k = i;
	mouse m, temp;
	SDL_Surface *quit, *start, *help, *hscr, *mine;
	SDL_Event event;
	SDL_Rect menu, play;
	makerect(&menu, 400, 450, 300, 50);
	makerect(&play, 750, 450, 200, 50);
	quit = SDL_LoadBMP("quit.bmp"); 
	start = SDL_LoadBMP("start.bmp"); 
	help = SDL_LoadBMP("help.bmp");
	hscr = SDL_LoadBMP("hscr.bmp"); 
	mine = SDL_LoadBMP("mines.bmp"); 
	SDL_SetColorKey(mine, SDL_SRCCOLORKEY, SDL_MapRGB(mine->format,0,0,0)); 
	SDL_Rect startlocation = {SCREEN_WIDTH - 300, 50, 200, 100};	
	SDL_Rect helplocation = {SCREEN_WIDTH - 300, 175, 200, 100};	
	SDL_Rect hscrlocation = {SCREEN_WIDTH - 300, 300, 200, 100};	
	SDL_Rect quitlocation = {SCREEN_WIDTH - 300, 425, 200, 100};	
	SDL_Rect minelocation = {200, y, 300, 300};
	TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSansBold.ttf", 64);
	if (font == NULL) {
		printf("font not initialized");
		exit (1);
	} 
	SDL_Color textcolor = {192, 192, 192};
        SDL_Rect textlocation = { 100, 50, 50, 200};	
	text = TTF_RenderText_Solid(font, "MINESWEEPER", textcolor);
	if(text == NULL) {
		printf("text not rendered");
		exit(1);
	}
	while(1) {	
		/* moves the mine up and down */
		if(direction == 1) {
			k += i;
			minelocation.y = y + k;	
			if(minelocation.y == 270) {
				direction = 0;
				k = 0;
			}
		}
		else if(direction == 0) {
			k += i;
			minelocation.y = 270 - k;	
			if(minelocation.y == 110) {
				direction = 1;
				k = 0;
			}
		}			
		if(SDL_BlitSurface(mine, NULL, surface, &minelocation))
			printf("Unsuccessful Blit%s", SDL_GetError());
		if(SDL_BlitSurface(start, NULL, surface, &startlocation) != 0)
			printf("Unsuccessful Blit%s", SDL_GetError());
		if(SDL_BlitSurface(help, NULL, surface, &helplocation) != 0)
			printf("Unsuccessful Blit%s", SDL_GetError());
		if(SDL_BlitSurface(hscr, NULL, surface, &hscrlocation) != 0)
			printf("Unsuccessful Blit%s", SDL_GetError());
		if(SDL_BlitSurface(quit, NULL, surface, &quitlocation) != 0)
			printf("Unsuccessful Blit%s", SDL_GetError());
		SDL_BlitSurface(text, NULL, surface, &textlocation);
		SDL_UpdateRect(surface, 0, 0, 0, 0);
		SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
		if(SDL_PollEvent(&event)) {
			switch (event.type) {
		
				case SDL_QUIT:
					exit(0);
					break;
				
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							exit(0);
							break;
						
						default:
							break;
					}
					break;
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) 
                		if (event.button.button == SDL_BUTTON_LEFT) {	
					m.x = event.button.x;
					m.y = event.button.y;
				}
		}
		if(m.x > startlocation.x && m.x < (startlocation.x + 200) && m.y > startlocation.y && m.y < (startlocation.y + 100) ) {
			chooselevel();
		}
		else if(m.x > quitlocation.x && m.x < (quitlocation.x + 200) && m.y > quitlocation.y && m.y < (quitlocation.y + 100) ) {
			exit(0);
			return;
		}
		else if(m.x > helplocation.x && m.x < (helplocation.x + 200) && m.y > helplocation.y && m.y < (helplocation.y + 100)) {
			SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
			writetext("1. Click on the different mines to open them.", 32, 100, 50, 50, 200, 128, 128, 128);
			writetext("2. Each opened block gives the number of mines around it.", 32, 100, 100, 50, 200, 128, 128, 128);
			writetext("3. RightClick to set flag on a particular mine.", 32, 100, 150, 50, 200, 128, 128, 128);
			writetext("4. Another rightclick removes the flag.", 32, 100, 200, 50, 250, 128, 128, 128);
			writetext("5. If you open a mine game is over.", 32, 100, 250, 50, 300, 128, 128, 128);
			writetext("6. Open all blocks excluding the mines to win the game.", 32, 100, 300, 50, 200, 128, 128, 128);
			SDL_FillRect(surface, &menu, SDL_MapRGB(surface->format, 128, 128, 128));
			writetext("RETURN TO MENU", 32, 400, 455, 300, 50, 0, 0, 0);
			SDL_FillRect(surface, &play, SDL_MapRGB(surface->format, 128, 128, 128));
			writetext("PLAY GAME", 32, 750, 455, 300, 50, 0, 0, 0);

			/* for returning to main menu or starting the game */
			while(1) {	
				temp = mouseclk();
				if(temp.x > menu.x && temp.x < (menu.x + 300) && temp.y > menu.y && temp.y < (menu.y + 50) ) {
					main();
					break;
				}
				if(temp.x > play.x && temp.x < (play.x + 200) && temp.y > play.y && temp.y < (play.y + 50) ) {
					chooselevel();
					break;
				}
			}	
						
		}

		/* displays the top five scores on clicking on highscores */
		else if(m.x > hscrlocation.x && m.x < (hscrlocation.x + 200) && m.y > hscrlocation.y && m.y < (hscrlocation.y + 100)) {
			SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
			topfive();			
		}
	}
	SDL_FreeSurface(hscr);
	SDL_FreeSurface(mine);
	SDL_FreeSurface(start);
	SDL_FreeSurface(quit);
	SDL_FreeSurface(help);
}

/* returns the number of mines around the tile */
int minecount(int row, int col) {
	int cnt = 0;
	cnt  = checkmine(row, col + 1, cnt);
	cnt  = checkmine(row, col - 1, cnt);
	cnt  = checkmine(row + 1, col - 1, cnt);
	cnt  = checkmine(row + 1, col, cnt); 
	cnt  = checkmine(row + 1, col + 1, cnt);
	cnt  = checkmine(row - 1, col - 1, cnt);	
	cnt  = checkmine(row - 1, col, cnt);
	cnt  = checkmine(row - 1, col + 1, cnt);
	return cnt;
}

/* checks if the tile is a mine where row and column are coordinates of the 2D array*/
int checkmine(int row, int col, int i) {
	if(row < 0 || col < 0 || row == NUM_ROWS || col == NUM_COLS)
		return i ;
	if(blocks[row][col].mine == 1) 
			i++;
	return i;
}

/* makes rectangle of given coordinates */
void makerect(SDL_Rect *rect, int x, int y, int w, int h) {
	rect->x = x;	
	rect->y = y;
	rect->w = w;
	rect->h = h;
}

/* displays the top five scores */	
void topfive() {
	FILE *fp1, *fp2, *fp3, *f[3]; 
	int a[5], temp, temp1, i, j, k;
	char *ch[3], text_str[MAX] = {'0'};
	ch[0] = "BEGINNERS";
	ch[1] = "INTERMEDIATE";
	ch[2] = "EXPERT";
	fp1 = fopen("scores1.txt", "a+");			
	fp2 = fopen("scores2.txt", "a+");			
	fp3 = fopen("scores3.txt", "a+");
	f[0] = fp1;
	f[1] = fp2;
	f[2] = fp3;	
	SDL_Rect menu, play;
	mouse m;
	makerect(&menu, 400, 450, 300, 50);
	makerect(&play, 750, 450, 200, 50);
	writetext("HIGHSCORES", 32, 400, 50, 50, 200, 128, 128, 128);
	for(k = 0; k < 3; k++) {
		writetext(ch[k], 24, 150 + ( k * 300), 150, 50, 200, 128, 128, 128);
		for(i = 0; i < 5; i++)
			a[i] = INT_MAX;
		while(fscanf(f[k], "%d", &temp) != -1) 
			for(i = 0; i < 5; i++) 
				if(temp < a[i]) 
					for(j = i; j < 5; j++) {
						temp1 = a[j];
						a[j] = temp;
						temp = temp1;
						}
		for(i = 0; i < 5; i++) {
			inttostr(i + 1, text_str);
			writetext(text_str, 24, 150 + ( k * 300), 200 + (50 * i), 50, 200, 128, 128, 128);
			writetext(".", 24, 170 + ( k * 300), 200 + (50 * i), 50, 200, 128, 128, 128);
			printscore(a[i], 200 + (300 * k), 200 + (50 * i));
		}
	}	
	SDL_FillRect(surface, &menu, SDL_MapRGB(surface->format, 128, 128, 128));
	writetext("RETURN TO MENU", 32, 400, 455, 300, 50, 0, 0, 0);
	SDL_FillRect(surface, &play, SDL_MapRGB(surface->format, 128, 128, 128));
	writetext("PLAY GAME", 32, 750, 455, 300, 50, 0, 0, 0);
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	while(1) {	
		m = mouseclk();
		if(m.x > menu.x && m.x < (menu.x + 300) && m.y > menu.y && m.y < (menu.y + 50) ) {
				main();
				return;
		}
		if(m.x > play.x && m.x < (play.x + 200) && m.y > play.y && m.y < (play.y + 50) ) {
				chooselevel();
				return;
		}
	}
}

/*prints the score at coordinates x and y */
void printscore(int score, int x, int y) {
	char text_str[MAX] = {'0'};
	twotostr(score / 60, text_str);
	writetext(text_str, 24, x, y, 100, 100, 128, 128, 128);  
	writetext(":", 24, x + 30, y, 100, 100, 128, 128, 128);  
	twotostr(score % 60, text_str);
	writetext(text_str, 24, x + 40, y, 100, 100, 128, 128, 128);  
}

/*if there are no mines around a tile the following function is carried out */								
void Countzero(int row, int col) {
	Gameone(row, col + 1);
	Gameone(row + 1, col); 
	Gameone(row + 1, col + 1);
	Gameone(row - 1, col + 1);
	Gameone(row - 1, col);
	Gameone(row, col - 1);
	Gameone(row + 1, col - 1);
	Gameone(row - 1, col - 1);	
}

/* colour of the number displayed for the number of mines around a tile is alloted here */
SDL_Color color(int i) {
	SDL_Color textcolor;
	switch(i) {
		case 1: textcolor.r = 0;
			textcolor.g = 0;
			textcolor.b = 0;
			break;
		case 2: textcolor.r = 255;
			textcolor.g = 0;
			textcolor.b = 157;
			break;
		case 3: textcolor.r = 0;
			textcolor.g = 0;
			textcolor.b = 255;
			break;
		case 4: textcolor.r = 0;
			textcolor.g = 255;
			textcolor.b = 0;
			break;
		case 5: textcolor.r = 255;
			textcolor.g = 255;
			textcolor.b = 0;
			break;
		case 6: textcolor.r = 51;
			textcolor.g = 255;
			textcolor.b = 255;
			break;
		case 7: textcolor.r = 255;
			textcolor.g = 153;
			textcolor.b = 255;
			break;
		case 8: textcolor.r = 255;
			textcolor.g = 153;
			textcolor.b = 153;
			break;
	}      
	return textcolor; 
}	

/* if no mine around the tile the surrounding tiles are opened and marked clicked */
void Gameone(int row, int col) {
	SDL_Color textcolor;
	if(row < 0 || col < 0 || row == NUM_ROWS || col == NUM_COLS)
		return;
	if(blocks[row][col].clk == 1 || blocks[row][col].mine == 1)
		return;
	blocks[row][col].clk = 1;
	cnt++;
	char number[2];
	if(SDL_BlitSurface(block, NULL, surface, &blocks[row][col].screen_location)!=0)
		printf("Unsuccessful blit%s", SDL_GetError());
	SDL_UpdateRect(surface, blocks[row][col].screen_location.x, blocks[row][col].screen_location.y, BLOCK_WIDTH, BLOCK_HEIGHT);
	if(blocks[row][col].count == 0) {
		Countzero(row, col);
                return;
        }
	number[0] = blocks[row][col].count + '0';
	number[1] = '\0';
	TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSansBold.ttf", BLOCK_WIDTH / 2);
	if (font == NULL) {
		printf("font not initialized");
		exit (1);
	}
	textcolor = color(blocks[row][col].count);
       	SDL_Rect textlocation = {blocks[row][col].screen_location.x + (BLOCK_WIDTH / 4) + 5, blocks[row][col].screen_location.y + 		(BLOCK_HEIGHT / 4), 200, 200};				
	text = TTF_RenderText_Solid(font, number, textcolor);
        SDL_BlitSurface(text, NULL, surface, &textlocation);
	SDL_UpdateRect(surface, 0, 0, 0, 0);
	return;	
}

/* opens the tile and writes number of mines around the tile in it */
void Game(int row, int col, int tiles) {
	SDL_Color textcolor = {255, 255, 255};
	char number[2];
	cnt++;
	if(SDL_BlitSurface(block, NULL, surface, &blocks[row][col].screen_location)!=0) 
		printf("Unsuccessful blit%s", SDL_GetError());
	SDL_UpdateRect(surface, blocks[row][col].screen_location.x, blocks[row][col].screen_location.y, BLOCK_WIDTH, BLOCK_HEIGHT);
	if(blocks[row][col].count == 0 ) {
		Countzero(row, col);
                return;
        }
	number[0] = blocks[row][col].count + '0';
	number[1] = '\0';
	TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSansBold.ttf", BLOCK_WIDTH / 2);
	if (font == NULL) {
		printf("font not initialized");
		exit (1);
	}
	textcolor = color(blocks[row][col].count);
	SDL_Rect textlocation = {blocks[row][col].screen_location.x + (BLOCK_WIDTH / 4) + 5, blocks[row][col].screen_location.y + 		(BLOCK_HEIGHT / 4), 200, 200};				
	text = TTF_RenderText_Solid(font, number, textcolor);
        SDL_BlitSurface(text, NULL, surface, &textlocation);
	SDL_UpdateRect(surface, 0, 0, 0, 0);
	return;	
}

/* when mine is clicked all mines are opened and game is declared as over. Also smiley at the corner changes to sad*/
void GameLost(int tiles) {
	SDL_Surface *smile;
	int x, y, row, col;
	SDL_Color back = {196, 196, 196};
	SDL_Event ev;	
	int game = 0;
	for (row=0; row < NUM_ROWS; row++) 
	        for (col=0; col < NUM_COLS; col++) {
			blocks[row][col].clk = 1; 
			if(blocks[row][col].mine == 1){
				if(SDL_BlitSurface(block, NULL, surface, &blocks[row][col].screen_location)!=0)
						printf("Unsuccessful blit%s", SDL_GetError());
				SDL_SetColorKey(mines, SDL_SRCCOLORKEY, SDL_MapRGB(mines->format,0,0,0)); 
				if(SDL_BlitSurface(mines, NULL, surface, &blocks[row][col].screen_location)!=0)
						printf("Unsuccessful blit%s", SDL_GetError());
			}
		}
	writetext("YOU LOST!!!", 64,blocks[1][1].screen_location.x, blocks[1][0].screen_location.y, 200, 200, 0, 0, 0); 
	smile = SDL_LoadBMP("sad.bmp"); 
	SDL_SetColorKey(smile, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 255, 255, 255));	
	if(SDL_BlitSurface(smile, NULL, surface, &menu)!=0)
		printf("Unsuccessful blit%s", SDL_GetError());	
}

/* writes the string at given location x, y, z and given rgb colourformat */
void writetext(char *str, int fontsize, int x, int y, int w, int h, int r, int g, int b) {
	TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSansBold.ttf", fontsize);
	if (font == NULL) {
		printf("font not initialized");
		exit (1);
	} 
	SDL_Color textcolor = {r, g, b};
        SDL_Rect textlocation = { x, y, w, h};	
	text = TTF_RenderText_Solid(font, str, textcolor);
	if(text == NULL) {
		printf("text not rendered");
		exit(1);
	}
	SDL_BlitSurface(text, NULL, surface, &textlocation);
	SDL_UpdateRect(surface, 0, 0, 0, 0);
}

/* after all tiles are opened game is declared as won and smiley at corner changes to happy */
void GameWon(int tiles) {
	SDL_Surface *smile;
	smile = SDL_LoadBMP("happy.bmp"); 
	SDL_SetColorKey(smile, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 0, 0, 0));	
	if(SDL_BlitSurface(smile, NULL, surface, &menu)!=0)
		printf("Unsuccessful blit%s", SDL_GetError());	
	writetext("YOU WON!!!", 64,blocks[1][1].screen_location.x, blocks[1][0].screen_location.y, 200, 200, 0, 0, 0); 
}	

/* the game initiates as grid is created */
 void playgame() {
	int x = 0, y = 0, row, col, right = 0, count = 0, presenttime, minute, second, i = 0, tiles= 0, flags = 0, hscore;
	text = NULL;
	char time_str[MAX] = {'0'}, hscore_str[MAX] = {'0'}, text_str[MAX] = {'0'}, h_str[MAX] = {'0'};
	gameover = 1;
	SDL_Event event;
	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));			
	tiles = InitBlocks();	
	SDL_Rect m, sec, colon;
	makerect(&m, 760, 5, 20, 27);
	makerect(&sec, 795, 5, 20, 27);
	makerect(&colon, 785, 5, 10, 10);
	SDL_Color textcolor = {0, 0, 0};
	TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSansBold.ttf", 18);
	while (1) {	
		/* displays time at the top right corner */
		if(gameover) {
			presenttime=SDL_GetTicks();
			result=presenttime-starttime;
			result=result/1000;
			minute = result / 60;
			second = result % 60;
			SDL_FillRect(surface, &m, SDL_MapRGB(surface->format, 128, 128, 128));
			twotostr(minute, time_str);
			text = TTF_RenderText_Solid(font, time_str, textcolor);
			if(text == NULL) {
				printf("text not rendered");
				exit(1);
			}
			SDL_BlitSurface(text, NULL, surface, &m);
			SDL_FillRect(surface, &sec, SDL_MapRGB(surface->format, 128, 128, 128));
			twotostr(second, time_str);
			text = TTF_RenderText_Solid(font, time_str, textcolor);
			if(text == NULL) {
				printf("text not rendered");
				exit(1);
			}
			SDL_BlitSurface(text, NULL, surface, &sec);
			SDL_FillRect(surface, &colon, SDL_MapRGB(surface->format, 128, 128, 128));
			text = TTF_RenderText_Solid(font, ":", textcolor);
			if(text == NULL) {
				printf("text not rendered");
				exit(1);
			}
			SDL_BlitSurface(text, NULL, surface, &colon);
			SDL_UpdateRect(surface, 0, 0, 0, 0);
		}

		/* each click of the mouse is monitored on whereever the mouse click is done */
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					exit(0);
				break;
				
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							exit(0);
						break;
						
						default:
						break;
					}
				break;
			}		
			if (event.type == SDL_MOUSEBUTTONDOWN) {
		                if (event.button.button == SDL_BUTTON_LEFT) {	
					x = event.button.x;
					y = event.button.y;
					if(x > 50 && x < 100 && y > 5 && y < 32) 
						x = main();
					if(x > 155 && x < 255 && y > 5 && y < 32) {
						gameover = 1;
						flags = 0;
						InitBlocks();
					}
					if(x > 255 && x < 305 && y > 5 && y < 32) 
						exit(0);
					for (row=0; row < NUM_ROWS; row++) 
					        for (col=0; col < NUM_COLS; col++) 

							/* one of the tiles is clicked */
							if(x > blocks[row][col].screen_location.x && x < (blocks[row][col].screen_location.x + BLOCK_WIDTH) && y > blocks[row][col].screen_location.y && y < (blocks[row][col].screen_location.y + BLOCK_HEIGHT) && blocks[row][col].clk == 0 && blocks[row][col].rtclk == 0) {

								/* tile is marked as clicked */
								blocks[row][col].clk = 1;

								/* tile clicked is a mine */
								if(blocks[row][col].mine == 1) {
									gameover = 0;
									GameLost(tiles);
								}	

								/* tile clicked is not a mine */
								else if(blocks[row][col].mine == 0) {
									Game(row, col, tiles);

									/* all tiles exclduing mines are clicked */
									if(cnt == (tiles - MINE_CNT)) {
										gameover = 0;
										GameWon(tiles);
										SDL_Rect r;
										r.x = blocks[1][0].screen_location.x;
										r.y = blocks[1][0].screen_location.y + 100;
										r.w = BLOCK_WIDTH * NUM_COLS;
										r.h = 80;
										SDL_FillRect(surface, &r, SDL_MapRGB(surface->format, 128, 128, 128));	
										/* score is displayed */
										writetext("Your Score - ", 32, blocks			[1]										[1].screen_location.x, blocks[1][0].screen_location.y + 										100, 100, 100, 0, 0, 0);		
										twotostr(minute, text_str);
										writetext(text_str, 32, blocks[1][1].screen_location.x + 											200, blocks[1][0].screen_location.y + 100, 100, 100, 0, 0, 											0);  	
										writetext(":", 32, blocks[1][1].screen_location.x + 240, 											blocks[1][0].screen_location.y + 100, 100, 100, 0, 0, 											0);	
										twotostr(second, text_str);
										writetext(text_str, 32, blocks[1][1].screen_location.x + 											250, blocks[1][0].screen_location.y + 100, 100, 100, 0, 0, 											0);  	

										/* highscore is displayed */	
										hscore = highscore(result);
										inttostr(hscore, h_str);
										writetext("Highscore - ", 24, blocks[1]										[1].screen_location.x, blocks[1][0].screen_location.y + 										150, 100, 100, 0, 0, 0);
										twotostr(hscore / 60, text_str);
										writetext(text_str, 24, blocks[1][1].screen_location.x + 											150, blocks[1][0].screen_location.y + 150, 100, 100, 0, 0, 											0);  
										writetext(":", 24, blocks[1][1].screen_location.x + 180, 											blocks[1][0].screen_location.y + 150, 100, 100, 0, 0, 0);  
										twotostr(hscore % 60, text_str);
										writetext(text_str, 24, blocks[1][1].screen_location.x + 											190, blocks[1][0].screen_location.y + 150, 100, 100, 0, 0, 											0);  
									}
								}						
							}	
				}	
											
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_RIGHT) {	
					x = event.button.x;
					y = event.button.y;
					for (row=0; row < NUM_ROWS; row++) 
					        for (col=0; col < NUM_COLS; col++) 
					
							/* right click is at given tile of row, col */
							if(x > blocks[row][col].screen_location.x && x < (blocks        [row]							[col].screen_location.x + BLOCK_WIDTH) && y > blocks[row][col].screen_location.y 								&& y < (blocks[row][col].screen_location.y + BLOCK_HEIGHT)) {
	
								/* there is no previous flag and tile is not clicked */
								if(blocks[row][col].rtclk == 0 && blocks[row][col].clk == 0) {	
									SDL_BlitSurface(flag, NULL, surface, &blocks[row][col].screen_location);
									flags++;
									flagtext(flags);
									blocks[row][col].rtclk = 1;
								}

								/* rightclick is at flag marked tile */
								else if(blocks[row][col].rtclk == 1 && blocks[row][col].clk == 0) {
									SDL_BlitSurface(mine, NULL, surface, &blocks[row][col].screen_location);
									flags--;
									flagtext(flags);
									blocks[row][col].rtclk = 0;
								}

								else if(blocks[row][col].rtclk == 1 && blocks[row][col].clk == 0) {
									SDL_BlitSurface(block, NULL, surface, &blocks[row][col].screen_location);
									flags--;
									flagtext(flags);
									blocks[row][col].rtclk = 0;
								}					
							}
				}
			}
			SDL_UpdateRect(surface, 0, 0, 0, 0);
		}
	}				
}	

void initsdl() {
	if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)) {
		printf("Could not initialize SDL: %s.\n", SDL_GetError());
		exit(1);
	}	
	if(TTF_Init() == -1) {
		printf("Could not initialize ttf:%s.\n", SDL_GetError());
		exit(1);
	}
}

void quitsdl() {
	TTF_Quit();
	SDL_Quit();
	SDL_FreeSurface(mine);
	SDL_FreeSurface(block);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(mines);
	SDL_FreeSurface(text);
	SDL_FreeSurface(textr);
	SDL_FreeSurface(textq);
	SDL_FreeSurface(flag);
	fclose(fp);
	exit(0);
}
		
int main(int argc, char *argv[]) {
	if(argc == 2)
		if(strcmp(argv[1], "--help") == 0) {
			printf("usage: ./project\n No arguments required\n");
			return 0;
		}
	initsdl();
	char *s = {"MyMinesweeper"};
	surface = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SDL_HWPALETTE);
	if(surface == NULL) {
		printf("Couldn't set screen mode to 1040 x 720: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	SDL_WM_SetCaption(s, NULL);
	SDL_Surface* temp = SDL_LoadBMP("icon1.bmp");
	SDL_Surface* bg = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);
	
	Startgame();
	
	playgame();				

	quitsdl();
	exit(0);
	return 0;
}
	
