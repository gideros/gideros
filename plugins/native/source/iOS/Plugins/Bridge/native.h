#ifndef NATIVE_H
#define NATIVE_H

#ifndef OBJC_BOOL_DEFINED
typedef signed char BOOL;
#endif

#ifndef G_API
#define G_API
#endif

typedef struct gnative_Class gnative_Class;
typedef struct gnative_Object gnative_Object;
typedef struct gnative_Function gnative_Function;
typedef struct gnative_Constructor gnative_Constructor;

typedef enum gnative_Type
{
	GNATIVE_TYPE_VOID,
	GNATIVE_TYPE_BOOLEAN,
	GNATIVE_TYPE_CHAR,
	GNATIVE_TYPE_SHORT,
	GNATIVE_TYPE_INT,
	GNATIVE_TYPE_LONG,
	GNATIVE_TYPE_FLOAT,
	GNATIVE_TYPE_DOUBLE,
	GNATIVE_TYPE_LONG_DOUBLE,
	GNATIVE_TYPE_STRING,
	GNATIVE_TYPE_OBJECT,
    GNATIVE_TYPE_COMPLEX,
	GNATIVE_TYPE_JAVA_CHAR,
} gnative_Type;

typedef union gnative_Value
{
	BOOL z;
	char c;
	short s;
	int i;
	long long l;
	float f;
	double d;
	long double ld;
	const char *str;
	gnative_Object *obj;
    void *ptr;
	unsigned short jc;
} gnative_Value;

typedef void (*gnative_Implementation)(gnative_Object*, gnative_Function*, gnative_Value*);
typedef void (*gnative_StaticImplementation)(gnative_Class*, const char*, gnative_Value*);

#ifdef __cplusplus
extern "C" {
#endif
    
G_API int gnative_IsAvailable();

G_API void gnative_Init();
G_API void gnative_Cleanup();

G_API gnative_Class *gnative_GetClass(const char *className);

G_API gnative_Class *gnative_ClassGetSuperclass(gnative_Class *cls);

G_API const char *gnative_ClassGetName(gnative_Class *cls);
G_API gnative_Class *gnative_ObjectGetClass(gnative_Object *obj);

G_API gnative_Constructor **gnative_ClassGetConstructors(gnative_Class *cls);
G_API gnative_Function **gnative_ClassGetFunctionsByName(gnative_Class *cls, const char *name);

G_API gnative_Value gnative_FunctionCall(gnative_Function *function, gnative_Object *object, gnative_Value *parameters);

G_API BOOL gnative_FunctionIsStatic(gnative_Function *function);

G_API gnative_Type gnative_FunctionGetReturnType(gnative_Function *function);
G_API gnative_Class *gnative_FunctionGetReturnClass(gnative_Function *function);
G_API const char *gnative_FunctionGetReturnSignature(gnative_Function *function);
G_API int gnative_FunctionGetParameterCount(gnative_Function *function);
G_API gnative_Type *gnative_FunctionGetParameterTypes(gnative_Function *function);
G_API gnative_Class **gnative_FunctionGetParameterClasses(gnative_Function *function);
G_API const char **gnative_FunctionGetParameterSignatures(gnative_Function *function);

G_API int gnative_ConstructorGetParameterCount(gnative_Constructor *constructor);
G_API gnative_Type *gnative_ConstructorGetParameterTypes(gnative_Constructor *constructor);
G_API gnative_Class **gnative_ConstructorGetParameterClasses(gnative_Constructor *constructor);

G_API gnative_Object *gnative_CreateObject(gnative_Class *cls, gnative_Constructor *constructor, gnative_Value *parameters);
G_API void gnative_ObjectRetain(gnative_Object *obj);
G_API BOOL gnative_ObjectRelease(gnative_Object *obj);

G_API BOOL gnative_ClassIsSubclass(gnative_Class *cls, gnative_Class *subclass);

G_API gnative_Class *gnative_CreateClass(const char *name, gnative_Class *superclass);

G_API gnative_Function *gnative_ClassAddFunction(gnative_Class *cls, const char *name, const char *types, gnative_Implementation imp);
G_API void gnative_ClassAddStaticFunction(gnative_Class *cls, const char *name, const char *types, gnative_StaticImplementation imp);

#ifdef __cplusplus
}
#endif

#endif
