#include "tags.h"

Tag *TagContainer::addChild(Tag *tag)
{
    QSharedPointer<SharedObject> tagSharedPointer = tag->sharedFromThis();
    if(tagSharedPointer.isNull())
        tagSharedPointer = QSharedPointer<SharedObject>(tag);

    childTags.append(tagSharedPointer);

    if(tag->hasParent())
    {
        if(tag->parentTag->type == COMPOUND)
        {
            TagCompound* parentTag = qobject_cast<TagCompound*>(tag->parentTag);
            parentTag->removeChild(tag->getRow());
        }
        else if(tag->parentTag->type == LIST)
        {
            TagList* parentTag = qobject_cast<TagList*>(tag->parentTag);
            parentTag->removeChild(tag->getRow());
        }
    }

    tag->parentTag = this;

    childCount++;

    return tag;
}
