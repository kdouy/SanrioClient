#pragma once
#include "../../../jimmy/jimmy.h"

#include <memory>

class c_player
{
private:
	Jobj playerObj;
public:
	c_player();
	c_player(Jobj player);

	Jobj getPlayerObj();

	double getX();
	double getY();
	double getZ();
};

typedef std::shared_ptr<c_player> player;

inline std::unique_ptr<c_player> localPlayer;