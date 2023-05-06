#include "minecraft.h"

Jclass c_minecraft::klass()
{
	return jmy_findclass("ave");
}

Jobj c_minecraft::getMinecraft()
{
	Jid sfid = jmy_getfieldid(klass(), "S", "Lave;");

	return jmy_getstaticobjfield(klass(), sfid);
}