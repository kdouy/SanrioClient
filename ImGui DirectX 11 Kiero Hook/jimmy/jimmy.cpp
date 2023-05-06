#include "jimmy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "mem.h"
#ifdef _WIN32
#include "jvmwin.h"
#else
#include "jvm.h"
#endif

#define LOADER(a) ((size_t)a >> 8) & 0x7fffffff
#define PTRMATH(a) (void *) ((uintptr_t) a)

const void* jvm;
const char* license = "MIT license - jimmy";

/* from java/lang/String */
static unsigned int
hashcode(const char* s)
{
	unsigned int h = 0;
	size_t n = strlen(s);
	while (n-- > 0) {
		h = 31 * h + *s++;
	}
	return h;
}

static Jobj
system_loader()
{
	Jobj systemCl;
	readmem(&systemCl, PTRMATH(jvm + JVM_SYSTEMCL), sizeof(systemCl));
	return systemCl;
}

static void*
findsym(const char* sym) {
	/* TODO clean this whole function */
	unsigned int seed;
	int index;
	unsigned int hash;
	void* symtab;
	unsigned int symtablen;
	void* hashmap;
	void* listelem;

	readmem(&seed, PTRMATH(jvm + JVM_SEED_SYMBOLTABLE), sizeof(seed));
	if (seed != 0) {
		/* TODO try to be less rude */
		exit(1);
	}

	hash = hashcode(sym);
	readmem(&symtab, PTRMATH(jvm + JVM_SYMBOLTABLE), sizeof(symtab));
	readmem(&symtablen, symtab, sizeof(symtablen));
	readmem(&hashmap, PTRMATH(symtab + 8), sizeof(hashmap));
	if (symtablen == 0) {
		/* TODO try to be less rude */
		exit(1);
	}
	index = hash % symtablen;

	readmem(&listelem, PTRMATH(hashmap + index * 8), sizeof(listelem));
	while (listelem != NULL) {
		unsigned int hashelem;
		readmem(&hashelem, listelem, sizeof(hashelem));
		if (hashelem == hash) {
			void* symbol;
			readmem(&symbol, PTRMATH(listelem + 0x10), sizeof(symbol));
			unsigned short symlen;
			readmem(&symlen, symbol, sizeof(symlen));
			if (symlen == strlen(sym)) {
				void* buf = malloc(symlen);
				readmem(buf, PTRMATH(symbol + 8), symlen);
				if (memcmp(buf, sym, symlen) == 0) {
					free(buf);
					return symbol;
				}
				free(buf);
			}
		}
		readmem(&listelem, PTRMATH(listelem + 8), sizeof(listelem));
	}
	return NULL;
}

static Jobj
decodeoop(Jobj oop)
{
	char flag;
	void* base;
	int shift;
	uintptr_t decodedoop = (uintptr_t)oop;

	if (JVM_USE_COMPRESSED_OOPS) {
		readmem(&flag, PTRMATH(jvm + JVM_USE_COMPRESSED_OOPS), sizeof(flag));
		if (flag) {
			readmem(&base, PTRMATH(jvm + JVM_COMPRESSED_OOPS_BASE), sizeof(base));
			readmem(&shift, PTRMATH(jvm + JVM_COMPRESSED_OOPS_SHIFT), sizeof(shift));
			decodedoop <<= shift;
			decodedoop += (uintptr_t)base;
		}
	}
	return (Jobj)decodedoop;
}

static Jobj
encodeoop(Jobj oop)
{
	char flag;
	void* base;
	int shift;
	uintptr_t encodedoop = (uintptr_t)oop;

	if (JVM_USE_COMPRESSED_OOPS) {
		readmem(&flag, PTRMATH(jvm + JVM_USE_COMPRESSED_OOPS), sizeof(flag));
		if (flag) {
			readmem(&base, PTRMATH(jvm + JVM_COMPRESSED_OOPS_BASE), sizeof(base));
			readmem(&shift, PTRMATH(jvm + JVM_COMPRESSED_OOPS_SHIFT), sizeof(shift));
			encodedoop -= (uintptr_t)base;
			encodedoop >>= shift;
		}
	}
	return (Jobj)encodedoop;
}

