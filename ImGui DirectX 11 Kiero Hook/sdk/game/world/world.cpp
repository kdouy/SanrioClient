#include "world.h"
#include <iostream>

c_world::c_world()
{
	Jid fid = jmy_getfieldid(mc->klass(), "f", "Lbdb;");
	worldObj = jmy_getobjfield(mc->getMinecraft(), fid);
}

Jobj c_world::getWorldObj()
{
	return worldObj;
}

bool c_world::isMultiplayer()
{
	Jclass klass = jmy_findclass("bdb");
	Jid fid = jmy_getfieldid(klass, "j", "Ljava/util/List;");
	Jclass listklass = jmy_findclass("java/util/List");

	std::vector<std::shared_ptr<c_player>> res;
	Jobj playerEntities = jmy_getobjfield(worldObj, fid);

	if (!playerEntities)
		return false;

	return true;
}

int c_world::getPlayerCount()
{
	Jclass klass = jmy_findclass("bdb");
	Jid fid = jmy_getfieldid(klass, "j", "Ljava/util/List;");
	Jclass listklass = jmy_findclass("java/util/List");

	std::vector<std::shared_ptr<c_player>> res;
	Jobj playerEntities = jmy_getobjfield(worldObj, fid);

	if (!playerEntities)
		res;

	Jclass arraylistklass = jmy_findclass("java/util/ArrayList");
	Jid elemsid = jmy_getfieldid(arraylistklass, "elementData", "[Ljava/lang/Object;");
	Jid sizeid = jmy_getfieldid(arraylistklass, "size", "I");

	int size = jmy_getintfield(playerEntities, sizeid);
	Jobj array = jmy_getobjfield(playerEntities, elemsid);
	Jobj* arrayplayerlist = (Jobj*)malloc(size * sizeof(Jobj));
	jmy_getobjarrayelems(array, arrayplayerlist, 0, size);

	return size;
}

std::vector<std::shared_ptr<c_player>> c_world::getPlayers()
{
	Jclass klass = jmy_findclass("bdb");
	Jid fid = jmy_getfieldid(klass, "j", "Ljava/util/List;");
	Jclass listklass = jmy_findclass("java/util/List");

	std::vector<std::shared_ptr<c_player>> res;
	Jobj playerEntities = jmy_getobjfield(worldObj, fid);

	if (!playerEntities)
		res;

	Jclass arraylistklass = jmy_findclass("java/util/ArrayList");
	Jid elemsid = jmy_getfieldid(arraylistklass, "elementData", "[Ljava/lang/Object;");
	Jid sizeid = jmy_getfieldid(arraylistklass, "size", "I");

	int size = jmy_getintfield(playerEntities, sizeid);
	Jobj array = jmy_getobjfield(playerEntities, elemsid);
	Jobj* arrayplayerlist = (Jobj*)malloc(size * sizeof(Jobj));
	jmy_getobjarrayelems(array, arrayplayerlist, 0, size);

	/*
	std::cout << "Class -> " << klass << std::endl;
	std::cout << "FID -> " << fid << std::endl;
	std::cout << "List -> " << listklass << std::endl;
	std::cout << "WorldObj -> " << worldObj << std::endl;
	std::cout << "World -> " << world << std::endl;
	std::cout << "Player Entities -> " << playerEntities << std::endl;
	std::cout << "LocalPlayer -> " << localPlayer << std::endl;
	std::cout << "ArrayList Class -> " << arraylistklass << std::endl;
	std::cout << "Elements Id -> " << elemsid << std::endl;
	std::cout << "Size -> " << size << std::endl;
	*/

	if (!arrayplayerlist)
		return res;

	for (int i = 0; i < size; i++)
	{
		;		Jobj player = arrayplayerlist[i];

		if (player != localPlayer->getPlayerObj())
			res.push_back(std::make_shared<c_player>(player));
	}

	free((void*)arrayplayerlist);

	return res;
}