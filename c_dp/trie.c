#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "trie.h"
#include "util.h"

#define MAXWORD 100

static inline size_t IX(const char c)
{
  return c-CHAR_MIN;
}

extern inline long get_value(Trie *);

Trie *mk_trie()
{
  Trie *p = nv_alloc(sizeof(Trie));
  memset(p,0,sizeof(Trie));
  return p;
}

void increment(Trie *tp,const char *s)
{
  for (;*s;s++){
    if (tp->child[IX(*s)]==NULL){
      Trie *nxt = mk_trie();
      nxt->depth = tp->depth+1;
      nxt->c = *s;
      nxt->prev = tp;
      tp->child[IX(*s)] = nxt;
    }
    tp = tp->child[IX(*s)];
  }
  tp->count++;
}

char *get_key(Trie *tp)
{
  char *s=nv_alloc(tp->depth+1);
  char *t=s+tp->depth;
  *t='\0';
  while (tp->prev!=NULL){
    t--;
    *t = tp->c;
    tp=tp->prev;
  }
  return s;
}

void visit_neighbours(Trie *tp,char *s,int maxedit,
                      callback_t f,void *state)
{
  /*No edit*/
  if (!*s && tp->count)
    f(state,tp);
  else if (tp->child[IX(*s)]!=NULL)
    visit_neighbours(tp->child[IX(*s)],s+1,maxedit,f,state);
  
  if (maxedit<1)
    return;

  /*Insert*/
  for (int i=0;i<JTRIE_NUM_CHILD;i++){
    if (tp->child[i]!=NULL)
      visit_neighbours(tp->child[i],s,maxedit-1,f,state);
  }

  /*Replace, delete and traspose require length>0*/
  if (!*s)
    return;

  /*Replace*/
  for (int i=0;i<JTRIE_NUM_CHILD;i++){
    if (i!=IX(*s) && tp->child[i]!=NULL)
      visit_neighbours(tp->child[i],s+1,maxedit-1,f,state);
  }

  /*Delete*/
  visit_neighbours(tp,s+1,maxedit-1,f,state);

  /*Transpose*/
  char c = s[1];
  if ((!c) || tp->child[IX(c)]==NULL)
    return;
  s[1]=s[0];
  visit_neighbours(tp->child[IX(c)],s+1,maxedit-1,f,state);
  s[1]=c;
  return;
}
  
long lookup(const Trie *tp,const char *s)
{
  for (;*s;s++){
    if (tp->child[IX(*s)]==NULL)
      return 0;
    tp = tp->child[IX(*s)];
  }
  return tp->count;
}

Trie *train(FILE *fp)
{
  int c='.';
  int i;
  char wrd[MAXWORD+1];
  Trie *tp=mk_trie();

  while (c!=EOF){
    while ((c=getc(fp))!=EOF && !isalpha(c))
      ;
    i=0;
    while (isalpha(c) && i<MAXWORD){
      wrd[i++] = tolower(c);
      c=getc(fp);
    }
    wrd[i]='\0';
    if(i==MAXWORD){
      fputs("Word length exceeded\n",stderr);
      exit(1);
    }
    if (i>0)
      increment(tp,wrd);
  }
  return tp;
}