static Jclass
decodeklass(Jclass klass)
{
	char flag;
	void* base;
	int shift;
	uintptr_t decodedklass = (uintptr_t)klass;

	if (JVM_USE_COMPRESSED_CLASS_POINTERS) {
		readmem(&flag, PTRMATH(jvm + JVM_USE_COMPRESSED_CLASS_POINTERS), sizeof(flag));
		if (flag) {
			readmem(&base, PTRMATH(jvm + JVM_COMPRESSED_CLASS_POINTERS_BASE), sizeof(base));
			readmem(&shift, PTRMATH(jvm + JVM_COMPRESSED_CLASS_POINTERS_SHIFT), sizeof(shift));
			decodedklass <<= shift;
			decodedklass += (uintptr_t)base;
		}
	}
	return (Jclass)decodedklass;
}

static Jclass
encodeklass(Jclass klass)
{
	char flag;
	void* base;
	int shift;
	uintptr_t encodedklass = (uintptr_t)klass;

	if (JVM_USE_COMPRESSED_CLASS_POINTERS) {
		readmem(&flag, PTRMATH(jvm + JVM_USE_COMPRESSED_CLASS_POINTERS), sizeof(flag));
		if (flag) {
			readmem(&base, PTRMATH(jvm + JVM_COMPRESSED_CLASS_POINTERS_BASE), sizeof(base));
			readmem(&shift, PTRMATH(jvm + JVM_COMPRESSED_CLASS_POINTERS_SHIFT), sizeof(shift));
			encodedklass -= (uintptr_t)base;
			encodedklass >>= shift;
		}
	}
	return (Jclass)encodedklass;
}

Jclass
jmy_findclassfromcl(const char* clsname, Jobj classloader)
{
	void* sym, * loaderdata, * dict, * dictht, * entry;
	Jclass klass;
	unsigned int hash, index, dictlen, entryhash;

	sym = findsym(clsname);
	if (sym == NULL)
		return NULL;
	readmem(&hash, PTRMATH(sym + 4), sizeof(hash));
	if (classloader != NULL) {
		readmem(&loaderdata, classloader, sizeof(loaderdata));
		hash ^= LOADER(loaderdata);
	}
	readmem(&dict, PTRMATH(jvm + JVM_SYSTEMDICTIONARY), sizeof(dict));
	readmem(&dictlen, dict, sizeof(dictlen));
	if (dictlen == 0) {
		/* TODO try to be less rude */
		exit(1);
	}
	readmem(&dictht, PTRMATH(dict + 8), sizeof(dictht));
	index = hash % dictlen;

	readmem(&entry, PTRMATH(dictht + index * 8), sizeof(entry));
	while (entry != NULL) {
		readmem(&entryhash, entry, sizeof(entryhash));
		if (entryhash == hash) {
			readmem(&klass, PTRMATH(entry + 0x10), sizeof(klass));
			return klass;
		}
		readmem(&entry, PTRMATH(entry + 8), sizeof(entry));
	}
	return NULL;
}

Jclass
jmy_findclass(const char* clsname)
{
	return jmy_findclassfromcl(clsname, system_loader());
}

static Jid
findlocalfield(const void* klass, const void* namesym, const void* sigsym)
{
	void* constants, * fields, * field, * fieldname, * fieldsig;
	unsigned short fieldscnt, nameidx, sigidx, i;
	Jid offset;

	readmem(&constants, PTRMATH(klass + JVM_KLASS_CONSTANTS), sizeof(constants));
	readmem(&fields, PTRMATH(klass + JVM_KLASS_FIELDS), sizeof(fields));
	readmem(&fieldscnt, PTRMATH(klass + JVM_KLASS_FIELDS_COUNT), sizeof(fieldscnt));
	fields = PTRMATH(fields + 4);
	for (i = 0; i < fieldscnt; i++) {
		field = PTRMATH(fields + i * 6 * sizeof(unsigned short));
		readmem(&nameidx, PTRMATH(field + sizeof(unsigned short)), sizeof(nameidx));
		readmem(&sigidx, PTRMATH(field + 2 * sizeof(unsigned short)), sizeof(sigidx));
		readmem(&fieldname, PTRMATH(constants + JVM_CP_BASE + nameidx * 8), sizeof(fieldname));
		readmem(&fieldsig, PTRMATH(constants + JVM_CP_BASE + sigidx * 8), sizeof(fieldsig));
		if (fieldname == namesym && fieldsig == sigsym) {
			readmem(&offset, PTRMATH(field + 5 * sizeof(unsigned short)), sizeof(unsigned short));
			offset <<= sizeof(unsigned short);
			readmem(&offset, PTRMATH(field + 4 * sizeof(unsigned short)), sizeof(unsigned short));
			offset >>= 2;
			return offset;
		}
	}
	return 0;
}

