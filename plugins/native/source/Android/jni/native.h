#ifndef NATIVE_H
#define NATIVE_H

#include <gglobal.h>
#include <gevent.h>

typedef struct gnative_Class gnative_Class;
typedef struct gnative_Object gnative_Object;
typedef struct gnative_Function gnative_Function;
typedef struct gnative_Field gnative_Field;
typedef struct gnative_Constructor gnative_Constructor;

enum gnative_Type
{
	GNATIVE_TYPE_VOID,
	GNATIVE_TYPE_BOOLEAN,
	GNATIVE_TYPE_BYTE,
	GNATIVE_TYPE_CHAR,
	GNATIVE_TYPE_SHORT,
	GNATIVE_TYPE_INT,
	GNATIVE_TYPE_LONG,
	GNATIVE_TYPE_FLOAT,
	GNATIVE_TYPE_DOUBLE,
	GNATIVE_TYPE_STRING,
	GNATIVE_TYPE_OBJECT,
};

typedef union gnative_Value
{
	g_bool z;
	unsigned char b;
	unsigned short c;
	short s;
	int i;
	long long l;
	float f;
	double d;
	const char *str;
	gnative_Object *obj;
} gnative_Value;


#ifdef __cplusplus
extern "C" {
#endif
    
G_API int gnative_IsAvailable();

G_API void gnative_Init();
G_API void gnative_Cleanup();

G_API gnative_Class *gnative_ClassCreate(const char *className);

G_API const char *gnative_ClassGetName(gnative_Class *cls);
G_API const char *gnative_ObjectGetName(gnative_Object *obj);

G_API gnative_Constructor **gnative_ClassGetConstructors(gnative_Class *cls);
G_API gnative_Function **gnative_ClassGetStaticFunctionsByName(gnative_Class *cls, const char *name);
G_API gnative_Function **gnative_ObjectGetFunctionsByName(gnative_Object *obj, const char *name);
G_API gnative_Field **gnative_ClassGetFieldsByName(gnative_Class *cls, const char *name);
G_API gnative_Field **gnative_ObjectGetFieldsByName(gnative_Object *obj, const char *name);

G_API gnative_Value gnative_FunctionCall(gnative_Function *function, gnative_Object *object, gnative_Value *parameters);
G_API gnative_Value gnative_FunctionStaticCall(gnative_Function *function, gnative_Value *parameters);

G_API gnative_Type gnative_FunctionGetReturnType(gnative_Function *function);
G_API gnative_Type *gnative_FunctionGetParameterTypes(gnative_Function *function);
G_API int gnative_FunctionGetParameterCount(gnative_Function *function);

G_API gnative_Type gnative_FieldGetReturnType(gnative_Field *field);
G_API gnative_Value gnative_FieldGetValue(gnative_Field *field, gnative_Object *object);
G_API void gnative_FieldSetValue(gnative_Field *field, gnative_Object *object, gnative_Value *value);
G_API gnative_Value gnative_FieldGetStaticValue(gnative_Field *field);
G_API void gnative_FieldSetStaticValue(gnative_Field *field, gnative_Value *value);

G_API gnative_Type *gnative_ConstructorGetParameterTypes(gnative_Constructor *constructor);
G_API int gnative_ConstructorGetParameterCount(gnative_Constructor *constructor);

G_API gnative_Object *gnative_CreateObject(gnative_Constructor *constructor, gnative_Value *parameters);
G_API void gnative_DeleteObject(gnative_Object *obj);

G_API g_bool gnative_ClassIsInstance(gnative_Class *cls, gnative_Object *obj);
G_API g_bool gnative_ClassIsSubclass(const char *cls, const char *subclass);

G_API gnative_Class **gnative_FunctionGetParameterClasses(gnative_Function *function);
G_API gnative_Class **gnative_ConstructorGetParameterClasses(gnative_Constructor *constructor);

G_API gnative_Object *gnative_CreateProxy(const char *className, long data);

G_API void gnative_InvokeMethod(const char *methodName, int length, gnative_Value *values, int types[], long data);

G_API gnative_Object *gnative_CreateArray(gnative_Class *cls, gnative_Object **obj, int length);

G_API gnative_Object *gnative_CreatePrimitiveArray(const char *type, gnative_Value *values, int length);

#ifdef __cplusplus
}
#endif

#endif
