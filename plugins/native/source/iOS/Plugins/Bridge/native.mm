#include "native.h"
#include <stdlib.h>
#include <map>
#include <string>
#include <vector>
#import <objc/runtime.h>
#include "pystring.h"
#import <Foundation/Foundation.h>
#include <stdarg.h>

typedef struct gnative_Constructor
{
} gnative_Constructor;

typedef struct gnative_Function
{
	gnative_Class *cls;
	std::string name;
    std::string signature;
	SEL selector;
	gnative_Type returnType;
	std::string returnName;
	gnative_Class *returnClass;
	std::vector<gnative_Type> parameterTypes;
	std::vector<std::string> parameterNames;
	std::vector<gnative_Class*> parameterClasses;
	std::vector<const char*> parameterNamePointers;
	bool isStatic;
	bool retain;
	gnative_Implementation implementation;
} gnative_Function;

typedef struct gnative_Class
{
	Class cls;
	std::string name;
	std::map<SEL, gnative_Function*> functions;
	std::vector<gnative_Function*> functionPointers;
	std::map<std::string, std::vector<gnative_Function*> > functionPointersByName;
} gnative_Class;

typedef struct gnative_Object
{
	int refCount;
	gnative_Class *cls;
	id obj;
} gnative_Object;

// need to check what types objc provides
static gnative_Type getType(const char *parameter)
{
	gnative_Type type;
	if (!strcmp(parameter, "v"))
	{
		type = GNATIVE_TYPE_VOID;
	}
	else if (!strcmp(parameter, "B"))
	{
		type = GNATIVE_TYPE_BOOLEAN;
	}
	else if (!strcmp(parameter, "c") || !strcmp(parameter, "C"))
	{
		type = GNATIVE_TYPE_CHAR;
	}
	else if (!strcmp(parameter, "s") || !strcmp(parameter, "S"))
	{
		type = GNATIVE_TYPE_SHORT;
	}
	else if (!strcmp(parameter, "i") || !strcmp(parameter, "I") || !strcmp(parameter, "l") || !strcmp(parameter, "L"))
	{
		type = GNATIVE_TYPE_INT;
	}
	else if (!strcmp(parameter, "q") || !strcmp(parameter, "Q"))
	{
		type = GNATIVE_TYPE_LONG;
	}
	else if (!strcmp(parameter, "f"))
	{
		type = GNATIVE_TYPE_FLOAT;
	}
	else if (!strcmp(parameter, "d"))
	{
		type = GNATIVE_TYPE_DOUBLE;
	}
	else if (!strcmp(parameter, "D"))
	{
		type = GNATIVE_TYPE_LONG_DOUBLE;
	}
	else if (!strcmp(parameter, "*") || !strcmp(parameter, "r*"))
	{
		type = GNATIVE_TYPE_STRING;
	}
	else if (!strcmp(parameter, "@"))
	{
		type = GNATIVE_TYPE_OBJECT;
	}
	else
	{
		type = GNATIVE_TYPE_COMPLEX;
	}
	return type;
}

static id s_instanceMethodImp(id obj, SEL sel, ...);

class GNative
{
public:
	GNative()
	{
        
    }

	~GNative()
	{
		std::map<std::string, gnative_Class*>::iterator iter, e = classes_.end();
        for (iter = classes_.begin(); iter != e; ++iter)
        {
        	ClassDelete(iter->second);
        }
        classes_.clear();
	}