static Jid
findinterfacefield(Jclass klass, const void* namesym, const void* sigsym)
{
	void* interfaces = NULL, * interface;
	int interfaceslen, i;
	Jid offset;

	readmem(&interfaces, PTRMATH(klass + JVM_KLASS_INTERFACES), sizeof(interfaces));
	if (!interfaces)
		return 0;
	readmem(&interfaceslen, interfaces, sizeof(interfaceslen));
	interfaces = PTRMATH(interfaces + 8);
	for (i = 0; i < interfaceslen; i++) {
		readmem(&interface, PTRMATH(interfaces + i * sizeof(interface)), sizeof(interface));
		if ((offset = findlocalfield(interface, namesym, sigsym)))
			return offset;
		if ((offset = findinterfacefield(interface, namesym, sigsym)))
			return offset;
	}
	return 0;
}

static Jid
findfield(Jclass klass, const void* namesym, const void* sigsym)
{
	void* super = NULL;
	Jid offset;

	if (!(offset = findlocalfield(klass, namesym, sigsym))) {
		if (!(offset = findinterfacefield(klass, namesym, sigsym))) {
			readmem(&super, PTRMATH(klass + JVM_KLASS_SUPER), sizeof(super));
			if (super) {
				offset = findfield(super, namesym, sigsym);
			}
		}
	}
	return offset;
}

Jid
jmy_getfieldid(Jclass klass, const char* name, const char* sig)
{
	void* namesym, * sigsym;

	namesym = findsym(name);
	sigsym = findsym(sig);
	if (namesym == NULL || sigsym == NULL)
		return 0;
	return findfield(klass, namesym, sigsym);
}

Jobj
jmy_getstaticobjfield(Jclass klass, Jid fieldid)
{
	void* classoop;
	Jobj value = NULL;

	if (fieldid == 0)
		return 0;
	readmem(&classoop, PTRMATH(klass + JVM_KLASS_JAVAMIRROR), sizeof(classoop));
	readmem(&value, PTRMATH(classoop + fieldid), sizeof(unsigned int));
	value = decodeoop(value);
	return value;
}

#define DEFINE_GETSTATICFIELD(type) \
J##type \
jmy_getstatic##type##field(Jclass klass, Jid fieldid)\
{\
	void *classoop;\
	J##type value;\
	\
	if(fieldid == 0)\
	return 0;\
	readmem(&classoop, PTRMATH(klass + JVM_KLASS_JAVAMIRROR), sizeof(classoop));\
	readmem(&value, PTRMATH(classoop + fieldid), sizeof(value));\
	return value;\
}

DEFINE_GETSTATICFIELD(bool)
DEFINE_GETSTATICFIELD(byte)
DEFINE_GETSTATICFIELD(char)
DEFINE_GETSTATICFIELD(short)
DEFINE_GETSTATICFIELD(int)
DEFINE_GETSTATICFIELD(long)
DEFINE_GETSTATICFIELD(float)
DEFINE_GETSTATICFIELD(double)

Jobj
jmy_getobjfield(Jobj oop, Jid fieldid)
{
	Jobj value = NULL;

	if (fieldid == 0)
		return 0;
	readmem(&value, PTRMATH(oop + fieldid), sizeof(unsigned int));
	value = decodeoop(value);
	return value;
}

#define DEFINE_GETFIELD(type) \
J##type \
jmy_get##type##field(Jobj oop, Jid fieldid)\
{\
	J##type value;\
	\
	if(fieldid == 0)\
		return 0;\
	readmem(&value, PTRMATH(oop + fieldid), sizeof(value));\
	return value;\
}

DEFINE_GETFIELD(bool)
DEFINE_GETFIELD(byte)
DEFINE_GETFIELD(char)
DEFINE_GETFIELD(short)
DEFINE_GETFIELD(int)
DEFINE_GETFIELD(long)
DEFINE_GETFIELD(float)
DEFINE_GETFIELD(double)

void
jmy_setstaticobjfield(Jclass klass, Jid fieldid, Jobj value)
{
	void* classoop;

	if (fieldid == 0)
		return;
	readmem(&classoop, PTRMATH(klass + JVM_KLASS_JAVAMIRROR), sizeof(classoop));
	value = encodeoop(value);
	writemem(&value, PTRMATH(classoop + fieldid), sizeof(unsigned int));
}

