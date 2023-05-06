#include "../minecraft.h"
#include "../player/player.h"
#include <vector>

class c_world
{
private:
	Jobj worldObj;

public:
	c_world();

	Jobj getWorldObj();
	bool isMultiplayer();
	int getPlayerCount();
	std::vector<std::shared_ptr<c_player>> getPlayers();
};

inline std::unique_ptr<c_world> world;