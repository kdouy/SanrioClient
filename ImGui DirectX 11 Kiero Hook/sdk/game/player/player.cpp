#include "../minecraft.h"
#include "player.h"

c_player::c_player() : playerObj(NULL)
{
	Jid fid = jmy_getfieldid(mc->klass(), "h", "Lbew;");
	playerObj = jmy_getobjfield(mc->getMinecraft(), fid);
}

c_player::c_player(Jobj initPlayerObj)
{
	playerObj = initPlayerObj;
}

Jobj c_player::getPlayerObj()
{
	return playerObj;
}

double c_player::getX()
{
	Jclass c = jmy_findclass("bew");
	Jid fid = jmy_getfieldid(c, "s", "D");
	return jmy_getdoublefield(playerObj, fid);
}

double c_player::getY()
{
	Jclass c = jmy_findclass("bew");
	Jid fid = jmy_getfieldid(c, "t", "D");
	return jmy_getdoublefield(playerObj, fid);
}

double c_player::getZ()
{
	Jclass c = jmy_findclass("bew");
	Jid fid = jmy_getfieldid(c, "u", "D");
	return jmy_getdoublefield(playerObj, fid);
}