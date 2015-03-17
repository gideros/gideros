#include <native.h>
#include <jni.h>
#include <stdlib.h>
#include <glog.h>
#include <map>
#include <string>
#include <vector>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

//__android_log_print(ANDROID_LOG_DEBUG, "tag", "Output String");
#include <android/log.h>

typedef struct gnative_Constructor
{
	gnative_Class *cls;
	int index;
	std::vector<gnative_Type> parameterTypes;
	std::vector<std::string> parameterNames;
	std::vector<gnative_Class*> parameterClasses;
} gnative_Constructor;

typedef struct gnative_Function
{
	gnative_Class *cls;
	int index;
	std::string name;
	gnative_Type returnType;
	std::string returnName;
	gnative_Class *returnClass;
	std::vector<gnative_Type> parameterTypes;
	std::vector<std::string> parameterNames;
	std::vector<gnative_Class*> parameterClasses;
} gnative_Function;

typedef struct gnative_Field
{
	gnative_Class *cls;
	int index;
	std::string name;
	gnative_Type returnType;
} gnative_Field;

typedef struct gnative_Class
{
	jclass cls;
	std::string name;
	std::vector<gnative_Constructor> constructors;
	std::vector<gnative_Constructor*> constructorPointers;
	std::vector<gnative_Function> functions;
	std::vector<gnative_Function*> functionPointers;
	std::map<std::string, std::vector<gnative_Function*> > functionPointersByName;
	std::vector<gnative_Function> staticFunctions;
	std::vector<gnative_Function*> staticFunctionPointers;
	std::map<std::string, std::vector<gnative_Function*> > staticFunctionPointersByName;
	std::vector<gnative_Field> fields;
	std::vector<gnative_Field*> fieldPointers;
	std::map<std::string, std::vector<gnative_Field*> > fieldPointersByName;
	std::vector<gnative_Field> staticFields;
	std::vector<gnative_Field*> staticFieldPointers;
	std::map<std::string, std::vector<gnative_Field*> > staticFieldPointersByName;
} gnative_Class;

typedef struct gnative_Object
{
	gnative_Class *cls;
	jobject obj;
} gnative_Object;

static gnative_Type getType(const char *parameter)
{
	gnative_Type type;
	if (!strcmp(parameter, "void"))
	{
		type = GNATIVE_TYPE_VOID;
	}
	else if (!strcmp(parameter, "boolean"))
	{
		type = GNATIVE_TYPE_BOOLEAN;
	}
	else if (!strcmp(parameter, "byte"))
	{
		type = GNATIVE_TYPE_BYTE;
	}
	else if (!strcmp(parameter, "char"))
	{
		type = GNATIVE_TYPE_CHAR;
	}
	else if (!strcmp(parameter, "short"))
	{
		type = GNATIVE_TYPE_SHORT;
	}
	else if (!strcmp(parameter, "int"))
	{
		type = GNATIVE_TYPE_INT;
	}
	else if (!strcmp(parameter, "long"))
	{
		type = GNATIVE_TYPE_LONG;
	}
	else if (!strcmp(parameter, "float"))
	{
		type = GNATIVE_TYPE_FLOAT;
	}
	else if (!strcmp(parameter, "double"))
	{
		type = GNATIVE_TYPE_DOUBLE;
	}
	else if (!strcmp(parameter, "java.lang.String"))
	{
		type = GNATIVE_TYPE_STRING;
	}
	else if(gnative_ClassIsSubclass(parameter, "java.lang.String") == true)
	{
		type = GNATIVE_TYPE_STRING;
	}
	else
	{
		type = GNATIVE_TYPE_OBJECT;
	}
	return type;
}

