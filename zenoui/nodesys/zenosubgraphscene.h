#ifndef __ZENO_SUBGRAPH_SCENE_H__
#define __ZENO_SUBGRAPH_SCENE_H__

#include <QtWidgets>
#include "../render/ztfutil.h"
#include "nodesys_common.h"
#include "../model/modeldata.h"


class SubGraphModel;
class ZenoNode;
class ZenoFullLink;
class ZenoTempLink;

class ZenoSubGraphScene : public QGraphicsScene
{
	Q_OBJECT
public:
    ZenoSubGraphScene(QObject* parent = nullptr);
    void initModel(SubGraphModel* pModel);
    QPointF getSocketPos(bool bInput, const QString &nodeid, const QString &portName);
    void undo();
    void redo();
    void copy();
    void paste(QPointF pos);
    QRectF _sceneRect() const;
    void _setSceneRect(const QRectF &rc);
    SubGraphModel* model() const;
    void select(const QString& id);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event);

public slots:
    void onNewNodeCreated();    //todo: category.
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void onRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onLinkChanged(bool bAdd, const QString &outputId, const QString &outputPort, const QString &inputId, const QString &inputPort);
    void onParamUpdated(const QString& nodeid, const QString& paramName, const QVariant& val);

private slots:
    void reload();
    void clearLayout();
    void onSocketPosInited(const QString& nodeid, const QString& sockName, bool bInput);

private:
    void updateLinkPos(ZenoNode *pNode, QPointF newPos);
    bool _enableLink(const QString &outputNode, const QString &outputSocket,
                     const QString &inputNode, const QString &inputSocket);

    QRectF m_scene_rect;
    NodeUtilParam m_nodeParams;
	SubGraphModel* m_subgraphModel;
    std::map<QString, ZenoNode*> m_nodes;
    std::map<EdgeInfo, ZenoFullLink*/*, cmpEdge*/> m_links;
    ZenoTempLink* m_tempLink;
};

#endif