	void updateFunctionsHelper(gnative_Class *cls, Method *methods, unsigned int length, bool isStatic)
	{
		if (methods == NULL)
			return;

		// going through methods
		for (int i = 0; i < length; ++i)
		{
			SEL selector = method_getName(methods[i]);
			
			if (cls->functions.find(selector) == cls->functions.end())
				cls->functions[selector] = new gnative_Function;
			
			gnative_Function &function = *cls->functions[selector];
			
			function.cls = cls;
			const char *method = sel_getName(selector);
			function.name = functionNameFromSelector(method);
			function.signature = method;
			function.selector = selector;
			
			// get return type
			const char *ret = method_copyReturnType(methods[i]);
			gnative_Type type = getType(ret);
			function.returnType = type;
			function.returnName = ret;
			function.returnClass = NULL;
			free((char*)ret);
			
			// get method args
			function.parameterTypes.clear();
			function.parameterNames.clear();
			unsigned size = method_getNumberOfArguments(methods[i]);
			for (int j = 2; j < size; ++j)
			{
				const char *parameter = method_copyArgumentType(methods[i], j);
				gnative_Type type = getType(parameter);
				function.parameterTypes.push_back(type);
				function.parameterNames.push_back(parameter);
				free((char*)parameter);
			}
			
			function.isStatic = isStatic;
			
			function.retain =
				function.returnType == GNATIVE_TYPE_OBJECT &&
				!pystring::startswith(function.signature, "alloc") &&
				!pystring::startswith(function.signature, "new") &&
				!pystring::startswith(function.signature, "copy") &&
				!pystring::startswith(function.signature, "mutableCopy");
		}
		free(methods);
	}
	
	void updateFunctions(gnative_Class *cls)
	{
		cls->functions.clear();
		cls->functionPointers.clear();
		cls->functionPointersByName.clear();
		
		Class tempClass = cls->cls;

		Method *methods;
		unsigned int length;

		if ((methods = class_copyMethodList(object_getClass(tempClass), &length)))
			updateFunctionsHelper(cls, methods, length, true);

		if ((methods = class_copyMethodList(tempClass, &length)))
			updateFunctionsHelper(cls, methods, length, false);
		
		{
			std::map<SEL, gnative_Function*>::iterator iter, e = cls->functions.end();
			for (iter = cls->functions.begin(); iter != e; ++iter)
			{
				cls->functionPointers.push_back(iter->second);
				cls->functionPointersByName[iter->second->name].push_back(iter->second);
			}
		}

		{
			cls->functionPointers.push_back(NULL);
			std::map<std::string, std::vector<gnative_Function*> >::iterator iter, e = cls->functionPointersByName.end();
			for (iter = cls->functionPointersByName.begin(); iter != e; ++iter)
				iter->second.push_back(NULL);
		}
	}
	
	gnative_Class *GetClass(const char *className)
	{
		std::map<std::string, gnative_Class*>::iterator iter = classes_.find(className);
		if (iter != classes_.end())
			return iter->second;

		Class tempClass = objc_getClass(className);
		if (tempClass == NULL)
			return NULL;

		gnative_Class *cls = new gnative_Class;
		cls->name = className;
		cls->cls = tempClass;

		updateFunctions(cls);

		classes_[className] = cls;
		return cls;	
	}
	
	gnative_Class *ClassGetSuperclass(gnative_Class *cls)
	{
		Class superclass = class_getSuperclass(cls->cls);
		
		return superclass ? GetClass(class_getName(superclass)) : NULL;
	}

	void ClassDelete(gnative_Class *cls)
	{
		delete cls;
	}
    
    gnative_Object *CreateObject(gnative_Class *cls)
    {
        id obj = class_createInstance(cls->cls, 0);
		return createObject(obj, false);
    }

	gnative_Function **ClassGetFunctionsByName(gnative_Class *cls, const char *name)
	{
		std::map<std::string, std::vector<gnative_Function*> >::iterator iter = cls->functionPointersByName.find(name);
		if (iter == cls->functionPointersByName.end())
			return NULL;
		return &iter->second[0];
	}

	static std::string functionNameFromSelector(const char *name)
	{
		//std::vector<std::string> result;
		//pystring::split(name, result, ":");
		//return result[0];
		return pystring::replace(name, ":", "_");
	}

	gnative_Object *createObject(id obj, bool retain = true)
	{
		if (obj == nil)
			return NULL;
		
		std::map<id, gnative_Object*>::iterator iter = objects_.find(obj);
		
		gnative_Object *obj2;
		if (iter == objects_.end())
		{
			if (retain)
				[obj retain];

			obj2 = new gnative_Object;
			obj2->refCount = 1;
			obj2->cls = GetClass(object_getClassName(obj));
			obj2->obj = obj;
			
			objects_[obj] = obj2;
		}
		else
		{
			obj2 = iter->second;
			obj2->refCount++;
		}

		return obj2;
	}

