#ifndef _WIN32
#include <unistd.h>
void jmy_init(pid_t pid, const void* jvmbase);
#else
#include <windows.h>
#undef interface
void jmy_init(HANDLE process, const void* jvmbase);
#endif

typedef unsigned char Jbool;
typedef unsigned char Jbyte;
typedef unsigned short Jchar;
typedef short Jshort;
typedef int Jint;
typedef long Jlong;
typedef float Jfloat;
typedef double Jdouble;
typedef unsigned short Jid;
typedef const void* Jclass;
typedef const void* Jobj;

Jclass jmy_findclassfromcl(const char* clsname, Jobj classloader);
Jclass jmy_findclass(const char* clsname);

Jid jmy_getfieldid(Jclass klass, const char* name, const char* sig);

Jobj jmy_getstaticobjfield(Jclass klass, Jid fieldid);
#define DECLARE_GETSTATICFIELD(type) J##type jmy_getstatic##type##field(Jclass klass, Jid fieldid);
DECLARE_GETSTATICFIELD(bool)
DECLARE_GETSTATICFIELD(byte)
DECLARE_GETSTATICFIELD(char)
DECLARE_GETSTATICFIELD(short)
DECLARE_GETSTATICFIELD(int)
DECLARE_GETSTATICFIELD(long)
DECLARE_GETSTATICFIELD(float)
DECLARE_GETSTATICFIELD(double)

Jobj jmy_getobjfield(Jobj oop, Jid fieldid);
#define DECLARE_GETFIELD(type) J##type jmy_get##type##field(Jobj oop, Jid fieldid);
DECLARE_GETFIELD(bool)
DECLARE_GETFIELD(byte)
DECLARE_GETFIELD(char)
DECLARE_GETFIELD(short)
DECLARE_GETFIELD(int)
DECLARE_GETFIELD(long)
DECLARE_GETFIELD(float)
DECLARE_GETFIELD(double)

void jmy_setstaticobjfield(Jclass klass, Jid fieldid, Jobj value);
#define DECLARE_SETSTATICFIELD(type) void jmy_setstatic##type##field(Jclass klass, Jid fieldid, J##type value);
DECLARE_SETSTATICFIELD(bool)
DECLARE_SETSTATICFIELD(byte)
DECLARE_SETSTATICFIELD(char)
DECLARE_SETSTATICFIELD(short)
DECLARE_SETSTATICFIELD(int)
DECLARE_SETSTATICFIELD(long)
DECLARE_SETSTATICFIELD(float)
DECLARE_SETSTATICFIELD(double)

void jmy_setobjfield(Jobj oop, Jid fieldid, Jobj value);
#define DECLARE_SETFIELD(type) void jmy_set##type##field(Jobj oop, Jid fieldid, J##type value);
DECLARE_SETFIELD(bool)
DECLARE_SETFIELD(byte)
DECLARE_SETFIELD(char)
DECLARE_SETFIELD(short)
DECLARE_SETFIELD(int)
DECLARE_SETFIELD(long)
DECLARE_SETFIELD(float)
DECLARE_SETFIELD(double)

int jmy_isinstanceof(Jobj obj, Jclass klass);
Jclass jmy_getsuperclass(Jclass klass);
Jclass jmy_getobjectclass(Jobj oop);

int jmy_getarraylen(Jobj oop);
void jmy_getobjarrayelems(Jobj oop, Jobj* array, int index, int nbelem);
#define DECLARE_GETARRAYELEMS(type) void jmy_get##type##arrayelems(Jobj oop, J##type *array, int index, int nbelem);
DECLARE_GETARRAYELEMS(bool)
DECLARE_GETARRAYELEMS(byte)
DECLARE_GETARRAYELEMS(char)
DECLARE_GETARRAYELEMS(short)
DECLARE_GETARRAYELEMS(int)
DECLARE_GETARRAYELEMS(long)
DECLARE_GETARRAYELEMS(float)
DECLARE_GETARRAYELEMS(double)

void jmy_setobjarrayelems(Jobj oop, Jobj* array, int index, int nbelem);
#define DECLARE_SETARRAYELEMS(type) void jmy_set##type##arrayelems(Jobj oop, J##type *array, int index, int nbelem);
DECLARE_SETARRAYELEMS(bool)
DECLARE_SETARRAYELEMS(byte)
DECLARE_SETARRAYELEMS(char)
DECLARE_SETARRAYELEMS(short)
DECLARE_SETARRAYELEMS(int)
DECLARE_SETARRAYELEMS(long)
DECLARE_SETARRAYELEMS(float)
DECLARE_SETARRAYELEMS(double)

#define DECLARE_GETCP(type) J##type jmy_get##type##cp(Jclass klass, int index);
DECLARE_GETCP(int)
DECLARE_GETCP(long)
DECLARE_GETCP(float)
DECLARE_GETCP(double)

#define DECLARE_SETCP(type) void jmy_set##type##cp(Jclass klass, int index, J##type value);
DECLARE_SETCP(int)
DECLARE_SETCP(long)
DECLARE_SETCP(float)
DECLARE_SETCP(double)