class GNative
{
public:
	GNative()
	{
		JNIEnv *env = g_getJNIEnv();

		jobject obj;

		obj = env->FindClass("com/giderosmobile/android/plugins/bridge/GBridge");
		cls_ = (jclass)env->NewGlobalRef(obj);
		env->DeleteLocalRef(obj);

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(J)V"), (jlong)this);

		obj = env->FindClass("java/lang/Object");
		ObjectCls_ = (jclass)env->NewGlobalRef(obj);
		env->DeleteLocalRef(obj);

		obj = env->FindClass("java/lang/Boolean");
		BooleanCls_ = (jclass)env->NewGlobalRef(obj);
		env->DeleteLocalRef(obj);
		BooleanInit_ = env->GetMethodID(BooleanCls_, "<init>", "(Z)V");

		obj = env->FindClass("java/lang/Byte");
		ByteCls_ = (jclass)env->NewGlobalRef(obj);
		env->DeleteLocalRef(obj);
		ByteInit_ = env->GetMethodID(ByteCls_, "<init>", "(B)V");

		obj = env->FindClass("java/lang/Character");
		CharCls_ = (jclass)env->NewGlobalRef(obj);
		env->DeleteLocalRef(obj);
		CharInit_ = env->GetMethodID(CharCls_, "<init>", "(C)V");

		obj = env->FindClass("java/lang/Short");
		ShortCls_ = (jclass)env->NewGlobalRef(obj);
		env->DeleteLocalRef(obj);
		ShortInit_ = env->GetMethodID(ShortCls_, "<init>", "(S)V");

		obj = env->FindClass("java/lang/Integer");
		IntegerCls_ = (jclass)env->NewGlobalRef(obj);
		env->DeleteLocalRef(obj);
		IntegerInit_ = env->GetMethodID(IntegerCls_, "<init>", "(I)V");

		obj = env->FindClass("java/lang/Long");
		LongCls_ = (jclass)env->NewGlobalRef(obj);
		env->DeleteLocalRef(obj);
		LongInit_ = env->GetMethodID(LongCls_, "<init>", "(J)V");

		obj = env->FindClass("java/lang/Float");
		FloatCls_ = (jclass)env->NewGlobalRef(obj);
		env->DeleteLocalRef(obj);
		FloatInit_ = env->GetMethodID(FloatCls_, "<init>", "(F)V");

		obj = env->FindClass("java/lang/Double");
		DoubleCls_ = (jclass)env->NewGlobalRef(obj);
		env->DeleteLocalRef(obj);
		DoubleInit_ = env->GetMethodID(DoubleCls_, "<init>", "(D)V");

		getClassName_ = env->GetStaticMethodID(cls_, "getClassName", "(Ljava/lang/Object;)Ljava/lang/String;");
		getClassConstructorsLength_ = env->GetStaticMethodID(cls_, "getClassConstructorsLength", "(Ljava/lang/Class;)I");
		getClassConstructorParameters_ = env->GetStaticMethodID(cls_, "getClassConstructorParameters", "(Ljava/lang/Class;I)[Ljava/lang/String;");
		getClassMethods_ = env->GetStaticMethodID(cls_, "getClassMethods", "(Ljava/lang/Class;)[Ljava/lang/String;");
		getClassMethodParameters_ = env->GetStaticMethodID(cls_, "getClassMethodParameters", "(Ljava/lang/Class;I)[Ljava/lang/String;");
		getClassFields_ = env->GetStaticMethodID(cls_, "getClassFields", "(Ljava/lang/Class;)[Ljava/lang/String;");
		getClassFieldTypes_ = env->GetStaticMethodID(cls_, "getClassFieldTypes", "(Ljava/lang/Class;)[Ljava/lang/String;");

		getFieldObject_ = env->GetStaticMethodID(cls_, "getFieldObject", "(Ljava/lang/Class;ILjava/lang/Object;)Ljava/lang/Object;");
		getFieldVoid_ = env->GetStaticMethodID(cls_, "getFieldVoid", "(Ljava/lang/Class;ILjava/lang/Object;)V");
		getFieldBoolean_ = env->GetStaticMethodID(cls_, "getFieldBoolean", "(Ljava/lang/Class;ILjava/lang/Object;)Z");
		getFieldByte_ = env->GetStaticMethodID(cls_, "getFieldByte", "(Ljava/lang/Class;ILjava/lang/Object;)B");
		getFieldChar_ = env->GetStaticMethodID(cls_, "getFieldChar", "(Ljava/lang/Class;ILjava/lang/Object;)C");
		getFieldShort_ = env->GetStaticMethodID(cls_, "getFieldShort", "(Ljava/lang/Class;ILjava/lang/Object;)S");
		getFieldInt_ = env->GetStaticMethodID(cls_, "getFieldInt", "(Ljava/lang/Class;ILjava/lang/Object;)I");
		getFieldLong_ = env->GetStaticMethodID(cls_, "getFieldLong", "(Ljava/lang/Class;ILjava/lang/Object;)J");
		getFieldFloat_ = env->GetStaticMethodID(cls_, "getFieldFloat", "(Ljava/lang/Class;ILjava/lang/Object;)F");
		getFieldDouble_ = env->GetStaticMethodID(cls_, "getFieldDouble", "(Ljava/lang/Class;ILjava/lang/Object;)D");
		getFieldString_ = env->GetStaticMethodID(cls_, "getFieldString", "(Ljava/lang/Class;ILjava/lang/Object;)Ljava/lang/String;");
		
		setFieldObject_ = env->GetStaticMethodID(cls_, "setFieldObject", "(Ljava/lang/Class;ILjava/lang/Object;Ljava/lang/Object;)V");
		setFieldVoid_ = env->GetStaticMethodID(cls_, "setFieldVoid", "(Ljava/lang/Class;ILjava/lang/Object;)V");
		setFieldBoolean_ = env->GetStaticMethodID(cls_, "setFieldBoolean", "(Ljava/lang/Class;ILjava/lang/Object;Z)V");
		setFieldByte_ = env->GetStaticMethodID(cls_, "setFieldByte", "(Ljava/lang/Class;ILjava/lang/Object;B)V");
		setFieldChar_ = env->GetStaticMethodID(cls_, "setFieldChar", "(Ljava/lang/Class;ILjava/lang/Object;C)V");
		setFieldShort_ = env->GetStaticMethodID(cls_, "setFieldShort", "(Ljava/lang/Class;ILjava/lang/Object;S)V");
		setFieldInt_ = env->GetStaticMethodID(cls_, "setFieldInt", "(Ljava/lang/Class;ILjava/lang/Object;I)V");
		setFieldLong_ = env->GetStaticMethodID(cls_, "setFieldLong", "(Ljava/lang/Class;ILjava/lang/Object;J)V");
		setFieldFloat_ = env->GetStaticMethodID(cls_, "setFieldFloat", "(Ljava/lang/Class;ILjava/lang/Object;F)V");
		setFieldDouble_ = env->GetStaticMethodID(cls_, "setFieldDouble", "(Ljava/lang/Class;ILjava/lang/Object;D)V");
		setFieldString_ = env->GetStaticMethodID(cls_, "setFieldString", "(Ljava/lang/Class;ILjava/lang/Object;Ljava/lang/String;)V");

		getBoolean_ = env->GetStaticMethodID(cls_, "getBoolean", "(I[Ljava/lang/Object;)Z");
		getByte_ = env->GetStaticMethodID(cls_, "getByte", "(I[Ljava/lang/Object;)B");
		getChar_ = env->GetStaticMethodID(cls_, "getChar", "(I[Ljava/lang/Object;)C");
		getShort_ = env->GetStaticMethodID(cls_, "getShort", "(I[Ljava/lang/Object;)S");
		getInt_ = env->GetStaticMethodID(cls_, "getInt", "(I[Ljava/lang/Object;)I");
		getLong_ = env->GetStaticMethodID(cls_, "getLong", "(I[Ljava/lang/Object;)J");
		getFloat_ = env->GetStaticMethodID(cls_, "getFloat", "(I[Ljava/lang/Object;)F");
		getDouble_ = env->GetStaticMethodID(cls_, "getDouble", "(I[Ljava/lang/Object;)D");

		createObject_ = env->GetStaticMethodID(cls_, "createObject", "(Ljava/lang/Class;I[Ljava/lang/Object;)Ljava/lang/Object;");
		createProxy_ = env->GetStaticMethodID(cls_, "createProxy", "(Ljava/lang/String;J)Ljava/lang/Object;");
		callStaticFunctionObject_ = env->GetStaticMethodID(cls_, "callStaticFunctionObject", "(Ljava/lang/Class;ILjava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
		callStaticFunctionVoid_ = env->GetStaticMethodID(cls_, "callStaticFunctionVoid", "(Ljava/lang/Class;ILjava/lang/Object;[Ljava/lang/Object;)V");
		callStaticFunctionBoolean_ = env->GetStaticMethodID(cls_, "callStaticFunctionBoolean", "(Ljava/lang/Class;ILjava/lang/Object;[Ljava/lang/Object;)Z");
		callStaticFunctionByte_ = env->GetStaticMethodID(cls_, "callStaticFunctionByte", "(Ljava/lang/Class;ILjava/lang/Object;[Ljava/lang/Object;)B");
		callStaticFunctionChar_ = env->GetStaticMethodID(cls_, "callStaticFunctionChar", "(Ljava/lang/Class;ILjava/lang/Object;[Ljava/lang/Object;)C");
		callStaticFunctionShort_ = env->GetStaticMethodID(cls_, "callStaticFunctionShort", "(Ljava/lang/Class;ILjava/lang/Object;[Ljava/lang/Object;)S");
		callStaticFunctionInt_ = env->GetStaticMethodID(cls_, "callStaticFunctionInt", "(Ljava/lang/Class;ILjava/lang/Object;[Ljava/lang/Object;)I");
		callStaticFunctionLong_ = env->GetStaticMethodID(cls_, "callStaticFunctionLong", "(Ljava/lang/Class;ILjava/lang/Object;[Ljava/lang/Object;)J");
		callStaticFunctionFloat_ = env->GetStaticMethodID(cls_, "callStaticFunctionFloat", "(Ljava/lang/Class;ILjava/lang/Object;[Ljava/lang/Object;)F");
		callStaticFunctionDouble_ = env->GetStaticMethodID(cls_, "callStaticFunctionDouble", "(Ljava/lang/Class;ILjava/lang/Object;[Ljava/lang/Object;)D");
		callStaticFunctionString_ = env->GetStaticMethodID(cls_, "callStaticFunctionString", "(Ljava/lang/Class;ILjava/lang/Object;[Ljava/lang/Object;)Ljava/lang/String;");
		isInstance_ = env->GetStaticMethodID(cls_, "isInstance", "(Ljava/lang/Class;Ljava/lang/Object;)Z");
		isSubclass_ = env->GetStaticMethodID(cls_, "isSubclass", "(Ljava/lang/String;Ljava/lang/String;)Z");
	}

	~GNative()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		env->DeleteGlobalRef(cls_);
		env->DeleteGlobalRef(ObjectCls_);
		env->DeleteGlobalRef(BooleanCls_);
		env->DeleteGlobalRef(ByteCls_);
		env->DeleteGlobalRef(CharCls_);
		env->DeleteGlobalRef(ShortCls_);
		env->DeleteGlobalRef(IntegerCls_);
		env->DeleteGlobalRef(LongCls_);
		env->DeleteGlobalRef(FloatCls_);
		env->DeleteGlobalRef(DoubleCls_);
		std::map<std::string, gnative_Class*>::iterator iter, e = classes_.end();
        for (iter = classes_.begin(); iter != e; ++iter)
        {
        	ClassDelete(iter->second);
        }
        classes_.clear();
	}