	gnative_Value FunctionCall(gnative_Function *function, gnative_Object *object, gnative_Value *parameters)
	{
		SEL selector = function->selector;
		id target = function->isStatic ? function->cls->cls : object->obj;
		NSMethodSignature *signature = [target methodSignatureForSelector:selector];
		NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:signature];
		[invocation setTarget:target];
		[invocation setSelector:selector];

        for (size_t i = 0; i < function->parameterTypes.size(); ++i)
        {
            switch (function->parameterTypes[i])
            {
                case GNATIVE_TYPE_BOOLEAN:
                    [invocation setArgument:&parameters[i].z atIndex:i + 2];
                    break;
                case GNATIVE_TYPE_CHAR:
                    [invocation setArgument:&parameters[i].c atIndex:i + 2];
                    break;
                case GNATIVE_TYPE_SHORT:
                    [invocation setArgument:&parameters[i].s atIndex:i + 2];
                    break;
                case GNATIVE_TYPE_INT:
                    [invocation setArgument:&parameters[i].i atIndex:i + 2];
                    break;
                case GNATIVE_TYPE_LONG:
                    [invocation setArgument:&parameters[i].l atIndex:i + 2];
                    break;
                case GNATIVE_TYPE_FLOAT:
                    [invocation setArgument:&parameters[i].f atIndex:i + 2];
                    break;
                case GNATIVE_TYPE_DOUBLE:
                    [invocation setArgument:&parameters[i].d atIndex:i + 2];
                    break;
                case GNATIVE_TYPE_LONG_DOUBLE:
                    [invocation setArgument:&parameters[i].ld atIndex:i + 2];
                    break;
                case GNATIVE_TYPE_STRING:
                    [invocation setArgument:&parameters[i].str atIndex:i + 2];
                    break;
                case GNATIVE_TYPE_OBJECT:
					if(parameters[i].obj != NULL)
						[invocation setArgument:&parameters[i].obj->obj atIndex:i + 2];
					else
						[invocation setArgument:nil atIndex:i + 2];
					break;
				case GNATIVE_TYPE_COMPLEX:
                    [invocation setArgument:parameters[i].ptr atIndex:i + 2];
					break;
				default:
					break;
            }
        }

        @try {
            [invocation invoke];
        }
        @catch (NSException *exception) {
            NSLog(@"Error invoking method '%s' because %s", function->name.c_str(), [[exception description] UTF8String]);
        }
        
        NSUInteger methodReturnLength = [signature methodReturnLength];
        
        gnative_Value result;
        
        if (methodReturnLength > 0)
		{
            void *buffer = malloc(methodReturnLength);
            [invocation getReturnValue:buffer];
            
			switch (function->returnType)
            {
                case GNATIVE_TYPE_BOOLEAN:
                    result.z = *(BOOL *)buffer;
                    break;
                case GNATIVE_TYPE_CHAR:
                    result.c = *(char *)buffer;
                    break;
                case GNATIVE_TYPE_SHORT:
                    result.s = *(short *)buffer;
                    break;
                case GNATIVE_TYPE_INT:
                    result.i = *(int *)buffer;
                    break;
                case GNATIVE_TYPE_LONG:
                    result.l = *(long long *)buffer;
                    break;
                case GNATIVE_TYPE_FLOAT:
                    result.f = *(float *)buffer;
                    break;
                case GNATIVE_TYPE_DOUBLE:
                    result.d = *(double *)buffer;
                    break;
                case GNATIVE_TYPE_LONG_DOUBLE:
                    result.ld = *(long double *)buffer;
                    break;
                case GNATIVE_TYPE_STRING:
                    result.str = strdup(*(char **)buffer);
                    break;
                case GNATIVE_TYPE_OBJECT:
                {
                    id obj = *(id *)buffer;
					result.obj = createObject(obj, function->retain);
                    break;
                }
            }
            
            free(buffer);
        }
        
