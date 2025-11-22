#ifndef SAVE_H
#define SAVE_H

#include "game.h"

int save_game(const GameState *game, const char *filename);
int load_game(GameState *game, const char *filename);

#endif /* SAVE_H */