	gnative_Class *ClassCreate(const char *className)
	{
		JNIEnv *env = g_getJNIEnv();
		std::map<std::string, gnative_Class*>::iterator iter = classes_.find(className);
		if (iter != classes_.end())
			return iter->second;

		jclass tempClass = env->FindClass(className);
		if (tempClass == NULL)
			return NULL;

		gnative_Class *cls = new gnative_Class;
		cls->name = className;
		cls->cls = (jclass)env->NewGlobalRef(tempClass);
		env->DeleteLocalRef(tempClass);

		// Getting constructors
		int length = env->CallStaticIntMethod(cls_, getClassConstructorsLength_, cls->cls);
		for (int i = 0; i < length; ++i)
		{
			gnative_Constructor constructor;
			constructor.cls = cls;
			constructor.index = i;
			jobjectArray parameters = (jobjectArray)env->CallStaticObjectMethod(cls_, getClassConstructorParameters_, cls->cls, i);
			int size = env->GetArrayLength(parameters);
			for (int j = 0; j < size; ++j)
			{
				jstring jparameter = (jstring)env->GetObjectArrayElement(parameters, j);
				const char *parameter = env->GetStringUTFChars(jparameter, NULL);

				gnative_Type type = getType(parameter);

				constructor.parameterTypes.push_back(type);
				constructor.parameterNames.push_back(parameter);

				env->ReleaseStringUTFChars(jparameter, parameter);

				env->DeleteLocalRef(jparameter);
			}
			env->DeleteLocalRef(parameters);

			cls->constructors.push_back(constructor);
		}

		for (std::size_t i = 0; i < cls->constructors.size(); ++i)
			cls->constructorPointers.push_back(&cls->constructors[i]);
		cls->constructorPointers.push_back(NULL);

		// Getting methods
		jobjectArray methods = (jobjectArray)env->CallStaticObjectMethod(cls_, getClassMethods_, cls->cls);
		length = env->GetArrayLength(methods);
		for (int i = 0; i < length; ++i)
		{
			gnative_Function function;
			function.cls = cls;
			function.index = i;
			jstring jmethod = (jstring)env->GetObjectArrayElement(methods, i);
			const char *method = env->GetStringUTFChars(jmethod, NULL);
			bool isStatic = (method[0] == '+');
			function.name = method + 1;

			env->ReleaseStringUTFChars(jmethod, method);

			env->DeleteLocalRef(jmethod);

			jobjectArray parameters = (jobjectArray)env->CallStaticObjectMethod(cls_, getClassMethodParameters_, cls->cls, i);
			int size = env->GetArrayLength(parameters);
			for (int j = 0; j < size; ++j)
			{
				jstring jparameter = (jstring)env->GetObjectArrayElement(parameters, j);
				const char *parameter = env->GetStringUTFChars(jparameter, NULL);

				gnative_Type type = getType(parameter);
				if (j == 0)
				{
					function.returnType = type;
					function.returnName = parameter;
					function.returnClass = NULL;
				}
				else
				{
					function.parameterTypes.push_back(type);
					function.parameterNames.push_back(parameter);
				}

				env->ReleaseStringUTFChars(jparameter, parameter);

				env->DeleteLocalRef(jparameter);
			}
			env->DeleteLocalRef(parameters);

			if (isStatic)
				cls->staticFunctions.push_back(function);
			else
				cls->functions.push_back(function);
		}
		env->DeleteLocalRef(methods);

		// function pointers
		{
			for (std::size_t i = 0; i < cls->functions.size(); ++i)
				cls->functionPointers.push_back(&cls->functions[i]);
			cls->functionPointers.push_back(NULL);

			for (std::size_t i = 0; i < cls->functions.size(); ++i)
				cls->functionPointersByName[cls->functions[i].name].push_back(&cls->functions[i]);
			std::map<std::string, std::vector<gnative_Function*> >::iterator iter, e = cls->functionPointersByName.end();
			for (iter = cls->functionPointersByName.begin(); iter != e; ++iter)
				iter->second.push_back(NULL);
		}

		// static function pointers
		{
			for (std::size_t i = 0; i < cls->staticFunctions.size(); ++i)
				cls->staticFunctionPointers.push_back(&cls->staticFunctions[i]);
			cls->staticFunctionPointers.push_back(NULL);

			for (std::size_t i = 0; i < cls->staticFunctions.size(); ++i)
				cls->staticFunctionPointersByName[cls->staticFunctions[i].name].push_back(&cls->staticFunctions[i]);
			std::map<std::string, std::vector<gnative_Function*> >::iterator iter, e = cls->staticFunctionPointersByName.end();
			for (iter = cls->staticFunctionPointersByName.begin(); iter != e; ++iter)
				iter->second.push_back(NULL);
		}

		// Getting fields
		jobjectArray fields = (jobjectArray)env->CallStaticObjectMethod(cls_, getClassFields_, cls->cls);
		jobjectArray fieldTypes = (jobjectArray)env->CallStaticObjectMethod(cls_, getClassFieldTypes_, cls->cls);
		length = env->GetArrayLength(fields);
		for (int i = 0; i < length; ++i)
		{
			gnative_Field field;
			field.cls = cls;
			field.index = i;
			jstring jfield = (jstring)env->GetObjectArrayElement(fields, i);
			const char *fname = env->GetStringUTFChars(jfield, NULL);
			bool isStatic = (fname[0] == '+');
			field.name = fname + 1;

			env->ReleaseStringUTFChars(jfield, fname);

			env->DeleteLocalRef(jfield);

			jstring jfieldType = (jstring)env->GetObjectArrayElement(fieldTypes, i);
			const char *fieldType = env->GetStringUTFChars(jfieldType, NULL);

			gnative_Type type = getType(fieldType);
			field.returnType = type;

			env->ReleaseStringUTFChars(jfieldType, fieldType);

			env->DeleteLocalRef(jfieldType);

			if (isStatic)
				cls->staticFields.push_back(field);
			else
				cls->fields.push_back(field);
		}
		env->DeleteLocalRef(fields);
		env->DeleteLocalRef(fieldTypes);

		// field pointers
		{
			for (std::size_t i = 0; i < cls->fields.size(); ++i)
				cls->fieldPointers.push_back(&cls->fields[i]);
			cls->fieldPointers.push_back(NULL);

			for (std::size_t i = 0; i < cls->fields.size(); ++i)
				cls->fieldPointersByName[cls->fields[i].name].push_back(&cls->fields[i]);
			std::map<std::string, std::vector<gnative_Field*> >::iterator iter, e = cls->fieldPointersByName.end();
			for (iter = cls->fieldPointersByName.begin(); iter != e; ++iter)
				iter->second.push_back(NULL);
		}

		// static field pointers
		{
			for (std::size_t i = 0; i < cls->staticFields.size(); ++i)
				cls->staticFieldPointers.push_back(&cls->staticFields[i]);
			cls->staticFieldPointers.push_back(NULL);

			for (std::size_t i = 0; i < cls->staticFields.size(); ++i)
				cls->staticFieldPointersByName[cls->staticFields[i].name].push_back(&cls->staticFields[i]);
			std::map<std::string, std::vector<gnative_Field*> >::iterator iter, e = cls->staticFieldPointersByName.end();
			for (iter = cls->staticFieldPointersByName.begin(); iter != e; ++iter)
				iter->second.push_back(NULL);
		}
		
		classes_[className] = cls;
		return cls;	
	}

	void ClassDelete(gnative_Class *cls)
	{
		JNIEnv *env = g_getJNIEnv();
		env->DeleteGlobalRef(cls->cls);
		delete cls;
	}

	gnative_Constructor **ClassGetConstructors(gnative_Class *cls)
	{
		return &cls->constructorPointers[0];
	}

	gnative_Function **ObjectGetFunctionsByName(gnative_Object *obj, const char *name)
	{
		std::map<std::string, std::vector<gnative_Function*> >::iterator iter = obj->cls->functionPointersByName.find(name);
		if (iter == obj->cls->functionPointersByName.end())
			return NULL;
		return &iter->second[0];
	}

	gnative_Function **ClassGetStaticFunctionsByName(gnative_Class *cls, const char *name)
	{
		std::map<std::string, std::vector<gnative_Function*> >::iterator iter = cls->staticFunctionPointersByName.find(name);
		if (iter == cls->staticFunctionPointersByName.end())
			return NULL;
		return &iter->second[0];
	}

	gnative_Field **ClassGetFieldsByName(gnative_Class *cls, const char *name)
	{
		std::map<std::string, std::vector<gnative_Field*> >::iterator iter = cls->staticFieldPointersByName.find(name);
		if (iter == cls->staticFieldPointersByName.end())
			return NULL;
		return &iter->second[0];
	}

	gnative_Field **ObjectGetFieldsByName(gnative_Object *obj, const char *name)
	{
		std::map<std::string, std::vector<gnative_Field*> >::iterator iter = obj->cls->fieldPointersByName.find(name);
		if (iter == obj->cls->fieldPointersByName.end())
			return NULL;
		return &iter->second[0];
	}

	jobject createBoolean(JNIEnv* env, jboolean value)
	{
		return env->NewObject(BooleanCls_, BooleanInit_, value);
	}

	jobject createByte(JNIEnv* env, jbyte value)
	{
		return env->NewObject(ByteCls_, ByteInit_, value);
	}

	jobject createChar(JNIEnv* env, jchar value)
	{
		return env->NewObject(CharCls_, CharInit_, value);
	}

	jobject createShort(JNIEnv* env, jshort value)
	{
		return env->NewObject(ShortCls_, ShortInit_, value);
	}

	jobject createInteger(JNIEnv* env, jint value)
	{
		return env->NewObject(IntegerCls_, IntegerInit_, value);
	}

	jobject createLong(JNIEnv* env, jlong value)
	{
		return env->NewObject(LongCls_, LongInit_, value);
	}

	jobject createFloat(JNIEnv* env, jfloat value)
	{
		return env->NewObject(FloatCls_, FloatInit_, value);
	}

	jobject createDouble(JNIEnv* env, jdouble value)
	{
		return env->NewObject(DoubleCls_, DoubleInit_, value);
	}

