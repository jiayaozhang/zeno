#include "qdmgraphicssocketin.h"
#include "qdmgraphicsnode.h"
#include "qdmgraphicslinkfull.h"

ZENO_NAMESPACE_BEGIN

QDMGraphicsSocketIn::QDMGraphicsSocketIn()
{
    label->setPos(SIZE / 2, -SIZE * 0.7f);
}

void QDMGraphicsSocketIn::unlinkAll()
{
    QDMGraphicsSocket::unlinkAll();
    auto parentNode = static_cast<QDMGraphicsNode *>(parentItem());
    parentNode->invalidate();
}

void QDMGraphicsSocketIn::linkRemoved(QDMGraphicsLinkFull *link)
{
    QDMGraphicsSocket::linkRemoved(link);
    auto parentNode = static_cast<QDMGraphicsNode *>(parentItem());
    parentNode->invalidate();
}

void QDMGraphicsSocketIn::linkAttached(QDMGraphicsLinkFull *link)
{
    QDMGraphicsSocket::unlinkAll();
    QDMGraphicsSocket::linkAttached(link);
    if (link) {
        auto parentNode = static_cast<QDMGraphicsNode *>(parentItem());
        parentNode->invalidate();
    }
}

QPointF QDMGraphicsSocketIn::getLinkedPos() const
{
    return scenePos() - QPointF(SIZE / 2, 0);
}

ZENO_NAMESPACE_END