        return result;
    }
	
	BOOL FunctionIsStatic(gnative_Function *function)
	{
		return function->isStatic;
	}

	gnative_Type FunctionGetReturnType(gnative_Function *function)
	{
		return function->returnType;
	}
	
	gnative_Class *FunctionGetReturnClass(gnative_Function *function)
	{
		return function->returnClass;
	}
	
	const char *FunctionGetReturnSignature(gnative_Function *function)
	{
		return function->returnName.c_str();
	}

	gnative_Type *FunctionGetParameterTypes(gnative_Function *function)
	{
		return function->parameterTypes.empty() ? NULL : &function->parameterTypes[0];
	}

	int FunctionGetParameterCount(gnative_Function *function)
	{
		return function->parameterTypes.size();
	}

	void ObjectRetain(gnative_Object *obj)
	{
		if (obj == NULL)
			return;

		obj->refCount++;
	}

	BOOL ObjectRelease(gnative_Object *obj)
	{
		if (obj == NULL)
			return 0;
		
		if (--obj->refCount == 0)
		{
			objects_.erase(obj->obj);
			[obj->obj release];
			delete obj;
			return 1;
		}

		return 0;
	}

	BOOL ClassIsSubclass(gnative_Class *cls, gnative_Class *subclass)
	{
        return [subclass->cls isSubclassOfClass:cls->cls];
	}

	gnative_Class **FunctionGetParameterClasses(gnative_Function *function)
	{
		std::vector<gnative_Type> &types = function->parameterTypes;
		std::vector<std::string> &names = function->parameterNames;
		std::vector<gnative_Class*> &classes = function->parameterClasses;

		if (classes.empty())
		{
			classes.resize(types.size());
			for (std::size_t i = 0; i < classes.size(); ++i)
				classes[i] = (types[i] == GNATIVE_TYPE_OBJECT) ? GetClass(names[i].c_str()) : NULL;
		}

		return classes.empty() ? NULL : &classes[0];
	}

	const char *ClassGetName(gnative_Class *cls)
	{
		return cls->name.c_str();
	}

	gnative_Class *ObjectGetClass(gnative_Object *obj)
	{
		return obj->cls;
	}
	
	gnative_Class *CreateClass(const char *name, gnative_Class *superclass)
	{
		Class cls = objc_allocateClassPair(superclass ? superclass->cls : NULL, name, 0);
		if (cls)
			objc_registerClassPair(cls);
		return GetClass(name);
	}

	gnative_Function *ClassAddFunction(gnative_Class *cls, const char *name, const char *types, gnative_Implementation imp)
	{
		SEL sel = sel_getUid(name);
		class_replaceMethod(cls->cls, sel, s_instanceMethodImp, types);
		updateFunctions(cls);
		gnative_Function *function = cls->functions[sel];
		function->implementation = imp;
		return function;
	}

private:
	std::map<std::string, gnative_Class*> classes_;
	std::map<id, gnative_Object*> objects_;
};

static GNative *s_nat = NULL;

static id s_instanceMethodImp(id obj, SEL sel, ...)
{
	gnative_Class *cls = s_nat->GetClass(object_getClassName(obj));
	gnative_Function *function = cls->functions[sel];

	gnative_Object *self = s_nat->createObject(obj);
	
	std::vector<gnative_Value> parameters(function->parameterTypes.size());
	va_list ap;
	va_start(ap, sel);
	for (std::size_t i = 0; i < function->parameterTypes.size(); ++i)
	{
		switch (function->parameterTypes[i])
		{
		case GNATIVE_TYPE_BOOLEAN:
			parameters[i].z = va_arg(ap, BOOL);
			break;
		case GNATIVE_TYPE_CHAR:
			parameters[i].c = va_arg(ap, char);
			break;
		case GNATIVE_TYPE_SHORT:
			parameters[i].s = va_arg(ap, short);
			break;
		case GNATIVE_TYPE_INT:
			parameters[i].i = va_arg(ap, int);
			break;
		case GNATIVE_TYPE_LONG:
			parameters[i].l = va_arg(ap, long long);
			break;
		case GNATIVE_TYPE_FLOAT:
			parameters[i].f = va_arg(ap, float);
			break;
		case GNATIVE_TYPE_DOUBLE:
			parameters[i].d = va_arg(ap, double);
			break;
		case GNATIVE_TYPE_LONG_DOUBLE:
			parameters[i].d = va_arg(ap, long double);
			break;
		case GNATIVE_TYPE_STRING:
			break;
		case GNATIVE_TYPE_OBJECT:
			parameters[i].obj = s_nat->createObject(va_arg(ap, id));
			break;
		case GNATIVE_TYPE_COMPLEX:
			break;
		}
	}
	va_end(ap);

	function->implementation(self, function, &parameters[0]);
		
	for (std::size_t i = 0; i < function->parameterTypes.size(); ++i)
	{
		switch (function->parameterTypes[i])
		{
		case GNATIVE_TYPE_OBJECT:
			s_nat->ObjectRelease(parameters[i].obj);
			break;
		default:
			break;
		}
	}

	s_nat->ObjectRelease(self);
	
	return nil;
}


