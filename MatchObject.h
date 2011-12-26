#ifndef MATCH_OBJ_H_
#define MATCH_OBJ_H_

typedef struct _Group {
    int gbeg, gend;
} Group;

typedef struct _MatchObject {
    const char *str;
    Group *groups;
    int n;
} MatchObject;

inline void addGroup(MatchObject *m, int b, int e)
{
    Group newG, *oldG;
    int i;

    if (!m) return;

    oldG=m->groups;
    newG.gbeg=b;
    newG.gend=e;

    while (oldG != m->groups+m->n) {
        if (b < oldG->gbeg) {
            break;

        } else if (b == oldG->gbeg) {
            if (e > oldG->gend)
                break;

            else if (e == oldG->gend)
                return;

            else oldG++;

        } else oldG++;
    }

    i=oldG - m->groups;
    m->groups=(Group *)realloc(m->groups, ++m->n*sizeof(Group));
    if (!m->groups) {
        fprintf(stderr, "error: addGroup: out of memory\n");
        exit(1);
    }
    memmove(m->groups+i+1, m->groups+i, (m->n-i-1)*sizeof(Group));
    m->groups[i]=newG;
}

#endif /* MATCH_OBJ_H_ */
