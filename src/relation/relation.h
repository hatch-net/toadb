/*
 *	toadb relation 
 * Copyright (C) 2023-2023, senllang
*/
#ifndef HAT_RELATION_H_H
#define HAT_RELATION_H_H

typedef struct Relation
{
    int databaseId;
    int relid;
    int relType;
}Relation, *PRelation;

#define RelationCompare(rel1, rel2)  (((rel1)->databaseId == (rel2)->databaseId) \
                                     && ((rel1)->relid == (rel2)->relid))


#endif
