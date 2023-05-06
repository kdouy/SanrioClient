#include "../../jimmy/jimmy.h"

#include <memory>

class c_minecraft
{
public:
	Jclass klass();
	Jobj getMinecraft();
};

inline std::unique_ptr<c_minecraft> mc;