#ifndef __MODEL_ROLE_H__
#define __MODEL_ROLE_H__

enum MODEL_ROLE
{
	ROLE_OBJID = Qt::UserRole + 1,
	ROLE_OBJNAME,
	ROLE_OBJTYPE,
	ROLE_OBJPOS,
	ROLE_OBJRECT,
	ROLE_SRCNODE,
	ROLE_DSTNODE,
	ROLE_PARAMETERS,
	ROLE_INPUTS,
	ROLE_OUTPUTS,
	ROLE_COLORRAMPS,
	ROLE_GRAPHPTR,
	ROLE_BLACKBOARD_SIZE,
    ROLE_BLACKBOARD_CONTENT,
	ROLE_BLACKBOARD_SPECIAL,
    ROLE_BLACKBOARD_TITLE,
    ROLE_BLACKBOARD_PARAMS,
};

#endif