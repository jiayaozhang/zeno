#include "qdmgraphicsnode.h"
#include "qdmgraphicssocket.h"
#include "qdmgraphicsscene.h"
#include <zeno/dop/Descriptor.h>

QDMGraphicsNode::QDMGraphicsNode()
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);

    label = std::make_unique<QGraphicsTextItem>(this);
    label->setDefaultTextColor(QColor(0xcccccc));
    label->setPos(0, -SOCKSTRIDE);
}

QDMGraphicsNode::~QDMGraphicsNode() = default;

float QDMGraphicsNode::getHeight() const
{
    size_t count = std::max(socketIns.size(), socketOuts.size());
    return SOCKMARGINTOP + SOCKSTRIDE * count + SOCKMARGINBOT;
}

QRectF QDMGraphicsNode::boundingRect() const
{
    return QRectF(-QDMGraphicsSocket::SIZE, 1e-6f, WIDTH + QDMGraphicsSocket::SIZE * 2, getHeight() - 2e-6f);
}

void QDMGraphicsNode::paint(QPainter *painter, QStyleOptionGraphicsItem const *styleOptions, QWidget *widget)
{
    if (isSelected()) {
        QPen pen;
        pen.setColor(QColor(0xff8800));
        pen.setWidthF(BORDER);
        painter->setPen(pen);
    } else {
        painter->setPen(Qt::NoPen);
    }
    painter->setBrush(QColor(0x555555));

    QPainterPath path;
    QRectF rect(0, 0, WIDTH, getHeight());
    path.addRoundedRect(rect, ROUND, ROUND);
    painter->drawPath(path.simplified());
}

QDMGraphicsSocketIn *QDMGraphicsNode::addSocketIn()
{
    auto socketIn = std::make_unique<QDMGraphicsSocketIn>();
    auto socketInP = socketIn.get();
    socketIn->setParentItem(this);

    size_t index = socketIns.size();
    socketIn->setPos(-socketIn->SIZE / 2, SOCKMARGINTOP + SOCKSTRIDE * index);

    socketIns.push_back(move(socketIn));
    return socketInP;
}

QDMGraphicsSocketOut *QDMGraphicsNode::addSocketOut()
{
    auto socketOut = std::make_unique<QDMGraphicsSocketOut>();
    auto socketOutP = socketOut.get();
    socketOut->setParentItem(this);

    size_t index = socketOuts.size();
    socketOut->setPos(WIDTH + socketOut->SIZE / 2, SOCKMARGINTOP + SOCKSTRIDE * index);

    socketOuts.push_back(move(socketOut));
    return socketOuts.back().get();
    return socketOutP;
}

void QDMGraphicsNode::initByName(QString name)
{
    setName(name);
    auto const &desc = zeno::dop::desc_of(name.toStdString());
    dopNode = desc.factory();
    for (auto const &sockinfo: desc.inputs) {
        auto socket = addSocketIn();
        socket->setName(QString::fromStdString(sockinfo.name));
    }
    for (auto const &sockinfo: desc.outputs) {
        auto socket = addSocketOut();
        socket->setName(QString::fromStdString(sockinfo.name));
    }
}

void QDMGraphicsNode::setName(QString name)
{
    label->setPlainText(name);
}

void QDMGraphicsNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        auto parentScene = static_cast<QDMGraphicsScene *>(scene());
        parentScene->removeNode(this);
        return;
    }

    QGraphicsItem::mousePressEvent(event);
}

void QDMGraphicsNode::unlinkAll()
{
    for (auto const &p: socketIns) {
        p->unlinkAll();
    }
    for (auto const &p: socketOuts) {
        p->unlinkAll();
    }
}