extern "C" {

int gnative_IsAvailable()
{
	return 1;
}

void gnative_Init()
{
	if (s_nat == NULL)
		s_nat = new GNative;
}

void gnative_Cleanup()
{
	if (s_nat != NULL)
	{
		delete s_nat;
		s_nat = NULL;
	}
}

gnative_Class *gnative_GetClass(const char *className)
{
	return s_nat->GetClass(className);
}

gnative_Class *gnative_ClassGetSuperclass(gnative_Class *cls)
{
	return s_nat->ClassGetSuperclass(cls);
}

gnative_Constructor **gnative_ClassGetConstructors(gnative_Class *cls)
{
    return NULL;
}

gnative_Function **gnative_ClassGetFunctionsByName(gnative_Class *cls, const char *name)
{
	return s_nat->ClassGetFunctionsByName(cls, name);
}

gnative_Value gnative_FunctionCall(gnative_Function *function, gnative_Object *object, gnative_Value *parameters)
{
	return s_nat->FunctionCall(function, object, parameters);
}
	
BOOL gnative_FunctionIsStatic(gnative_Function *function)
{
	return s_nat->FunctionIsStatic(function);
}

gnative_Type gnative_FunctionGetReturnType(gnative_Function *function)
{
	return s_nat->FunctionGetReturnType(function);
}
	
gnative_Class *gnative_FunctionGetReturnClass(gnative_Function *function)
{
	return s_nat->FunctionGetReturnClass(function);
}

const char *gnative_FunctionGetReturnSignature(gnative_Function *function)
{
	return s_nat->FunctionGetReturnSignature(function);
}

gnative_Type *gnative_FunctionGetParameterTypes(gnative_Function *function)
{
	return s_nat->FunctionGetParameterTypes(function);
}

gnative_Type *gnative_ConstructorGetParameterTypes(gnative_Constructor *constructor)
{
    return NULL;
}

int gnative_FunctionGetParameterCount(gnative_Function *function)
{
	return s_nat->FunctionGetParameterCount(function);
}

int gnative_ConstructorGetParameterCount(gnative_Constructor *constructor)
{
    return 0;
}

gnative_Object *gnative_CreateObject(gnative_Class *cls, gnative_Constructor *constructor, gnative_Value *parameters)
{
	return s_nat->CreateObject(cls);
}

void gnative_ObjectRetain(gnative_Object *obj)
{
	s_nat->ObjectRetain(obj);
}
	
BOOL gnative_ObjectRelease(gnative_Object *obj)
{
	return s_nat->ObjectRelease(obj);
}

BOOL gnative_ClassIsSubclass(gnative_Class *cls, gnative_Class *subclass)
{
	return s_nat->ClassIsSubclass(cls, subclass);
}

gnative_Class **gnative_FunctionGetParameterClasses(gnative_Function *function)
{
	return s_nat->FunctionGetParameterClasses(function);
}

gnative_Class **gnative_ConstructorGetParameterClasses(gnative_Constructor *constructor)
{
    return NULL;
}

const char *gnative_ClassGetName(gnative_Class *cls)
{
	return s_nat->ClassGetName(cls);
}

gnative_Class *gnative_ObjectGetClass(gnative_Object *obj)
{
	return s_nat->ObjectGetClass(obj);
}

gnative_Class *gnative_CreateClass(const char *name, gnative_Class *superclass)
{
	return s_nat->CreateClass(name, superclass);
}

gnative_Function *gnative_ClassAddFunction(gnative_Class *cls, const char *name, const char *types, gnative_Implementation imp)
{
    return s_nat->ClassAddFunction(cls, name, types, imp);
}

}
