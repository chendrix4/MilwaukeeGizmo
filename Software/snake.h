#ifndef snake_h
#define snake_h

#include "Arduino.h"
#include "Adafruit_ST7735a.h"

// map parts
#define APPLE -1
#define SNAKE 1

// events
#define GAMEOVER -1

struct node {
	int y, x;
	node* next;
};

class Snake {
public:
	Snake(int y, int x);
	int move(int dir);
	void growSnake(int y, int x);

private:
	node *head, *tail;
	int length;
};

class Board {
public:
	Board(Adafruit_ST7735* Disp);
	void apple();
private:
	Adafruit_ST7735* disp;
	Snake* snake;
	int* map;
};


#endif