#include "snake.h"

Board::Board(Adafruit_ST7735* Disp) : disp(Disp) {
	mapHeight = disp->height() / 2;
	mapWidth = disp->height() / 2;
	map = new int[mapHeight][mapWidth];
}

void Board::apple() {
	// place a new apple, ensure not on snake body
	int x, y;
	while (map[y = rand() % mapHeight)][x = rand() % mapWidth] == SNAKE);
	map[y][x] = APPLE;
}

Snake& Snake operator++() {
	length++;
	return *this;
}

void Snake::setDir(int d) { dir = d; } // 0 - up, 1 - right, 2 - down, 3 - left

int Snake::move(int dir) {
	
	int y = &(snake->head->y), x = &(snake->head->x);

	if (dir == 0) {
		if ((y - 1 < 0) || (map[y - 1][x] == SNAKE))
			return GAMEOVER;
		map[y--][x] = SNAKE;
	}
	else if (dir == 1) {
		if ((x + 1 > mapWidth) || (map[head][x + 1] == SNAKE))
			return GAMEOVER;
		map[y][x++] = SNAKE;
	}
	else if (dir == 2) {
		if ((y + 1 > mapHeight) || (map[y + 1][x] == SNAKE))
			return GAMEOVER;
		map[y++][x] = SNAKE;
	}
	else {
		if ((x - 1 < 0) || (map[y][x - 1] == SNAKE))
			return GAMEOVER;
		map[y][x--] = SNAKE;
	}

	if (map[y][x] == APPLE) ++snake;
	else delete snake->tail;
}