	gnative_Value FunctionCall(gnative_Function *function, gnative_Object *object, gnative_Value *parameters)
	{
		JNIEnv *env = g_getJNIEnv();

		jobjectArray jparameters = env->NewObjectArray(function->parameterTypes.size(), ObjectCls_, NULL);

		for (size_t i = 0; i < function->parameterTypes.size(); ++i)
		{
			switch (function->parameterTypes[i])
			{
				case GNATIVE_TYPE_VOID:
					break;
				case GNATIVE_TYPE_BOOLEAN:
				{
					jobject jobj = createBoolean(env, parameters[i].z);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_BYTE:
				{
					jobject jobj = createByte(env, parameters[i].b);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_CHAR:
				{
					jobject jobj = createChar(env, parameters[i].c);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_SHORT:
				{
					jobject jobj = createShort(env, parameters[i].s);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_INT:
				{
					jobject jobj = createInteger(env, parameters[i].i);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_LONG:
				{
					jobject jobj = createLong(env, parameters[i].l);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_FLOAT:
				{
					jobject jobj = createFloat(env, parameters[i].f);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_DOUBLE:
				{
					jobject jobj = createDouble(env, parameters[i].d);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_STRING:
				{
					jstring jstr = env->NewStringUTF(parameters[i].str);
					env->SetObjectArrayElement(jparameters, i, jstr);
					env->DeleteLocalRef(jstr);
					break;
				}
				case GNATIVE_TYPE_OBJECT:
					if(parameters[i].obj != NULL)
						env->SetObjectArrayElement(jparameters, i, parameters[i].obj->obj);
					else
						env->SetObjectArrayElement(jparameters, i, NULL);
					break;
			}
		}

		gnative_Value result;

		switch (function->returnType)
		{
		case GNATIVE_TYPE_VOID:
			env->CallStaticVoidMethod(cls_, callStaticFunctionVoid_, function->cls->cls, (jint)function->index, object->obj, jparameters);
			break;
		case GNATIVE_TYPE_BOOLEAN:
			result.z = env->CallStaticBooleanMethod(cls_, callStaticFunctionBoolean_, function->cls->cls, (jint)function->index, object->obj, jparameters);
			break;
		case GNATIVE_TYPE_BYTE:
			result.b = env->CallStaticByteMethod(cls_, callStaticFunctionByte_, function->cls->cls, (jint)function->index, object->obj, jparameters);
			break;
		case GNATIVE_TYPE_CHAR:
			result.c = env->CallStaticCharMethod(cls_, callStaticFunctionChar_, function->cls->cls, (jint)function->index, object->obj, jparameters);
			break;
		case GNATIVE_TYPE_SHORT:
			result.s = env->CallStaticShortMethod(cls_, callStaticFunctionShort_, function->cls->cls, (jint)function->index, object->obj, jparameters);
			break;
		case GNATIVE_TYPE_INT:
			result.i = env->CallStaticIntMethod(cls_, callStaticFunctionInt_, function->cls->cls, (jint)function->index, object->obj, jparameters);
			break;
		case GNATIVE_TYPE_LONG:
			result.l = env->CallStaticLongMethod(cls_, callStaticFunctionLong_, function->cls->cls, (jint)function->index, object->obj, jparameters);
			break;
		case GNATIVE_TYPE_FLOAT:
			result.f = env->CallStaticFloatMethod(cls_, callStaticFunctionFloat_, function->cls->cls, (jint)function->index, object->obj, jparameters);
			break;
		case GNATIVE_TYPE_DOUBLE:
			result.d = env->CallStaticDoubleMethod(cls_, callStaticFunctionDouble_, function->cls->cls, (jint)function->index, object->obj, jparameters);
			break;
		case GNATIVE_TYPE_STRING:
		{
			jstring jstr = (jstring)env->CallStaticObjectMethod(cls_, callStaticFunctionString_, function->cls->cls, (jint)function->index, object->obj, jparameters);
			const char *str = env->GetStringUTFChars(jstr, NULL);
			result.str = strdup(str);
			env->ReleaseStringUTFChars(jstr, str);
			env->DeleteLocalRef(jstr);
			break;
		}
		case GNATIVE_TYPE_OBJECT:
		{
			
			jobject obj = env->CallStaticObjectMethod(cls_, callStaticFunctionObject_, function->cls->cls, (jint)function->index, object->obj, jparameters);
			if(obj != NULL)
			{
				jstring jClsName = (jstring)env->CallStaticObjectMethod(cls_, getClassName_, obj);
				const char *clsName = env->GetStringUTFChars(jClsName, NULL);
				gnative_Class *cls = ClassCreate(clsName);
				env->ReleaseStringUTFChars(jClsName, clsName);
				result.obj = new gnative_Object;
				result.obj->cls = cls;
				result.obj->obj = env->NewGlobalRef(obj);
				env->DeleteLocalRef(obj);
			}
			else
			{
				result.obj = NULL;
			}
			break;
		}
		}

		env->DeleteLocalRef(jparameters);

		return result;
	}


	gnative_Value FunctionStaticCall(gnative_Function *function, gnative_Value *parameters)
	{
		JNIEnv *env = g_getJNIEnv();

		jobjectArray jparameters = env->NewObjectArray(function->parameterTypes.size(), ObjectCls_, NULL);

		for (size_t i = 0; i < function->parameterTypes.size(); ++i)
		{
			switch (function->parameterTypes[i])
			{
				case GNATIVE_TYPE_VOID:
					break;
				case GNATIVE_TYPE_BOOLEAN:
				{
					jobject jobj = createBoolean(env, parameters[i].z);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_BYTE:
				{
					jobject jobj = createByte(env, parameters[i].b);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_CHAR:
				{
					jobject jobj = createChar(env, parameters[i].c);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_SHORT:
				{
					jobject jobj = createShort(env, parameters[i].s);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_INT:
				{
					jobject jobj = createInteger(env, parameters[i].i);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_LONG:
				{
					jobject jobj = createLong(env, parameters[i].l);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_FLOAT:
				{
					jobject jobj = createFloat(env, parameters[i].f);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_DOUBLE:
				{
					jobject jobj = createDouble(env, parameters[i].d);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_STRING:
				{
					jstring jstr = env->NewStringUTF(parameters[i].str);
					env->SetObjectArrayElement(jparameters, i, jstr);
					env->DeleteLocalRef(jstr);
					break;
				}
				case GNATIVE_TYPE_OBJECT:
					if(parameters[i].obj != NULL)
						env->SetObjectArrayElement(jparameters, i, parameters[i].obj->obj);
					else
						env->SetObjectArrayElement(jparameters, i, NULL);
					break;
			}
		}

		gnative_Value result;

		switch (function->returnType)
		{
		case GNATIVE_TYPE_VOID:
			env->CallStaticVoidMethod(cls_, callStaticFunctionVoid_, function->cls->cls, (jint)function->index, NULL, jparameters);
			break;
		case GNATIVE_TYPE_BOOLEAN:
			result.z = env->CallStaticBooleanMethod(cls_, callStaticFunctionBoolean_, function->cls->cls, (jint)function->index, NULL, jparameters);
			break;
		case GNATIVE_TYPE_BYTE:
			result.b = env->CallStaticByteMethod(cls_, callStaticFunctionByte_, function->cls->cls, (jint)function->index, NULL, jparameters);
			break;
		case GNATIVE_TYPE_CHAR:
			result.c = env->CallStaticCharMethod(cls_, callStaticFunctionChar_, function->cls->cls, (jint)function->index, NULL, jparameters);
			break;
		case GNATIVE_TYPE_SHORT:
			result.s = env->CallStaticShortMethod(cls_, callStaticFunctionShort_, function->cls->cls, (jint)function->index, NULL, jparameters);
			break;
		case GNATIVE_TYPE_INT:
			result.i = env->CallStaticIntMethod(cls_, callStaticFunctionInt_, function->cls->cls, (jint)function->index, NULL, jparameters);
			break;
		case GNATIVE_TYPE_LONG:
			result.l = env->CallStaticLongMethod(cls_, callStaticFunctionLong_, function->cls->cls, (jint)function->index, NULL, jparameters);
			break;
		case GNATIVE_TYPE_FLOAT:
			result.f = env->CallStaticFloatMethod(cls_, callStaticFunctionFloat_, function->cls->cls, (jint)function->index, NULL, jparameters);
			break;
		case GNATIVE_TYPE_DOUBLE:
			result.d = env->CallStaticDoubleMethod(cls_, callStaticFunctionDouble_, function->cls->cls, (jint)function->index, NULL, jparameters);
			break;
		case GNATIVE_TYPE_STRING:
		{
			jstring jstr = (jstring)env->CallStaticObjectMethod(cls_, callStaticFunctionString_, function->cls->cls, (jint)function->index, NULL, jparameters);
			const char *str = env->GetStringUTFChars(jstr, NULL);
			result.str = strdup(str);
			env->ReleaseStringUTFChars(jstr, str);
			env->DeleteLocalRef(jstr);
			break;
		}
		case GNATIVE_TYPE_OBJECT:
		{
			jobject obj = env->CallStaticObjectMethod(cls_, callStaticFunctionObject_, function->cls->cls, (jint)function->index, NULL, jparameters);
			if(obj != NULL)
			{
				jstring jClsName = (jstring)env->CallStaticObjectMethod(cls_, getClassName_, obj);
				const char *clsName = env->GetStringUTFChars(jClsName, NULL);
				gnative_Class *cls = ClassCreate(clsName);
				env->ReleaseStringUTFChars(jClsName, clsName);
				result.obj = new gnative_Object;
				result.obj->cls = cls;
				result.obj->obj = env->NewGlobalRef(obj);
				env->DeleteLocalRef(obj);
			}
			else
			{
				result.obj = NULL;
			}
			break;
		}
		}

		env->DeleteLocalRef(jparameters);

		return result;
	}

	gnative_Type FunctionGetReturnType(gnative_Function *function)
	{
		return function->returnType;
	}

	gnative_Type *FunctionGetParameterTypes(gnative_Function *function)
	{
		return function->parameterTypes.empty() ? NULL : &function->parameterTypes[0];
	}

	gnative_Type *ConstructorGetParameterTypes(gnative_Constructor *constructor)
	{
		return constructor->parameterTypes.empty() ? NULL : &constructor->parameterTypes[0];
	}

	int FunctionGetParameterCount(gnative_Function *function)
	{
		return function->parameterTypes.size();
	}

	int ConstructorGetParameterCount(gnative_Constructor *constructor)
	{
		return constructor->parameterTypes.size();
	}

	gnative_Type FieldGetReturnType(gnative_Field *field)
	{
		return field->returnType;
	}

	gnative_Value FieldGetValue(gnative_Field *field, gnative_Object *object)
	{
		JNIEnv *env = g_getJNIEnv();
		gnative_Value result;

		switch (field->returnType)
		{
		case GNATIVE_TYPE_VOID:
			env->CallStaticVoidMethod(cls_, getFieldVoid_, field->cls->cls, (jint)field->index, object->obj);
			break;
		case GNATIVE_TYPE_BOOLEAN:
			result.z = env->CallStaticBooleanMethod(cls_, getFieldBoolean_, field->cls->cls, (jint)field->index, object->obj);
			break;
		case GNATIVE_TYPE_BYTE:
			result.b = env->CallStaticByteMethod(cls_, getFieldByte_, field->cls->cls, (jint)field->index, object->obj);
			break;
		case GNATIVE_TYPE_CHAR:
			result.c = env->CallStaticCharMethod(cls_, getFieldChar_, field->cls->cls, (jint)field->index, object->obj);
			break;
		case GNATIVE_TYPE_SHORT:
			result.s = env->CallStaticShortMethod(cls_, getFieldShort_, field->cls->cls, (jint)field->index, object->obj);
			break;
		case GNATIVE_TYPE_INT:
			result.i = env->CallStaticIntMethod(cls_, getFieldInt_, field->cls->cls, (jint)field->index, object->obj);
			break;
		case GNATIVE_TYPE_LONG:
			result.l = env->CallStaticLongMethod(cls_, getFieldLong_, field->cls->cls, (jint)field->index, object->obj);
			break;
		case GNATIVE_TYPE_FLOAT:
			result.f = env->CallStaticFloatMethod(cls_, getFieldFloat_, field->cls->cls, (jint)field->index, object->obj);
			break;
		case GNATIVE_TYPE_DOUBLE:
			result.d = env->CallStaticDoubleMethod(cls_, getFieldDouble_, field->cls->cls, (jint)field->index, object->obj);
			break;
		case GNATIVE_TYPE_STRING:
		{
			jstring jstr = (jstring)env->CallStaticObjectMethod(cls_, getFieldString_, field->cls->cls, (jint)field->index, object->obj);
			const char *str = env->GetStringUTFChars(jstr, NULL);
			result.str = strdup(str);
			env->ReleaseStringUTFChars(jstr, str);
			env->DeleteLocalRef(jstr);
			break;
		}
		case GNATIVE_TYPE_OBJECT:
		{
			jobject obj = env->CallStaticObjectMethod(cls_, getFieldObject_, field->cls->cls, (jint)field->index, object->obj);
			if(obj != NULL)
			{
				jstring jClsName = (jstring)env->CallStaticObjectMethod(cls_, getClassName_, obj);
				const char *clsName = env->GetStringUTFChars(jClsName, NULL);
				gnative_Class *cls = ClassCreate(clsName);
				env->ReleaseStringUTFChars(jClsName, clsName);
				result.obj = new gnative_Object;
				result.obj->cls = cls;
				result.obj->obj = env->NewGlobalRef(obj);
				env->DeleteLocalRef(obj);
			}
			else
			{
				result.obj = NULL;
			}
			break;
		}
		}

		return result;
	}

	gnative_Value FieldGetStaticValue(gnative_Field *field)
	{
		JNIEnv *env = g_getJNIEnv();
		gnative_Value result;

		switch (field->returnType)
		{
		case GNATIVE_TYPE_VOID:
			env->CallStaticVoidMethod(cls_, getFieldVoid_, field->cls->cls, (jint)field->index, NULL);
			break;
		case GNATIVE_TYPE_BOOLEAN:
			result.z = env->CallStaticBooleanMethod(cls_, getFieldBoolean_, field->cls->cls, (jint)field->index, NULL);
			break;
		case GNATIVE_TYPE_BYTE:
			result.b = env->CallStaticByteMethod(cls_, getFieldByte_, field->cls->cls, (jint)field->index, NULL);
			break;
		case GNATIVE_TYPE_CHAR:
			result.c = env->CallStaticCharMethod(cls_, getFieldChar_, field->cls->cls, (jint)field->index, NULL);
			break;
		case GNATIVE_TYPE_SHORT:
			result.s = env->CallStaticShortMethod(cls_, getFieldShort_, field->cls->cls, (jint)field->index, NULL);
			break;
		case GNATIVE_TYPE_INT:
			result.i = env->CallStaticIntMethod(cls_, getFieldInt_, field->cls->cls, (jint)field->index, NULL);
			break;
		case GNATIVE_TYPE_LONG:
			result.l = env->CallStaticLongMethod(cls_, getFieldLong_, field->cls->cls, (jint)field->index, NULL);
			break;
		case GNATIVE_TYPE_FLOAT:
			result.f = env->CallStaticFloatMethod(cls_, getFieldFloat_, field->cls->cls, (jint)field->index, NULL);
			break;
		case GNATIVE_TYPE_DOUBLE:
			result.d = env->CallStaticDoubleMethod(cls_, getFieldDouble_, field->cls->cls, (jint)field->index, NULL);
			break;
		case GNATIVE_TYPE_STRING:
		{
			jstring jstr = (jstring)env->CallStaticObjectMethod(cls_, getFieldString_, field->cls->cls, (jint)field->index, NULL);
			const char *str = env->GetStringUTFChars(jstr, NULL);
			result.str = strdup(str);
			env->ReleaseStringUTFChars(jstr, str);
			env->DeleteLocalRef(jstr);
			break;
		}
		case GNATIVE_TYPE_OBJECT:
		{
			jobject obj = env->CallStaticObjectMethod(cls_, getFieldObject_, field->cls->cls, (jint)field->index, NULL);
			if(obj != NULL)
			{
				jstring jClsName = (jstring)env->CallStaticObjectMethod(cls_, getClassName_, obj);
				const char *clsName = env->GetStringUTFChars(jClsName, NULL);
				gnative_Class *cls = ClassCreate(clsName);
				env->ReleaseStringUTFChars(jClsName, clsName);
				result.obj = new gnative_Object;
				result.obj->cls = cls;
				result.obj->obj = env->NewGlobalRef(obj);
				env->DeleteLocalRef(obj);
			}
			else
			{
				result.obj = NULL;
			}
			break;
		}
		}

		return result;
	}
	
	void FieldSetValue(gnative_Field *field, gnative_Object *object, gnative_Value *value)
	{
		JNIEnv *env = g_getJNIEnv();

		switch (field->returnType)
		{
		case GNATIVE_TYPE_VOID:
			env->CallStaticVoidMethod(cls_, setFieldVoid_, field->cls->cls, (jint)field->index, object->obj);
			break;
		case GNATIVE_TYPE_BOOLEAN:
			env->CallStaticVoidMethod(cls_, setFieldBoolean_, field->cls->cls, (jint)field->index, object->obj, (jboolean)value->z);
			break;
		case GNATIVE_TYPE_BYTE:
			env->CallStaticVoidMethod(cls_, setFieldByte_, field->cls->cls, (jint)field->index, object->obj, (jbyte)value->b);
			break;
		case GNATIVE_TYPE_CHAR:
			env->CallStaticVoidMethod(cls_, setFieldChar_, field->cls->cls, (jint)field->index, object->obj, (jchar)value->c);
			break;
		case GNATIVE_TYPE_SHORT:
			env->CallStaticVoidMethod(cls_, setFieldShort_, field->cls->cls, (jint)field->index, object->obj, (jshort)value->s);
			break;
		case GNATIVE_TYPE_INT:
			env->CallStaticVoidMethod(cls_, setFieldInt_, field->cls->cls, (jint)field->index, object->obj, (jint)value->i);
			break;
		case GNATIVE_TYPE_LONG:
			env->CallStaticVoidMethod(cls_, setFieldLong_, field->cls->cls, (jint)field->index, object->obj, (jlong)value->l);
			break;
		case GNATIVE_TYPE_FLOAT:
			env->CallStaticVoidMethod(cls_, setFieldFloat_, field->cls->cls, (jint)field->index, object->obj, (jfloat)value->f);
			break;
		case GNATIVE_TYPE_DOUBLE:
			env->CallStaticVoidMethod(cls_, setFieldDouble_, field->cls->cls, (jint)field->index, object->obj, (jdouble)value->d);
			break;
		case GNATIVE_TYPE_STRING:
		{
			jstring jstrValue = env->NewStringUTF(value->str);
			env->CallStaticVoidMethod(cls_, setFieldString_, field->cls->cls, (jint)field->index, object->obj, jstrValue);
			env->DeleteLocalRef(jstrValue);
			break;
		}
		case GNATIVE_TYPE_OBJECT:
			env->CallStaticVoidMethod(cls_, setFieldObject_, field->cls->cls, (jint)field->index, object->obj, value->obj->obj);
			break;
		}
	}
	
	void FieldSetStaticValue(gnative_Field *field, gnative_Value *value)
	{
		JNIEnv *env = g_getJNIEnv();

		switch (field->returnType)
		{
		case GNATIVE_TYPE_VOID:
			env->CallStaticVoidMethod(cls_, setFieldVoid_, field->cls->cls, (jint)field->index, NULL);
			break;
		case GNATIVE_TYPE_BOOLEAN:
			env->CallStaticVoidMethod(cls_, setFieldBoolean_, field->cls->cls, (jint)field->index, NULL, (jboolean)value->z);
			break;
		case GNATIVE_TYPE_BYTE:
			env->CallStaticVoidMethod(cls_, setFieldByte_, field->cls->cls, (jint)field->index, NULL, (jbyte)value->b);
			break;
		case GNATIVE_TYPE_CHAR:
			env->CallStaticVoidMethod(cls_, setFieldChar_, field->cls->cls, (jint)field->index, NULL, (jchar)value->c);
			break;
		case GNATIVE_TYPE_SHORT:
			env->CallStaticVoidMethod(cls_, setFieldShort_, field->cls->cls, (jint)field->index, NULL, (jshort)value->s);
			break;
		case GNATIVE_TYPE_INT:
			env->CallStaticVoidMethod(cls_, setFieldInt_, field->cls->cls, (jint)field->index, NULL, (jint)value->i);
			break;
		case GNATIVE_TYPE_LONG:
			env->CallStaticVoidMethod(cls_, setFieldLong_, field->cls->cls, (jint)field->index, NULL, (jlong)value->l);
			break;
		case GNATIVE_TYPE_FLOAT:
			env->CallStaticVoidMethod(cls_, setFieldFloat_, field->cls->cls, (jint)field->index, NULL, (jfloat)value->f);
			break;
		case GNATIVE_TYPE_DOUBLE:
			env->CallStaticVoidMethod(cls_, setFieldDouble_, field->cls->cls, (jint)field->index, NULL, (jdouble)value->d);
			break;
		case GNATIVE_TYPE_STRING:
		{
			jstring jstrValue = env->NewStringUTF(value->str);
			env->CallStaticVoidMethod(cls_, setFieldString_, field->cls->cls, (jint)field->index, NULL, jstrValue);
			env->DeleteLocalRef(jstrValue);
			break;
		}
		case GNATIVE_TYPE_OBJECT:
			env->CallStaticVoidMethod(cls_, setFieldObject_, field->cls->cls, (jint)field->index, NULL, value->obj->obj);
			break;
		}
	}

	gnative_Object *CreateObject(gnative_Constructor *constructor, gnative_Value *parameters)
	{
		JNIEnv *env = g_getJNIEnv();

		jobjectArray jparameters = env->NewObjectArray(constructor->parameterTypes.size(), ObjectCls_, NULL);
		for (size_t i = 0; i < constructor->parameterTypes.size(); ++i)
		{
			switch (constructor->parameterTypes[i])
			{
				case GNATIVE_TYPE_VOID:
					break;
				case GNATIVE_TYPE_BOOLEAN:
				{
					jobject jobj = createBoolean(env, parameters[i].z);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_BYTE:
				{
					jobject jobj = createByte(env, parameters[i].b);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_CHAR:
				{
					jobject jobj = createChar(env, parameters[i].c);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_SHORT:
				{
					jobject jobj = createShort(env, parameters[i].s);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_INT:
				{
					jobject jobj = createInteger(env, parameters[i].i);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_LONG:
				{
					jobject jobj = createLong(env, parameters[i].l);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_FLOAT:
				{
					jobject jobj = createFloat(env, parameters[i].f);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_DOUBLE:
				{
					jobject jobj = createDouble(env, parameters[i].d);
					env->SetObjectArrayElement(jparameters, i, jobj);
					env->DeleteLocalRef(jobj);
					break;
				}
				case GNATIVE_TYPE_STRING:
				{
					jstring jstr = env->NewStringUTF(parameters[i].str);
					env->SetObjectArrayElement(jparameters, i, jstr);
					env->DeleteLocalRef(jstr);
					break;
				}
				case GNATIVE_TYPE_OBJECT:
					if(parameters[i].obj != NULL)
						env->SetObjectArrayElement(jparameters, i, parameters[i].obj->obj);
					else
						env->SetObjectArrayElement(jparameters, i, NULL);
					break;
			}
		}

		jobject ob = env->CallStaticObjectMethod(cls_, createObject_, constructor->cls->cls, (jint)constructor->index, jparameters);

		gnative_Object *obj = new gnative_Object;
		obj->cls = constructor->cls;
		obj->obj = env->NewGlobalRef(ob);
		env->DeleteLocalRef(ob);

		env->DeleteLocalRef(jparameters);

		return obj;
	}

	gnative_Object *CreateArray(gnative_Class *cls, gnative_Object **objects, int length)
	{
		JNIEnv *env = g_getJNIEnv();
		
		jobjectArray jparameters = env->NewObjectArray(length, cls->cls, NULL);
		for(int i = 0; i < length; i++)
		{
			gnative_Object *obj = *objects;
			env->SetObjectArrayElement(jparameters, i, obj->obj);
			objects++;
		}
		
		gnative_Object *ob = new gnative_Object;
		std::string classArrayName = "[L" + cls->name + ";";
		gnative_Class *clsArray = ClassCreate(classArrayName.c_str());
		ob->cls = clsArray;
		ob->obj = env->NewGlobalRef(jparameters);
		env->DeleteLocalRef(jparameters);
		return ob;
	}

	gnative_Object *CreatePrimitiveArray(const char *type, gnative_Value *values, int length)
	{
		JNIEnv *env = g_getJNIEnv();

		gnative_Object *ob = new gnative_Object;

		if(strcmp("bool", type) == 0)
		{
			jbooleanArray jparameters = env->NewBooleanArray(length);
			jboolean fill[length];
			for(int i = 0; i < length; i++)
			{
				fill[i] = values->z;
				values++;
			}
			env->SetBooleanArrayRegion(jparameters, 0, length, fill);
			std::string stype = type;
			std::string classArrayName = "[Z";
			gnative_Class *cls = new gnative_Class;
			cls->name = classArrayName.c_str();
			ob->cls = cls;
			ob->obj = env->NewGlobalRef(jparameters);
			env->DeleteLocalRef(jparameters);
		}
		else if(strcmp("byte", type) == 0)
		{
			jbyteArray jparameters = env->NewByteArray(length);
			jbyte fill[length];
			for(int i = 0; i < length; i++)
			{
				fill[i] = values->b;
				values++;
			}
			env->SetByteArrayRegion(jparameters, 0, length, fill);
			std::string stype = type;
			std::string classArrayName = "[B";
			gnative_Class *cls = new gnative_Class;
			cls->name = classArrayName.c_str();
			ob->cls = cls;
			ob->obj = env->NewGlobalRef(jparameters);
			env->DeleteLocalRef(jparameters);	
		}
		else if(strcmp("char", type) == 0)
		{
			jcharArray jparameters = env->NewCharArray(length);
			jchar fill[length];
			for(int i = 0; i < length; i++)
			{
				fill[i] = values->b;
				values++;
			}
			env->SetCharArrayRegion(jparameters, 0, length, fill);
			std::string stype = type;
			std::string classArrayName = "[C";
			gnative_Class *cls = new gnative_Class;
			cls->name = classArrayName.c_str();
			ob->cls = cls;
			ob->obj = env->NewGlobalRef(jparameters);
			env->DeleteLocalRef(jparameters);	
		}
		else if(strcmp("short", type) == 0)
		{
			jshortArray jparameters = env->NewShortArray(length);
			jshort fill[length];
			for(int i = 0; i < length; i++)
			{
				fill[i] = values->s;
				values++;
			}
			env->SetShortArrayRegion(jparameters, 0, length, fill);
			std::string stype = type;
			std::string classArrayName = "[S";
			gnative_Class *cls = new gnative_Class;
			cls->name = classArrayName.c_str();
			ob->cls = cls;
			ob->obj = env->NewGlobalRef(jparameters);
			env->DeleteLocalRef(jparameters);	
		}
		else if(strcmp("int", type) == 0)
		{
			jintArray jparameters = env->NewIntArray(length);
			jint fill[length];
			for(int i = 0; i < length; i++)
			{
				fill[i] = values->i;
				values++;
			}
			env->SetIntArrayRegion(jparameters, 0, length, fill);
			std::string stype = type;
			std::string classArrayName = "[I";
			gnative_Class *cls = new gnative_Class;
			cls->name = classArrayName.c_str();
			ob->cls = cls;
			ob->obj = env->NewGlobalRef(jparameters);
			env->DeleteLocalRef(jparameters);	
		}
		else if(strcmp("long", type) == 0)
		{
			jlongArray jparameters = env->NewLongArray(length);
			jlong fill[length];
			for(int i = 0; i < length; i++)
			{
				fill[i] = values->l;
				values++;
			}
			env->SetLongArrayRegion(jparameters, 0, length, fill);
			std::string stype = type;
			std::string classArrayName = "[J";
			gnative_Class *cls = new gnative_Class;
			cls->name = classArrayName.c_str();
			ob->cls = cls;
			ob->obj = env->NewGlobalRef(jparameters);
			env->DeleteLocalRef(jparameters);	
		}
		else if(strcmp("float", type) == 0)
		{
			jfloatArray jparameters = env->NewFloatArray(length);
			jfloat fill[length];
			for(int i = 0; i < length; i++)
			{
				fill[i] = values->f;
				values++;
			}
			env->SetFloatArrayRegion(jparameters, 0, length, fill);
			std::string stype = type;
			std::string classArrayName = "[F";
			gnative_Class *cls = new gnative_Class;
			cls->name = classArrayName.c_str();
			ob->cls = cls;
			ob->obj = env->NewGlobalRef(jparameters);
			env->DeleteLocalRef(jparameters);	
		}
		else if(strcmp("double", type) == 0)
		{
			jdoubleArray jparameters = env->NewDoubleArray(length);
			jdouble fill[length];
			for(int i = 0; i < length; i++)
			{
				fill[i] = values->d;
				values++;
			}
			env->SetDoubleArrayRegion(jparameters, 0, length, fill);
			std::string stype = type;
			std::string classArrayName = "[D";
			gnative_Class *cls = new gnative_Class;
			cls->name = classArrayName.c_str();
			ob->cls = cls;
			ob->obj = env->NewGlobalRef(jparameters);
			env->DeleteLocalRef(jparameters);	
		}
		else if(strcmp("string", type) == 0)
		{
			jclass StringCls = env->FindClass("java.lang.String");
			jobjectArray jparameters = env->NewObjectArray(length, StringCls, NULL);
			env->DeleteLocalRef(StringCls);
			for(int i = 0; i < length; i++)
			{
				jstring jstr = env->NewStringUTF(values->str);
				env->SetObjectArrayElement(jparameters, i, jstr);
				env->DeleteLocalRef(jstr);
				values++;
			}
			gnative_Class *clsArray = ClassCreate("[Ljava.lang.String;");
			ob->cls = clsArray;
			ob->obj = env->NewGlobalRef(jparameters);
			env->DeleteLocalRef(jparameters);	
		}
		return ob;
	}

	gnative_Object *CreateProxy(const char *className, long data)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jstr = env->NewStringUTF(className);

		jobject ob = env->CallStaticObjectMethod(cls_, createProxy_, jstr, (jlong)data);

		env->DeleteLocalRef(jstr);

		gnative_Object *obj = new gnative_Object;
		gnative_Class *cls = ClassCreate("java.lang.Object");
		obj->cls = cls;
		obj->obj = env->NewGlobalRef(ob);
		env->DeleteLocalRef(ob);

		return obj;
	}

	void DeleteObject(gnative_Object *obj)
	{
		JNIEnv *env = g_getJNIEnv();
		env->DeleteGlobalRef(obj->obj);
		delete obj;
	}

	g_bool ClassIsInstance(gnative_Class *cls, gnative_Object *obj)
	{
		JNIEnv *env = g_getJNIEnv();
		return env->CallStaticBooleanMethod(cls_, isInstance_, cls->cls, obj->obj);
	}

	g_bool ClassIsSubclass(const char *cls, const char *subclass)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jcls = env->NewStringUTF(cls);
		jstring jsubcls = env->NewStringUTF(subclass);
		g_bool res = (g_bool)env->CallStaticBooleanMethod(cls_, isSubclass_, jcls, jsubcls);
		env->DeleteLocalRef(jcls);
		env->DeleteLocalRef(jsubcls);
		return res;
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
				classes[i] = (types[i] == GNATIVE_TYPE_OBJECT) ? ClassCreate(names[i].c_str()) : NULL;
		}

		return classes.empty() ? NULL : &classes[0];
	}

	gnative_Class **ConstructorGetParameterClasses(gnative_Constructor *constructor)
	{
		std::vector<gnative_Type> &types = constructor->parameterTypes;
		std::vector<std::string> &names = constructor->parameterNames;
		std::vector<gnative_Class*> &classes = constructor->parameterClasses;

		if (classes.empty())
		{
			classes.resize(types.size());
			for (std::size_t i = 0; i < classes.size(); ++i)
				classes[i] = (types[i] == GNATIVE_TYPE_OBJECT) ? ClassCreate(names[i].c_str()) : NULL;
		}

		return classes.empty() ? NULL : &classes[0];
	}

	const char *ClassGetName(gnative_Class *cls)
	{
		return cls->name.c_str();
	}

	const char *ObjectGetName(gnative_Object *obj)
	{
		return obj->cls->name.c_str();
	}

	void invokeMethod(jstring jmethodName, jobjectArray jargs, jobjectArray jargTypes, jlong proxy)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *methodName = env->GetStringUTFChars(jmethodName, NULL);
		int length = env->GetArrayLength(jargs);
		std::vector<gnative_Value> args;
		int types[length];
		for (int i = 0; i < length; ++i)
		{
			gnative_Value result;
			jstring jparameter = (jstring)env->GetObjectArrayElement(jargTypes, i);
			const char *parameter = env->GetStringUTFChars(jparameter, NULL);
			int type = getType(parameter);
			types[i] = type;
			switch (type)
			{
			case GNATIVE_TYPE_BOOLEAN:
				result.z = env->CallStaticBooleanMethod(cls_, getBoolean_, i, jargs);
				break;
			case GNATIVE_TYPE_BYTE:
				result.b = env->CallStaticByteMethod(cls_, getByte_, i, jargs);
				break;
			case GNATIVE_TYPE_CHAR:
				result.c = env->CallStaticCharMethod(cls_, getChar_, i, jargs);
				break;
			case GNATIVE_TYPE_SHORT:
				result.s = env->CallStaticShortMethod(cls_, getShort_, i, jargs);
				break;
			case GNATIVE_TYPE_INT:
				result.i = env->CallStaticIntMethod(cls_, getInt_, i, jargs);
				break;
			case GNATIVE_TYPE_LONG:
				result.l = env->CallStaticLongMethod(cls_, getLong_, i, jargs);
				break;
			case GNATIVE_TYPE_FLOAT:
				result.f = env->CallStaticFloatMethod(cls_, getFloat_, i, jargs);
				break;
			case GNATIVE_TYPE_DOUBLE:
				result.d = env->CallStaticDoubleMethod(cls_, getDouble_, i, jargs);
				break;
			case GNATIVE_TYPE_STRING:
			{
				jstring jstr = (jstring)env->GetObjectArrayElement(jargs, i);
				const char *str = env->GetStringUTFChars(jstr, NULL);
				result.str = strdup(str);
				env->ReleaseStringUTFChars(jstr, str);
				env->DeleteLocalRef(jstr);
				break;
			}
			case GNATIVE_TYPE_OBJECT:
			{
				jobject obj = env->GetObjectArrayElement(jargs, i);
				if(obj != NULL)
				{
					jstring jClsName = (jstring)env->CallStaticObjectMethod(cls_, getClassName_, obj);
					const char *clsName = env->GetStringUTFChars(jClsName, NULL);
					gnative_Class *cls = ClassCreate(clsName);
					env->ReleaseStringUTFChars(jClsName, clsName);
					result.obj = new gnative_Object;
					result.obj->cls = cls;
					result.obj->obj = env->NewGlobalRef(obj);
					env->DeleteLocalRef(obj);
				}
				else
				{
					result.obj = NULL;
				}
				break;
			}
			}
			args.push_back(result);
			env->ReleaseStringUTFChars(jparameter, parameter);

			env->DeleteLocalRef(jparameter);
		}
		gnative_InvokeMethod(methodName, length, &args[0], types, proxy);
		//gnative_InvokeMethod(methodName, proxy);
		env->ReleaseStringUTFChars(jmethodName, methodName);
		env->DeleteLocalRef(jmethodName);
	}

private:
	std::map<std::string, gnative_Class*> classes_;

	jclass cls_;

	jclass ObjectCls_;
	jclass BooleanCls_;
	jmethodID BooleanInit_;
	jclass ByteCls_;
	jmethodID ByteInit_;
	jclass CharCls_;
	jmethodID CharInit_;
	jclass ShortCls_;
	jmethodID ShortInit_;
	jclass IntegerCls_;
	jmethodID IntegerInit_;
	jclass LongCls_;
	jmethodID LongInit_;
	jclass FloatCls_;
	jmethodID FloatInit_;
	jclass DoubleCls_;
	jmethodID DoubleInit_;

	jmethodID getClassName_;
	jmethodID getClassConstructorsLength_;
	jmethodID getClassConstructorParameters_;
	jmethodID getClassMethods_;
	jmethodID getClassMethodParameters_;
	jmethodID getClassFields_;
	jmethodID getClassFieldTypes_;
	
	jmethodID createObject_;
	jmethodID createProxy_;
	
	jmethodID callStaticFunctionObject_;
	jmethodID callStaticFunctionVoid_;
	jmethodID callStaticFunctionBoolean_;
	jmethodID callStaticFunctionByte_;
	jmethodID callStaticFunctionChar_;
	jmethodID callStaticFunctionShort_;
	jmethodID callStaticFunctionInt_;
	jmethodID callStaticFunctionLong_;
	jmethodID callStaticFunctionFloat_;
	jmethodID callStaticFunctionDouble_;
	jmethodID callStaticFunctionString_;
	
	jmethodID getFieldObject_;
	jmethodID getFieldVoid_;
	jmethodID getFieldBoolean_;
	jmethodID getFieldByte_;
	jmethodID getFieldChar_;
	jmethodID getFieldShort_;
	jmethodID getFieldInt_;
	jmethodID getFieldLong_;
	jmethodID getFieldFloat_;
	jmethodID getFieldDouble_;
	jmethodID getFieldString_;
	
	jmethodID setFieldObject_;
	jmethodID setFieldVoid_;
	jmethodID setFieldBoolean_;
	jmethodID setFieldByte_;
	jmethodID setFieldChar_;
	jmethodID setFieldShort_;
	jmethodID setFieldInt_;
	jmethodID setFieldLong_;
	jmethodID setFieldFloat_;
	jmethodID setFieldDouble_;
	jmethodID setFieldString_;

	jmethodID getBoolean_;
	jmethodID getByte_;
	jmethodID getChar_;
	jmethodID getShort_;
	jmethodID getInt_;
	jmethodID getLong_;
	jmethodID getFloat_;
	jmethodID getDouble_;

	jmethodID isInstance_;
	jmethodID isSubclass_;
};

extern "C" {

void Java_com_giderosmobile_android_plugins_bridge_GBridge_onInvoke(JNIEnv *env, jclass clz, jstring jmethodName, jobjectArray jargs, jobjectArray jargTypes, jlong proxy, jlong data)
{
	((GNative*)data)->invokeMethod(jmethodName, jargs, jargTypes, proxy);
}

}

static GNative *s_nat = NULL;

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

gnative_Class *gnative_ClassCreate(const char *className)
{
	return s_nat->ClassCreate(className);
}

gnative_Constructor **gnative_ClassGetConstructors(gnative_Class *cls)
{
	return s_nat->ClassGetConstructors(cls);
}

gnative_Function **gnative_ClassGetStaticFunctionsByName(gnative_Class *cls, const char *name)
{
	return s_nat->ClassGetStaticFunctionsByName(cls, name);
}

gnative_Function **gnative_ObjectGetFunctionsByName(gnative_Object *obj, const char *name)
{
	return s_nat->ObjectGetFunctionsByName(obj, name);
}
gnative_Field **gnative_ClassGetFieldsByName(gnative_Class *cls, const char *name)
{
	return s_nat->ClassGetFieldsByName(cls, name);
}

gnative_Field **gnative_ObjectGetFieldsByName(gnative_Object *obj, const char *name)
{
	return s_nat->ObjectGetFieldsByName(obj, name);
}

gnative_Value gnative_FunctionStaticCall(gnative_Function *function, gnative_Value *parameters)
{
	return s_nat->FunctionStaticCall(function, parameters);
}

gnative_Value gnative_FunctionCall(gnative_Function *function, gnative_Object *object, gnative_Value *parameters)
{
	return s_nat->FunctionCall(function, object, parameters);
}

gnative_Type gnative_FunctionGetReturnType(gnative_Function *function)
{
	return s_nat->FunctionGetReturnType(function);
}
gnative_Type *gnative_FunctionGetParameterTypes(gnative_Function *function)
{
	return s_nat->FunctionGetParameterTypes(function);
}

gnative_Type *gnative_ConstructorGetParameterTypes(gnative_Constructor *constructor)
{
	return s_nat->ConstructorGetParameterTypes(constructor);
}

int gnative_FunctionGetParameterCount(gnative_Function *function)
{
	return s_nat->FunctionGetParameterCount(function);
}

int gnative_ConstructorGetParameterCount(gnative_Constructor *constructor)
{
	return s_nat->ConstructorGetParameterCount(constructor);
}

gnative_Object *gnative_CreateObject(gnative_Constructor *constructor, gnative_Value *parameters)
{
	return s_nat->CreateObject(constructor, parameters);
}

void gnative_DeleteObject(gnative_Object *obj)
{
	s_nat->DeleteObject(obj);
}

g_bool gnative_ClassIsInstance(gnative_Class *cls, gnative_Object *obj)
{
	return s_nat->ClassIsInstance(cls, obj);
}

g_bool gnative_ClassIsSubclass(const char *cls, const char *subclass)
{
	return s_nat->ClassIsSubclass(cls, subclass);
}

gnative_Class **gnative_FunctionGetParameterClasses(gnative_Function *function)
{
	return s_nat->FunctionGetParameterClasses(function);
}

gnative_Class **gnative_ConstructorGetParameterClasses(gnative_Constructor *constructor)
{
	return s_nat->ConstructorGetParameterClasses(constructor);
}

const char *gnative_ClassGetName(gnative_Class *cls)
{
	return s_nat->ClassGetName(cls);
}

const char *gnative_ObjectGetName(gnative_Object *obj)
{
	return s_nat->ObjectGetName(obj);
}

gnative_Type gnative_FieldGetReturnType(gnative_Field *field)
{
	return s_nat->FieldGetReturnType(field);
}

gnative_Value gnative_FieldGetValue(gnative_Field *field, gnative_Object *object)
{
	return s_nat->FieldGetValue(field, object);
}

void gnative_FieldSetValue(gnative_Field *field, gnative_Object *object, gnative_Value *value)
{
	s_nat->FieldSetValue(field, object, value);
}

gnative_Value gnative_FieldGetStaticValue(gnative_Field *field)
{
	return s_nat->FieldGetStaticValue(field);
}

void gnative_FieldSetStaticValue(gnative_Field *field, gnative_Value *value)
{
	s_nat->FieldSetStaticValue(field, value);
}

gnative_Object *gnative_CreateProxy(const char *className, long data)
{
	return s_nat->CreateProxy(className, data);
}

gnative_Object *gnative_CreateArray(gnative_Class *cls, gnative_Object **obj, int length)
{
	return s_nat->CreateArray(cls, obj, length);
}

gnative_Object *gnative_CreatePrimitiveArray(const char *type, gnative_Value *values, int length)
{
	return s_nat->CreatePrimitiveArray(type, values, length);
}

}
