#ifndef LUA_SCRIPT_H
#define LUA_SCRIPT_H

#include <QtCore>
#include <new>
#include "../tags.h"
#include "lua_src/lua.hpp"

struct PassByValue
{
    qint8 qint8Val;
    quint8 quint8Val;
    qint16 qint16Val;
    quint16 quint16Val;
    qint32 qint32Val;
    quint32 quint32Val;
    qint64 qint64Val;
    quint64 quint64Val;
    float floatVal;
    double doubleVal;
    QString stringVal;
    Tag* tagVal;
};

struct ReturnByValue
{
    qint8 qint8Val;
    quint8 quint8Val;
    qint16 qint16Val;
    quint16 quint16Val;
    qint32 qint32Val;
    quint32 quint32Val;
    qint64 qint64Val;
    quint64 quint64Val;
    float floatVal;
    double doubleVal;
    QString stringVal;
    bool boolVal;
    Tag* tagVal;
};

class LuaGlobals
{
public:
    static int Lua_ReadOnly(lua_State* L)
    {
        luaL_error(L, "Attempted to write to a read-only variable");
        return 0;
    }
};

class LuaScript
{
public:

    LuaScript(QString scriptPath) : scriptPath(scriptPath), running(false), error(false)
    {
        if(scriptPath.isEmpty())
            return;
        L = luaL_newstate();

        luaL_openlibs(L);

        lua_pushstring(L, QFileInfo(scriptPath).absolutePath().toStdString().c_str());
        lua_setglobal(L, "BASE_PATH");

        bindGlobals();
        bindObject(&SharedObject::staticMetaObject);
        bindObject(&Tag::staticMetaObject);
        bindObject(&TagInt::staticMetaObject);
        bindObject(&TagContainer::staticMetaObject);
        bindObject(&TagCompound::staticMetaObject);
        bindObject(&TagList::staticMetaObject);
    }

    ~LuaScript()
    {
        stop();
    }

    bool start()
    {
        QFile scriptFile(scriptPath);

        if(scriptFile.exists())
        {
            if(!scriptFile.open(QIODevice::ReadOnly))
            {
                return false;
            }

            QByteArray scriptData = scriptFile.readAll();
            scriptFile.close();

            int status = luaL_dostring(L, scriptData.toStdString().c_str());
            if(status != LUA_OK)
            {
                qDebug() << "ERROR-" + QString(lua_tostring(L, -1));
                return false;
            }
            else
            {
                running = true;
                return true;
            }
        }

        return false;
    }

    void stop()
    {
        if(running)
            lua_close(L);
        running = false;
    }

    void bindGlobals()
    {
        //setup tag types
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "TYPE");

        luaL_newmetatable(L, "TYPE_MT");
        lua_pushvalue(L, -1);
        lua_setmetatable(L, -3);

        lua_pushstring(L, "__index");
        lua_newtable(L);
        lua_pushnumber(L, -1);
        lua_setfield(L, -2, "UNDEFINED");
        lua_pushnumber(L, 1);
        lua_setfield(L, -2, "BYTE");
        lua_pushnumber(L, 2);
        lua_setfield(L, -2, "SHORT");
        lua_pushnumber(L, 3);
        lua_setfield(L, -2, "INT");
        lua_pushnumber(L, 4);
        lua_setfield(L, -2, "LONG");
        lua_pushnumber(L, 5);
        lua_setfield(L, -2, "FLOAT");
        lua_pushnumber(L, 6);
        lua_setfield(L, -2, "DOUBLE");
        lua_pushnumber(L, 7);
        lua_setfield(L, -2, "BYTE_ARRAY");
        lua_pushnumber(L, 8);
        lua_setfield(L, -2, "STRING");
        lua_pushnumber(L, 9);
        lua_setfield(L, -2, "LIST");
        lua_pushnumber(L, 10);
        lua_setfield(L, -2, "COMPOUND");
        lua_pushnumber(L, 11);
        lua_setfield(L, -2, "INT_ARRAY");
        lua_pushnumber(L, 12);
        lua_setfield(L, -2, "LONG_ARRAY");
        lua_settable(L, -3);

        lua_pushstring(L, "__newindex");
        lua_pushcfunction(L, LuaGlobals::Lua_ReadOnly);
        lua_settable(L, -3);

