#ifndef TAGS_H
#define TAGS_H

#include <QtCore>

#include <QSharedPointer>
class SharedObject : public QObject, public QEnableSharedFromThis<SharedObject>
{
    Q_OBJECT
public:
    SharedObject(QObject* parent = nullptr) : QObject(parent){}

    virtual ~SharedObject()
    {

    }

    QSharedPointer<SharedObject> getSharedPointer()
    {
        return sharedFromThis();
    }
};

//SAMPLE DATA TYPES TO BE USED THROUGH SCRIPT
//------------------------------
class Tag : public SharedObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(Type type READ getType)
    Q_PROPERTY(Tag* parentTag READ getParent)
public:
    enum Type
    {
        UNDEFINED = -1,
        END,
        BYTE,
        SHORT,
        INT,
        LONG,
        FLOAT,
        DOUBLE,
        BYTE_ARRAY,
        STRING,
        LIST,
        COMPOUND,
        INT_ARRAY,
        LONG_ARRAY
    };
    Q_ENUM(Type)

    explicit Tag(QString name = "", Type type = UNDEFINED) : SharedObject(nullptr), name(name), type(type), parentTag(nullptr){}

    Q_INVOKABLE virtual ~Tag(){
    }

    QString name;
    Type type;
    Tag *parentTag;
    QVector<QSharedPointer<SharedObject>> childTags;

public slots:
    bool hasParent()
    {
        return parentTag != nullptr;
    }

    qint32 getRow()
    {
        if(parentTag != nullptr)
        {
            for(int i=0; i<parentTag->childTags.count(); i++)
            {
                if(parentTag->childTags.at(i).data() == this)
                {
                    return i;
                }
            }
        }

        return -1;
    }

protected:
    QString getName(){return name;}
    void setName(QString name){this->name = name;}
    Type getType(){return type;}
    Tag* getParent(){return parentTag;}
};

class TagInt : public Tag
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(Type type READ getType)
    Q_PROPERTY(qint32 value READ getValue WRITE setValue)
public:
    Q_INVOKABLE explicit TagInt(QString name = "", qint32 value = 0) : Tag(name, INT), value(value){}

    Q_INVOKABLE ~TagInt(){}

    qint32 value;

public slots:
    bool hasParent(){return Tag::hasParent();}
    qint32 getRow(){return Tag::getRow();}

private:
    qint32 getValue(){return value;}
    void setValue(qint32 value){this->value = value;}
};

class TagContainer : public Tag
{
    Q_OBJECT
public:
    explicit TagContainer(QString name, Type type) : Tag(name, type), childCount(0){}

    Q_INVOKABLE virtual ~TagContainer()
    {
        childTags.clear();
    }

    template<typename T>
    T getChild(int i)
    {
        if(!childTags.at(i).isNull())
            return qobject_cast<T>(childTags.at(i).data());
        else
            return nullptr;
    }

    qint32 get_childCount(){return childCount;}

    qint32 childCount;

public slots:
    Tag* addChild(Tag* tag);

    void removeChild(qint32 index)
    {
        if(index < childTags.size() && index >= 0)
        {
            qobject_cast<Tag*>(childTags.at(index).data())->parentTag = nullptr;
            childTags.remove(index);
            childCount--;
        }
    }

    Tag* child(qint32 index)
    {
        if(index < childTags.size() && index >= 0)
        {
            if(!childTags.at(index).isNull())
                return qobject_cast<Tag*>(childTags.at(index).data());
            else
                return nullptr;
        }

        return nullptr;
    }

    void clear()
    {
        childTags.clear();
        childCount = 0;
    }
};

class TagList : public TagContainer
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(Type type READ getType)
    Q_PROPERTY(qint32 childCount READ get_childCount)
    Q_PROPERTY(Type listType READ getListType)
public:
    Q_INVOKABLE explicit TagList(QString name = "") : TagContainer(name, LIST), listType(UNDEFINED){}

    Q_INVOKABLE ~TagList(){}

    Type listType;

public slots:
    bool hasParent(){return Tag::hasParent();}
    qint32 getRow(){return Tag::getRow();}
    Tag* addChild(Tag* tag)
    {
        if(listType == tag->type || listType == UNDEFINED)
        {
            TagContainer::addChild(tag);

            if(listType == UNDEFINED)
                listType = tag->type;
        }

        return tag;
    }
    void removeChild(qint32 index)
    {
        TagContainer::removeChild(index);

        if(childCount == 0)
            listType = UNDEFINED;
    }
    Tag* child(qint32 index){return TagContainer::child(index);}
    void clear()
    {
        TagContainer::clear();
        listType = UNDEFINED;
    }

private:
    Type getListType(){return listType;}
};

class TagCompound : public TagContainer
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(Type type READ getType)
    Q_PROPERTY(qint32 childCount READ get_childCount)
    Q_PROPERTY(Tag* lastFound READ get_lastFound)
public:
    Q_INVOKABLE explicit TagCompound(QString name = "") : TagContainer(name, COMPOUND){}

    Q_INVOKABLE ~TagCompound()
    {
    }

    template<typename T>
    T getLastFound()
    {
        return qobject_cast<T>(lastFound);
    }

public slots:
    bool hasParent(){return Tag::hasParent();}
    qint32 getRow(){return Tag::getRow();}
    Tag* addChild(Tag* tag){return TagContainer::addChild(tag);}
    void removeChild(qint32 index){TagContainer::removeChild(index);}
    Tag* child(qint32 index){return TagContainer::child(index);}
    void clear(){TagContainer::clear();}

    bool contains(QString name, Tag::Type type = UNDEFINED, Tag::Type listType = UNDEFINED)
    {
        int childCount = childTags.count();
        for(int i=childCount-1; i>=0; i--)
        {
            Tag* tag = getChild<Tag*>(i);
            if(tag->name == name)
            {
                if(tag->type == type || type == Tag::UNDEFINED)
                {
                    if(tag->type == LIST)
                    {
                        TagList* tagList = qobject_cast<TagList*>(tag);
                        if(tagList->listType == listType || tagList->listType == UNDEFINED || listType == UNDEFINED)
                        {
                            lastFound = tag;
                            return true;
                        }
                    }
                    else
                    {
                        lastFound = tag;
                        return true;
                    }
                }
            }
        }

        return false;
    }

private:

    Tag* get_lastFound(){return lastFound;}
    Tag* lastFound;
};

Q_DECLARE_METATYPE(Tag*)

Q_DECLARE_METATYPE(Tag::Type)

//-----------------------------

#endif // TAGS_H