#define DEFINE_SETSTATICFIELD(type) \
void \
jmy_setstatic##type##field(Jclass klass, Jid fieldid, J##type value)\
{\
	void *classoop;\
	\
	if(fieldid == 0)\
		return;\
	readmem(&classoop, PTRMATH(klass + JVM_KLASS_JAVAMIRROR), sizeof(classoop));\
	writemem(&value, PTRMATH(classoop + fieldid), sizeof(value));\
}

DEFINE_SETSTATICFIELD(bool)
DEFINE_SETSTATICFIELD(byte)
DEFINE_SETSTATICFIELD(char)
DEFINE_SETSTATICFIELD(short)
DEFINE_SETSTATICFIELD(int)
DEFINE_SETSTATICFIELD(long)
DEFINE_SETSTATICFIELD(float)
DEFINE_SETSTATICFIELD(double)

void
jmy_setobjfield(Jobj oop, Jid fieldid, Jobj value)
{
	if (fieldid == 0)
		return;
	value = encodeoop(value);
	writemem(&value, PTRMATH(oop + fieldid), sizeof(unsigned int));
}

#define DEFINE_SETFIELD(type) \
void \
jmy_set##type##field(Jobj oop, Jid fieldid, J##type value)\
{\
	if(fieldid == 0)\
		return;\
	writemem(&value, PTRMATH(oop + fieldid), sizeof(value));\
}

DEFINE_SETFIELD(bool)
DEFINE_SETFIELD(byte)
DEFINE_SETFIELD(char)
DEFINE_SETFIELD(short)
DEFINE_SETFIELD(int)
DEFINE_SETFIELD(long)
DEFINE_SETFIELD(float)
DEFINE_SETFIELD(double)

int
jmy_isinstanceof(Jobj obj, Jclass klass)
{
	/* TODO: Should me recoded, it isn't exactly the same as hotspot's one */
	Jclass objklass = NULL, * super = NULL;
	void* supers;
	unsigned int off;
	int superscnt = 0, i;

	if (obj == NULL)
		return 0;
	objklass = jmy_getobjectclass(obj);
	readmem(&off, PTRMATH(objklass + JVM_KLASS_SUPER_OFFSET), sizeof(off));
	readmem(&super, PTRMATH(objklass + off), sizeof(super));
	if (super == klass)
		return 1;
	if (objklass == klass)
		return 1;
	readmem(&supers, PTRMATH(objklass + JVM_KLASS_SEC_SUPERS), sizeof(supers));
	readmem(&superscnt, PTRMATH(supers), sizeof(superscnt));
	supers = PTRMATH(supers + 8);
	for (i = 0; i < superscnt; i++) {
		readmem(&super, PTRMATH(supers + i * sizeof(super)), sizeof(super));
		if (super == klass)
			return 1;
	}
	readmem(&super, PTRMATH(objklass + JVM_KLASS_SUPER), sizeof(super));
	for (i = 0; super; i++) {
		if (super == klass)
			return 1;
		readmem(&super, PTRMATH(super + JVM_KLASS_SUPER), sizeof(super));
	}
	return 0;
}

Jclass
jmy_getsuperclass(Jclass klass)
{
	Jclass super;

	readmem(&super, PTRMATH(klass + JVM_KLASS_SUPER), sizeof(super));
	return super;
}

Jclass
jmy_getobjectclass(Jobj oop)
{
	char flag;
	Jclass klass;

	readmem(&flag, PTRMATH(jvm + JVM_USE_COMPRESSED_CLASS_POINTERS), sizeof(flag));
	readmem(&klass, PTRMATH(oop + JVM_OOP_KLASS), flag ? sizeof(unsigned int) : sizeof(klass));
	if (flag)
		klass = decodeklass(klass);
	return klass;
}

int
jmy_getarraylen(Jobj oop)
{
	char flag;
	int len, offset;

	readmem(&flag, PTRMATH(jvm + JVM_USE_COMPRESSED_CLASS_POINTERS), sizeof(flag));
	offset = flag ? 0xC : 0x10;
	readmem(&len, PTRMATH(oop + offset), sizeof(len));
	return len;
}