        lua_pop(L, 2);
    }

    void bindObject(const QMetaObject* metaObject, bool readOnly = false)
    {
        QString className = metaObject->className();

        lua_newtable(L);
        lua_pushvalue(L, -1);//makes copy of table cus when we set name, it pops off
        lua_setglobal(L, className.toStdString().c_str());//set table name

        //lua_pushvalue(L, -1);
        lua_pushlightuserdata(L, (void*)metaObject);//passing type information, might want to pass string cus pointer might change?
        lua_pushcclosure(L, CreateObject, 1);
        lua_setfield(L, -2, "new");

        luaL_newmetatable(L, QString(className + "_MT").toStdString().c_str());//creating metatable
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, DestroyObject);
        lua_settable(L, -3);

        lua_pushstring(L, "__index");
        lua_pushlightuserdata(L, (void*)metaObject);//passing type information, might want to pass string cus pointer might change?
        lua_pushcclosure(L, IndexObject, 1);
        lua_settable(L, -3);

        if(!readOnly)
        {
            lua_pushstring(L, "__newindex");
            lua_pushlightuserdata(L, (void*)metaObject);//passing type information, might want to pass string cus pointer might change?
            lua_pushcclosure(L, NewIndexObject, 1);
            lua_settable(L, -3);
        }

        lua_pop(L, 2);
    }

    void CallLuaFunction(QString functionName, QGenericReturnArgument &ret, QGenericArgument arg0 = QGenericArgument(), QGenericArgument arg1 = QGenericArgument(), QGenericArgument arg2 = QGenericArgument(), QGenericArgument arg3 = QGenericArgument(), QGenericArgument arg4 = QGenericArgument(), QGenericArgument arg5 = QGenericArgument(), QGenericArgument arg6 = QGenericArgument(), QGenericArgument arg7 = QGenericArgument(), QGenericArgument arg8 = QGenericArgument(), QGenericArgument arg9 = QGenericArgument())
    {
        if(running)
        {
            lua_getglobal(L, functionName.toStdString().c_str());
            if(lua_type(L, -1) == LUA_TFUNCTION)
            {
                const char *typeNames[] = {arg0.name(), arg1.name(), arg2.name(), arg3.name(), arg4.name(),
                                               arg5.name(), arg6.name(), arg7.name(), arg8.name(), arg9.name()};

                void *datas[] = {arg0.data(), arg1.data(), arg2.data(), arg3.data(), arg4.data(),
                                               arg5.data(), arg6.data(), arg7.data(), arg8.data(), arg9.data()};


                int numArgs;
                for (numArgs=0; numArgs<Q_METAMETHOD_INVOKE_MAX_ARGS; ++numArgs)
                {
                    if (qstrlen(typeNames[numArgs]) <= 0)
                        break;
                }

                for (int i=0; i<numArgs; i++)
                {
                    //if datas[i] is nullptr, do we generate a generic empty one?
                    //probably
                    if(!ToLuaProperty(L, QVariant(QMetaType::type(typeNames[i]), datas[i])))
                    {
                        return;
                    }
                }

                int numRet(0);
                if(qstrlen(ret.name()) != 0)
                {
                    numRet++;
                }

                if(lua_pcall(L, numArgs, numRet, 0) != 0)
                {
                    qDebug() << "ERROR-" + QString(lua_tostring(L, -1));
                    error = true;
                    return;
                }

                if(numRet > 0)
                {
                    int returnType = QMetaType::type(ret.name());

                    if(returnType == QMetaType::type("qint8"))
                    {
                        qint8 *retVal = (qint8*)ret.data();
                        *retVal = (qint8)lua_tonumber(L, -1);
                    }
                    else if(returnType == QMetaType::type("quint8"))
                    {
                        quint8 *retVal = (quint8*)ret.data();
                        *retVal = (quint8)lua_tonumber(L, -1);
                    }
                    else if(returnType == QMetaType::type("qint16"))
                    {
                        qint16 *retVal = (qint16*)ret.data();
                        *retVal = (qint16)lua_tonumber(L, -1);
                    }
                    else if(returnType == QMetaType::type("quint16"))
                    {
                        quint16 *retVal = (quint16*)ret.data();
                        *retVal = (quint16)lua_tonumber(L, -1);
                    }
                    else if(returnType == QMetaType::type("qint32"))
                    {
                        qint32 *retVal = (qint32*)ret.data();
                        *retVal = (qint32)lua_tonumber(L, -1);
                    }
                    else if(returnType == QMetaType::type("quint32"))
                    {
                        quint32 *retVal = (quint32*)ret.data();
                        *retVal = (quint32)lua_tonumber(L, -1);
                    }
                    else if(returnType == QMetaType::type("qint64"))
                    {
                        qint64 *retVal = (qint64*)ret.data();
                        *retVal = (qint64)lua_tonumber(L, -1);
                    }
                    else if(returnType == QMetaType::type("quint64"))
                    {
                        quint64 *retVal = (quint64*)ret.data();
                        *retVal = (quint64)lua_tonumber(L, -1);
                    }
                    else if(returnType == QMetaType::Float)
                    {
                        float *retVal = (float*)ret.data();
                        *retVal = (float)lua_tonumber(L, -1);
                    }
                    else if(returnType == QMetaType::Double)
                    {
                        double *retVal = (double*)ret.data();
                        *retVal = (double)lua_tonumber(L, -1);
                    }
                    else if(returnType == QMetaType::QString)
                    {
                        QString *retVal = (QString*)ret.data();
                        *retVal = (QString)lua_tostring(L, -1);
                    }
                    else if(returnType == QMetaType::Bool)
                    {
                        bool *retVal = (bool*)ret.data();
                        *retVal = (bool)lua_toboolean(L, -1);
                    }
                    else if(returnType == QMetaType::type("Tag*"))
                    {
                        Tag** retVal = (Tag**)ret.data();
                        *retVal = qobject_cast<Tag*>(((QSharedPointer<SharedObject>*)lua_touserdata(L, -1))->data());
                        lua_pop(L, 1);
                    }
                }
            }
            else
            {
                qDebug() << "ERROR-Could not find Lua function '" + functionName + "'";
                error = true;
                return;
            }
        }
    }

    void CallLuaFunction(QString functionName, QGenericArgument arg0, QGenericArgument arg1 = QGenericArgument(), QGenericArgument arg2 = QGenericArgument(), QGenericArgument arg3 = QGenericArgument(), QGenericArgument arg4 = QGenericArgument(), QGenericArgument arg5 = QGenericArgument(), QGenericArgument arg6 = QGenericArgument(), QGenericArgument arg7 = QGenericArgument(), QGenericArgument arg8 = QGenericArgument(), QGenericArgument arg9 = QGenericArgument())
    {
        QGenericReturnArgument ret = QGenericReturnArgument();
        CallLuaFunction(functionName, ret, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    }

    void CallLuaFunction(QString functionName)
    {
        QGenericArgument arg0 = QGenericArgument();
        CallLuaFunction(functionName, arg0);
    }

    void collectGarbage()
    {
        lua_gc(L, LUA_GCCOLLECT, 0);
    }

    lua_State *L;
    bool error;

private:

    static int CreateObject(lua_State *L)
    {
        if(!lua_isuserdata(L, lua_upvalueindex(1)))
        {
            luaL_error(L, "Object could not be created because no userdata was found on the upvalue stack 1");
            return 0;
        }

        const QMetaObject *metaObject = (const QMetaObject*)lua_touserdata(L, lua_upvalueindex(1));

        //handle constructor arguments
        int numLuaArgs = lua_gettop(L);
        PassByValue *pbv = new PassByValue[numLuaArgs]();
        QGenericArgument nativeArgs[10];

        if(numLuaArgs > 0)
        {
            bool validConstructorFound(false);
            for(int i=0; i<metaObject->constructorCount(); i++)
            {
                QMetaMethod constructorMethod = metaObject->constructor(i);

                if(constructorMethod.parameterCount() == numLuaArgs)
                {
                    ToNativeMethod(L, constructorMethod, numLuaArgs, 1, pbv, nativeArgs);
                    validConstructorFound = true;
                    break;
                }
            }

            if(!validConstructorFound)
            {
                luaL_error(L, QString("No valid constructor was found for object '" + QString(metaObject->className()) + "'").toStdString().c_str());
                delete[] pbv;
                return 0;
            }
        }

        QSharedPointer<SharedObject> *ud = (QSharedPointer<SharedObject>*)lua_newuserdata(L, sizeof(QSharedPointer<SharedObject>));
        SharedObject *object = qobject_cast<SharedObject*>(metaObject->newInstance(nativeArgs[0], nativeArgs[1], nativeArgs[2], nativeArgs[3], nativeArgs[4], nativeArgs[5], nativeArgs[6], nativeArgs[7], nativeArgs[8], nativeArgs[9]));

        if(object->sharedFromThis().isNull())
            new(ud) QSharedPointer<SharedObject>(object);
        else
            new(ud) QSharedPointer<SharedObject>(object->sharedFromThis());

        //uintptr_t *ud = (uintptr_t*)lua_newuserdata(L, sizeof(QObject*));
        //QObject *object = metaObject->newInstance(nativeArgs[0], nativeArgs[1], nativeArgs[2], nativeArgs[3], nativeArgs[4], nativeArgs[5], nativeArgs[6], nativeArgs[7], nativeArgs[8], nativeArgs[9]);
        //*ud = reinterpret_cast<uintptr_t>(object);

        luaL_setmetatable(L, QString(QString(metaObject->className()) + "_MT").toStdString().c_str());

        lua_newtable(L);
        lua_setuservalue(L, -2);//custom stuff

        //luaL_getmetatable(L, QString(QString(metaObject->className()) + "_MT").toStdString().c_str());//assigning metatable to object
        //lua_setmetatable(L, 1);

        delete[] pbv;
        return 1;
    }

    static int DestroyObject(lua_State* L)
    {
        if(!lua_isuserdata(L, -1))
        {
            luaL_error(L, "Object could not be destroyed because no userdata was found on the stack");
            return 0;
        }

        QSharedPointer<SharedObject> *ud = (QSharedPointer<SharedObject>*)lua_touserdata(L, -1);
        lua_pop(L, 1);

        ud->clear();

        return 0;
    }

    static int IndexObject(lua_State* L)
    {
        if(!lua_isuserdata(L, lua_upvalueindex(1)))
        {
            luaL_error(L, "Object could not be indexed because no userdata was found on the upvalue stack");
            return 0;
        }

        const QMetaObject *metaObject = (const QMetaObject*)lua_touserdata(L, lua_upvalueindex(1));

        if(!lua_isuserdata(L, 1))
        {
            luaL_error(L, "Attempting to index an object but no object was found on the stack");
            return 0;
        }

        if(!lua_isstring(L, 2))
        {
            luaL_error(L, QString("Attempting to index '" + QString(metaObject->className()) + "' without a native property or method name").toStdString().c_str());
            return 0;
        }

        QString fieldName = lua_tostring(L, 2);

        //check if method exists
        for(int i=metaObject->methodOffset(); i<metaObject->methodCount(); i++)
        {
            if(metaObject->method(i).name() == fieldName)
            {
                int stackSizeBefore = lua_gettop(L);

                lua_pushstring(L, fieldName.toStdString().c_str());
                lua_pushcclosure(L, InvokeMethodOnObject, 1);

                return lua_gettop(L) - stackSizeBefore;
            }
        }

        //check if property exists
        for(int i=metaObject->propertyOffset(); i<metaObject->propertyCount(); i++)
        {
            if(metaObject->property(i).name() == fieldName)
            {
                QSharedPointer<SharedObject> *ud = (QSharedPointer<SharedObject>*)lua_touserdata(L, 1);

                return ToLuaProperty(L, metaObject->property(i).read(ud->data()));
            }
        }

        //dont check for c_, jsut assume they're all uservalues
        /*
        //check if eligible for uservalue
        if(fieldName.startsWith("c_") && fieldName.length() > 2)
        {
            lua_getuservalue(L, 1);
            lua_pushvalue(L, 2);
            lua_gettable(L, -2);
            return 1;
        }

        luaL_error(L, QString("No methods or properties called '" + fieldName + "' for object '" + QString(metaObject->className()) + "'").toStdString().c_str());*/

        lua_getuservalue(L, 1);
        lua_pushvalue(L, 2);
        lua_gettable(L, -2);

        return 1;
    }

    static int NewIndexObject(lua_State* L)
    {
        if(!lua_isuserdata(L, lua_upvalueindex(1)))
        {
            luaL_error(L, "Object could not be indexed because no userdata was found on the upvalue stack");
            return 0;
        }

        const QMetaObject *metaObject = (const QMetaObject*)lua_touserdata(L, lua_upvalueindex(1));

        if(!lua_isuserdata(L, 1))
        {
            luaL_error(L, "Attempting to new index an object but no object was found on the stack");
            return 0;
        }

        if(!lua_isstring(L, 2))
        {
            luaL_error(L, QString("Attempting to new index '" + QString(metaObject->className()) + "' without a native property name").toStdString().c_str());
            return 0;
        }

        QString fieldName = lua_tostring(L, 2);

        //check if property exists
        for(int i=metaObject->propertyOffset(); i<metaObject->propertyCount(); i++)
        {
            if(metaObject->property(i).name() == fieldName)
            {
                if(metaObject->property(i).isWritable())
                    return ToNativeProperty(L, metaObject->property(i));
                else
                {
                    luaL_error(L, QString("Property '" + fieldName + "' for object '" + QString(metaObject->className()) + "' is not writable").toStdString().c_str());
                    return 0;
                }
            }
        }

        //dont check for c_, jsut assume they're all uservalues
        /*
        //check if eligible for uservalue
        if(fieldName.startsWith("c_") && fieldName.length() > 2)
        {
            lua_getuservalue(L, 1);
            lua_pushvalue(L, 2);
            lua_pushvalue(L, 3);
            lua_settable(L, -3);
            return 0;
        }*/
        //luaL_error(L, QString("No properties called '" + fieldName + "' for object '" + QString(metaObject->className()) + "'").toStdString().c_str());

        //uservalue
        lua_getuservalue(L, 1);
        lua_pushvalue(L, 2);
        lua_pushvalue(L, 3);
        lua_settable(L, -3);

        return 0;
    }

    static int InvokeMethodOnObject(lua_State* L)
    {
        if(!lua_isstring(L, lua_upvalueindex(1)))
        {
            luaL_error(L, "Method could not be invoked on object because no string was found on the upvalue stack");
            return 0;
        }

        QString methodName = lua_tostring(L, lua_upvalueindex(1));

        if(!lua_isuserdata(L, 1))
        {
            luaL_error(L, QString("Method '" + methodName + "' could not be invoked on object because no object was found on the stack").toStdString().c_str());
            return 0;
        }

        QSharedPointer<SharedObject> *ud = (QSharedPointer<SharedObject>*)lua_touserdata(L, 1);

        QObject *object = ud->data();

        int numLuaArgs = lua_gettop(L) - 1;
        if(numLuaArgs > Q_METAMETHOD_INVOKE_MAX_ARGS)
        {
            luaL_error(L, QString("Method '" + methodName + "' could not be invoked because more than '" + QString::number(Q_METAMETHOD_INVOKE_MAX_ARGS) + "' arguments were set in the script").toStdString().c_str());
            return 0;
        }

        PassByValue *pbv = new PassByValue[numLuaArgs]();
        //get the method with the right number of lua args
        const QMetaObject *metaObject = object->metaObject();
        for(int i=metaObject->methodOffset(); i<metaObject->methodCount(); i++)
        {
            QMetaMethod method = metaObject->method(i);
            if(method.name() == methodName && method.parameterCount() == numLuaArgs)
            {
                QGenericArgument nativeArgs[10];
                ToNativeMethod(L, method, numLuaArgs, 2, pbv, nativeArgs);

                int numReturnValues(0);
                if(!ToLuaMethod(L, method, object, numReturnValues, nativeArgs))
                {
                    luaL_error(L, QString("Failed to invoke method '" + methodName + "' on object '" + QString(metaObject->className()) + "'").toStdString().c_str());
                }

                delete[] pbv;
                return numReturnValues;
            }
        }

        luaL_error(L, QString("Method '" + methodName + "' could not be invoked because '" + methodName + "' with '" + QString::number(numLuaArgs) + "' arguments could not be found").toStdString().c_str());

        delete[] pbv;
        return 0;
    }

    static void ToNativeMethod(lua_State* L, QMetaMethod method, int numLuaArgs, int luaOffset, PassByValue *pbv, QGenericArgument (&nativeArgs)[10])
    {
        for(int i=0; i<numLuaArgs; i++)
        {
            int luaType = lua_type(L, i+luaOffset);

            switch(luaType)
            {
            case LUA_TNUMBER:
                if(method.parameterType(i) == QMetaType::type("qint8"))
                {
                    pbv[i].qint8Val =  (qint8)lua_tointeger(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(qint8, pbv[i].qint8Val);
                }
                else if(method.parameterType(i) == QMetaType::type("quint8"))
                {
                    pbv[i].quint8Val =  (quint8)lua_tointeger(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(quint8, pbv[i].quint8Val);
                }
                else if(method.parameterType(i) == QMetaType::type("qint16"))
                {
                    pbv[i].qint16Val =  (qint16)lua_tointeger(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(qint16, pbv[i].qint16Val);
                }
                else if(method.parameterType(i) == QMetaType::type("quint16"))
                {
                    pbv[i].quint16Val =  (quint16)lua_tointeger(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(quint16, pbv[i].quint16Val);
                }
                else if(method.parameterType(i) == QMetaType::type("qint32"))
                {
                    pbv[i].qint32Val =  (qint32)lua_tointeger(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(qint32, pbv[i].qint32Val);
                }
                else if(method.parameterType(i) == QMetaType::type("quint32"))
                {
                    pbv[i].quint32Val =  (quint32)lua_tointeger(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(quint32, pbv[i].quint32Val);
                }
                else if(method.parameterType(i) == QMetaType::type("qint64"))
                {
                    pbv[i].qint64Val =  (qint64)lua_tointeger(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(qint64, pbv[i].qint64Val);
                }
                else if(method.parameterType(i) == QMetaType::type("quint64"))
                {
                    pbv[i].quint64Val =  (quint64)lua_tointeger(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(quint64, pbv[i].quint64Val);
                }
                else if(method.parameterType(i) == QMetaType::Float)
                {
                    pbv[i].floatVal =  (float)lua_tonumber(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(float, pbv[i].floatVal);
                }
                else if(method.parameterType(i) == QMetaType::Double)
                {
                    pbv[i].doubleVal =  (double)lua_tonumber(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(double, pbv[i].doubleVal);
                }
                else if(method.parameterType(i) == QMetaType::type("TagType"))
                {
                    pbv[i].qint32Val = (qint32)lua_tonumber(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(qint32, pbv[i].qint32Val);
                }
                else
                {
                    luaL_error(L, QString("Method '" + method.name() + "' failed to invoke due to an unrecognized number native argument type '" + method.parameterNames().at(i) + "' " + QString::number(method.parameterType(i))).toStdString().c_str());
                }
                break;
            case LUA_TSTRING:
                if(method.parameterType(i) == QMetaType::QString)
                {
                    pbv[i].stringVal =  (QString)lua_tostring(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(QString, pbv[i].stringVal);
                }
                else
                {
                    luaL_error(L, QString("Method '" + method.name() + "' failed to invoke due to an unrecognized string native argument type '" + method.parameterNames().at(i) + "'").toStdString().c_str());
                }
                break;
            case LUA_TBOOLEAN:
                if(method.parameterType(i) == QMetaType::type("qint8"))
                {
                    pbv[i].qint8Val =  (qint8)lua_toboolean(L, i+luaOffset);
                    nativeArgs[i] = Q_ARG(qint8, pbv[i].qint8Val);
                }
                else
                {
                    luaL_error(L, QString("Method '" + method.name() + "' failed to invoke due to an unrecognized boolean native argument type '" + method.parameterNames().at(i) + "'").toStdString().c_str());
                }
                break;
            case LUA_TUSERDATA:
                if(method.parameterType(i) == QMetaType::type("Tag*"))
                {
                    QSharedPointer<SharedObject> *ud = (QSharedPointer<SharedObject>*)lua_touserdata(L, i+luaOffset);
                    pbv[i].tagVal = qobject_cast<Tag*>(ud->data());
                    nativeArgs[i] = Q_ARG(Tag*, pbv[i].tagVal);
                }
                break;
            default:
                QString errorMsg = QString("Method '" + method.name() + "' failed to invoke due to an unrecognized lua argument type '" + QString::number(luaType) + "'");
                luaL_error(L, errorMsg.toStdString().c_str());
                break;
            }
        }
    }

    static bool ToLuaMethod(lua_State* L, QMetaMethod method, QObject *object, int &numReturnValues, QGenericArgument (&nativeArgs)[10])
    {
        ReturnByValue rbv;

        int returnType = method.returnType();

        bool invokeSuccessful(false);
        if(returnType == QMetaType::Void)
        {
            invokeSuccessful = method.invoke(object, Qt::DirectConnection, nativeArgs[0], nativeArgs[1], nativeArgs[2], nativeArgs[3], nativeArgs[4], nativeArgs[5], nativeArgs[6], nativeArgs[7], nativeArgs[8], nativeArgs[9]);
        }
        else
        {
            QGenericReturnArgument returnArg;

            if(returnType == QMetaType::type("qint8"))
            {
                returnArg = Q_RETURN_ARG(qint8, rbv.qint8Val);
            }
            else if(returnType == QMetaType::type("quint8"))
            {
                returnArg = Q_RETURN_ARG(quint8, rbv.quint8Val);
            }
            else if(returnType == QMetaType::type("qint16"))
            {
                returnArg = Q_RETURN_ARG(qint16, rbv.qint16Val);
            }
            else if(returnType == QMetaType::type("quint16"))
            {
                returnArg = Q_RETURN_ARG(quint16, rbv.quint16Val);
            }
            else if(returnType == QMetaType::type("qint32"))
            {
                returnArg = Q_RETURN_ARG(qint32, rbv.qint32Val);
            }
            else if(returnType == QMetaType::type("quint32"))
            {
                returnArg = Q_RETURN_ARG(quint32, rbv.quint32Val);
            }
            else if(returnType == QMetaType::type("qint64"))
            {
                returnArg = Q_RETURN_ARG(qint64, rbv.qint64Val);
            }
            else if(returnType == QMetaType::type("quint64"))
            {
                returnArg = Q_RETURN_ARG(quint64, rbv.quint64Val);
            }
            else if(returnType == QMetaType::Float)
            {
                returnArg = Q_RETURN_ARG(float, rbv.floatVal);
            }
            else if(returnType == QMetaType::Double)
            {
                returnArg = Q_RETURN_ARG(double, rbv.doubleVal);
            }
            else if(returnType == QMetaType::QString)
            {
                returnArg = Q_RETURN_ARG(QString, rbv.stringVal);
            }
            else if(returnType == QMetaType::Bool)
            {
                returnArg = Q_RETURN_ARG(bool, rbv.boolVal);
            }
            else if(returnType == QMetaType::type("Tag*"))
            {
                returnArg = Q_RETURN_ARG(Tag*, rbv.tagVal);
            }
            else
            {
                luaL_error(L, QString("Method '" + method.name() + "' failed to invoke due to an unrecognized native return type").toStdString().c_str());
                return false;
            }

            if(method.invoke(object, Qt::DirectConnection, returnArg, nativeArgs[0], nativeArgs[1], nativeArgs[2], nativeArgs[3], nativeArgs[4], nativeArgs[5], nativeArgs[6], nativeArgs[7], nativeArgs[8], nativeArgs[9]))
            {
                if(returnType == QMetaType::type("qint8"))
                {
                    lua_pushinteger(L, rbv.qint8Val);
                    numReturnValues++;
                }
                else if(returnType == QMetaType::type("quint8"))
                {
                    lua_pushinteger(L, rbv.quint8Val);
                    numReturnValues++;
                }
                else if(returnType == QMetaType::type("qint16"))
                {
                    lua_pushinteger(L, rbv.qint16Val);
                    numReturnValues++;
                }
                else if(returnType == QMetaType::type("quint16"))
                {
                    lua_pushinteger(L, rbv.quint16Val);
                    numReturnValues++;
                }
                else if(returnType == QMetaType::type("qint32"))
                {
                    lua_pushinteger(L, rbv.qint32Val);
                    numReturnValues++;
                }
                else if(returnType == QMetaType::type("quint32"))
                {
                    lua_pushinteger(L, rbv.quint32Val);
                    numReturnValues++;
                }
                else if(returnType == QMetaType::type("qint64"))
                {
                    lua_pushinteger(L, rbv.qint64Val);
                    numReturnValues++;
                }
                else if(returnType == QMetaType::type("quint64"))
                {
                    lua_pushinteger(L, rbv.quint64Val);
                    numReturnValues++;
                }
                else if(returnType == QMetaType::Float)
                {
                    lua_pushnumber(L, rbv.floatVal);
                    numReturnValues++;
                }
                else if(returnType == QMetaType::Double)
                {
                    lua_pushnumber(L, rbv.doubleVal);
                    numReturnValues++;
                }
                else if(returnType == QMetaType::QString)
                {
                    lua_pushstring(L, rbv.stringVal.toStdString().c_str());
                    numReturnValues++;
                }
                else if(returnType == QMetaType::Bool)
                {
                    lua_pushboolean(L, rbv.boolVal);
                    numReturnValues++;
                }
                else if(returnType == QMetaType::type("Tag*"))
                {
                    Tag* foundTag = rbv.tagVal;

                    if(foundTag == nullptr)
                    {
                        luaL_error(L, "Returned tag from method '" + method.name() + "' was null");
                        return false;
                    }

                    QSharedPointer<SharedObject> *ud = (QSharedPointer<SharedObject>*)lua_newuserdata(L, sizeof(QSharedPointer<SharedObject>));

                    if(foundTag->sharedFromThis().isNull())
                        new(ud) QSharedPointer<SharedObject>(foundTag);
                    else
                        new(ud) QSharedPointer<SharedObject>(foundTag->sharedFromThis());

                    luaL_setmetatable(L, QString(QString(foundTag->metaObject()->className()) + "_MT").toStdString().c_str());

                    //NOT SURE IF THIS SHOULD BE HERE
                    //IT WAS ADDED TO SUPPORT CUSTOM TAG PROPERTIES
                    lua_newtable(L);
                    lua_setuservalue(L, -2);//custom stuff

                    numReturnValues++;
                }
                invokeSuccessful = true;
            }
        }

        return invokeSuccessful;
    }

    static int ToNativeProperty(lua_State* L, QMetaProperty property)
    {
        int luaType = lua_type(L, 3);
        QSharedPointer<SharedObject> *ud = (QSharedPointer<SharedObject>*)lua_touserdata(L, 1);

        switch(luaType)
        {
        case LUA_TNUMBER:
            if(property.type() == QMetaType::type("qint8"))
            {
                property.write(ud->data(), (qint8)lua_tointeger(L, 3));
            }
            else if(property.type() == QMetaType::type("quint8"))
            {
                property.write(ud->data(), (quint8)lua_tointeger(L, 3));
            }
            else if(property.type() == QMetaType::type("qint16"))
            {
                property.write(ud->data(), (qint16)lua_tointeger(L, 3));
            }
            else if(property.type() == QMetaType::type("quint16"))
            {
                property.write(ud->data(), (quint16)lua_tointeger(L, 3));
            }
            else if(property.type() == QMetaType::type("qint32"))
            {
                property.write(ud->data(), (qint32)lua_tointeger(L, 3));
            }
            else if(property.type() == QMetaType::type("quint32"))
            {
                property.write(ud->data(), (quint32)lua_tointeger(L, 3));
            }
            else if(property.type() == QMetaType::type("qint64"))
            {
                property.write(ud->data(), (qint64)lua_tointeger(L, 3));
            }
            else if(property.type() == QMetaType::type("quint64"))
            {
                property.write(ud->data(), (quint64)lua_tointeger(L, 3));
            }
            else if(property.type() == QMetaType::Float)
            {
                property.write(ud->data(), (float)lua_tonumber(L, 3));
            }
            else if(property.type() == QMetaType::Double)
            {
                property.write(ud->data(), (double)lua_tonumber(L, 3));
            }
            else if(property.type() == QMetaType::type("TagType"))
            {
                property.write(ud->data(), (qint32)lua_tonumber(L, 3));
            }
            else
            {
                luaL_error(L, QString("Property '" + QString(property.name()) + "' failed to be written due to an unrecognized number native type").toStdString().c_str());
            }
            break;
        case LUA_TSTRING:
            if(property.type() == QMetaType::QString)
            {
                property.write(ud->data(), (QString)lua_tostring(L, 3));
            }
            else
            {
                luaL_error(L, QString("Property '" + QString(property.name()) + "' failed to be written due to an unrecognized string native type").toStdString().c_str());
            }
            break;
        case LUA_TBOOLEAN:
            if(property.type() == QMetaType::type("qint8"))
            {
                property.write(ud->data(), (qint8)lua_toboolean(L, 3));
            }
            else
            {
                luaL_error(L, QString("Property '" + QString(property.name()) + "' failed to be written due to an unrecognized boolean native type").toStdString().c_str());
            }
            break;
        default:
            luaL_error(L, QString("Property '" + QString(property.name()) + "' failed to be written due to an unrecognized lua type " + QString::number(luaType)).toStdString().c_str());
            break;
        }

        return 0;
    }

    static int ToLuaProperty(lua_State* L, QVariant propertyValue)
    {
        int propertyType = propertyValue.userType();

        if(propertyType == QMetaType::type("qint8"))
        {
            lua_pushinteger(L, propertyValue.value<qint8>());
        }
        else if(propertyType == QMetaType::type("quint8"))
        {
            lua_pushinteger(L, propertyValue.value<quint8>());
        }
        else if(propertyType == QMetaType::type("qint16"))
        {
            lua_pushinteger(L, propertyValue.value<qint16>());
        }
        else if(propertyType == QMetaType::type("quint16"))
        {
            lua_pushinteger(L, propertyValue.value<quint16>());
        }
        else if(propertyType == QMetaType::type("qint32"))
        {
            lua_pushinteger(L, propertyValue.value<qint32>());
        }
        else if(propertyType == QMetaType::type("quint32"))
        {
            lua_pushinteger(L, propertyValue.value<quint32>());
        }
        else if(propertyType == QMetaType::type("qint64"))
        {
            lua_pushinteger(L, propertyValue.value<qint64>());
        }
        else if(propertyType == QMetaType::type("quint64"))
        {
            lua_pushinteger(L, propertyValue.value<quint64>());
        }
        else if(propertyType == QMetaType::Float)
        {
            lua_pushnumber(L, propertyValue.toFloat());
        }
        else if(propertyType == QMetaType::Double)
        {
            lua_pushnumber(L, propertyValue.toDouble());
        }
        else if(propertyType == QMetaType::QString)
        {
            lua_pushstring(L, propertyValue.toString().toStdString().c_str());
        }
        else if(propertyType == QMetaType::type("Tag"))
        {
            Tag* foundTag = propertyValue.value<Tag*>();

            if(foundTag == nullptr)
            {
                luaL_error(L, "Property of type 'Tag' was null");
                return 0;
            }

            QSharedPointer<SharedObject> *ud = (QSharedPointer<SharedObject>*)lua_newuserdata(L, sizeof(QSharedPointer<SharedObject>));

            if(foundTag->sharedFromThis().isNull())
                new(ud) QSharedPointer<SharedObject>(foundTag);
            else
                new(ud) QSharedPointer<SharedObject>(foundTag->sharedFromThis());

            luaL_setmetatable(L, QString(QString(foundTag->metaObject()->className()) + "_MT").toStdString().c_str());

            lua_newtable(L);
            lua_setuservalue(L, -2);//custom stuff
        }
        else if(propertyType == QMetaType::type("TagType"))
        {
            lua_pushnumber(L, propertyValue.toInt());
        }
        else
        {
            luaL_error(L, QString("Unrecognized property type '" + QString(propertyValue.typeName()) + "'").toStdString().c_str());
            return 0;
        }

        return 1;
    }


    QString scriptPath;
    bool running;
};

#endif // LUA_SCRIPT_H