void
jmy_getobjarrayelems(Jobj oop, Jobj* array, int index, int nbelem)
{
	/* 64 bits only ? */
	char flag;
	int base, i, len;

	readmem(&flag, PTRMATH(jvm + JVM_USE_COMPRESSED_CLASS_POINTERS), sizeof(flag));
	base = flag ? 0x10 : 0x18;
	len = jmy_getarraylen(oop);
	nbelem += index;
	for (i = index; i < nbelem && i < len; i++) {
		array[i - index] = jmy_getobjfield(oop, base + i * sizeof(unsigned int));
	}
}

#define DEFINE_GETARRAYELEMS(type) \
void \
jmy_get##type##arrayelems(Jobj oop, J##type *array, int index, int nbelem)\
{\
	/* 64 bits only ? */ \
	char flag;\
	int base, len;\
\
	readmem(&flag, PTRMATH(jvm + JVM_USE_COMPRESSED_CLASS_POINTERS), sizeof(flag));\
	base = flag ? 0x10 : 0x18;\
	len = jmy_getarraylen(oop);\
	if (index < 0 || index >= len)\
		return;\
	if (nbelem + index > len)\
		nbelem = len - index;\
	readmem(array, PTRMATH(oop + base + index * sizeof(*array)), nbelem * sizeof(*array));\
}

DEFINE_GETARRAYELEMS(bool)
DEFINE_GETARRAYELEMS(byte)
DEFINE_GETARRAYELEMS(char)
DEFINE_GETARRAYELEMS(short)
DEFINE_GETARRAYELEMS(int)
DEFINE_GETARRAYELEMS(long)
DEFINE_GETARRAYELEMS(float)
DEFINE_GETARRAYELEMS(double)

void
jmy_setobjarrayelems(Jobj oop, Jobj* array, int index, int nbelem)
{
	/* 64 bits only ? */
	char flag;
	int base, i, len;

	readmem(&flag, PTRMATH(jvm + JVM_USE_COMPRESSED_CLASS_POINTERS), sizeof(flag));
	base = flag ? 0x10 : 0x18;
	len = jmy_getarraylen(oop);
	nbelem += index;
	for (i = index; i < nbelem && i < len; i++) {
		jmy_setobjfield(oop, base + i * sizeof(unsigned int), array[i - index]);
	}
}

#define DEFINE_SETARRAYELEMS(type) \
void \
jmy_set##type##arrayelems(Jobj oop, J##type *array, int index, int nbelem)\
{\
	/* 64 bits only ? */ \
	char flag;\
	int base, len;\
\
	readmem(&flag, PTRMATH(jvm + JVM_USE_COMPRESSED_CLASS_POINTERS), sizeof(flag));\
	base = flag ? 0x10 : 0x18;\
	len = jmy_getarraylen(oop);\
	if (index < 0 || index >= len)\
		return;\
	if (nbelem + index > len)\
		nbelem = len - index;\
	writemem(array, PTRMATH(oop + base + index * sizeof(*array)), nbelem * sizeof(*array));\
}

DEFINE_SETARRAYELEMS(bool)
DEFINE_SETARRAYELEMS(byte)
DEFINE_SETARRAYELEMS(char)
DEFINE_SETARRAYELEMS(short)
DEFINE_SETARRAYELEMS(int)
DEFINE_SETARRAYELEMS(long)
DEFINE_SETARRAYELEMS(float)
DEFINE_SETARRAYELEMS(double)

#define DEFINE_GETCP(type) \
J##type \
jmy_get##type##cp(Jclass klass, int index)\
{\
	void *constants;\
	J##type value;\
\
	readmem(&constants, PTRMATH(klass + JVM_KLASS_CONSTANTS), sizeof(constants));\
	constants = PTRMATH(constants + JVM_CP_BASE);\
	readmem(&value, PTRMATH(constants + index * sizeof(value)), sizeof(value));\
	return value;\
}

DEFINE_GETCP(int)
DEFINE_GETCP(long)
DEFINE_GETCP(float)
DEFINE_GETCP(double)

#define DEFINE_SETCP(type) \
void \
jmy_set##type##cp(Jclass klass, int index, J##type value)\
{\
	void *constants;\
\
	readmem(&constants, PTRMATH(klass + JVM_KLASS_CONSTANTS), sizeof(constants));\
	constants = PTRMATH(constants + JVM_CP_BASE);\
	writemem(&value, PTRMATH(constants + index * sizeof(value)), sizeof(value));\
}

DEFINE_SETCP(int)
DEFINE_SETCP(long)
DEFINE_SETCP(float)
DEFINE_SETCP(double)
