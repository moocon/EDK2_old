/*
 * build.c -- functions associated with building syntax diagrams.
 *
 * SOFTWARE RIGHTS
 *
 * We reserve no LEGAL rights to the Purdue Compiler Construction Tool
 * Set (PCCTS) -- PCCTS is in the public domain.  An individual or
 * company may do whatever they wish with source code distributed with
 * PCCTS or the code generated by PCCTS, including the incorporation of
 * PCCTS, or its output, into commerical software.
 *
 * We encourage users to develop software with PCCTS.  However, we do ask
 * that credit is given to us for developing PCCTS.  By "credit",
 * we mean that if you incorporate our source code into one of your
 * programs (commercial product, research project, or otherwise) that you
 * acknowledge this fact somewhere in the documentation, research report,
 * etc...  If you like PCCTS and have developed a nice tool with the
 * output, please mention that you developed it using PCCTS.  In
 * addition, we ask that this header remain intact in our source code.
 * As long as these guidelines are kept, we expect to continue enhancing
 * this system and expect to make other tools available as they are
 * completed.
 *
 * ANTLR 1.33
 * Terence Parr
 * Parr Research Corporation
 * with Purdue University and AHPCRC, University of Minnesota
 * 1989-2001
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "pcctscfg.h"
#include "set.h"
#include "syn.h"
#include "hash.h"
#include "generic.h"
#include "dlgdef.h"

#define SetBlk(g, t, approx, first_set_symbol) {         		        \
			((Junction *)g.left)->jtype = t;					        \
			((Junction *)g.left)->approx = approx;				        \
			((Junction *)g.left)->pFirstSetSymbol = first_set_symbol;   \
			((Junction *)g.left)->end = (Junction *) g.right;	        \
			((Junction *)g.right)->jtype = EndBlk;}

/* Add the parameter string 'parm' to the parms field of a block-type junction
 * g.left points to the sentinel node on a block.  i.e. g.left->p1 points to
 * the actual junction with its jtype == some block-type.
 */
void
#ifdef __USE_PROTOS
addParm( Node *p, char *parm )
#else
addParm( p, parm )
Node *p;
char *parm;
#endif
{
	char *q = (char *) malloc( strlen(parm) + 1 );
	require(p!=NULL, "addParm: NULL object\n");
	require(q!=NULL, "addParm: unable to alloc parameter\n");

	strcpy(q, parm);
	if ( p->ntype == nRuleRef )
	{
		((RuleRefNode *)p)->parms = q;
	}
	else if ( p->ntype == nJunction )
	{
		((Junction *)p)->parm = q;	/* only one parameter allowed on subrules */
	}
	else fatal_internal("addParm: invalid node for adding parm");
}

/*
 * Build an action node for the syntax diagram
 *
 * buildAction(ACTION) ::= --o-->ACTION-->o--
 *
 * Where o is a junction node.
 */
Graph
#ifdef __USE_PROTOS
buildAction( char *action, int file, int line, int is_predicate )
#else
buildAction( action, file, line, is_predicate )
char *action;
int file;
int line;
int is_predicate;
#endif
{
	Junction *j1, *j2;
	Graph g;
	ActionNode *a;
	require(action!=NULL, "buildAction: invalid action");
	
	j1 = newJunction();
	j2 = newJunction();
	a = newActionNode();
	a->action = (char *) malloc( strlen(action)+1 );
	require(a->action!=NULL, "buildAction: cannot alloc space for action\n");
	strcpy(a->action, action);
	j1->p1 = (Node *) a;
	a->next = (Node *) j2;
	a->is_predicate = is_predicate;

    if (is_predicate) {
        PredEntry   *predEntry;
        char        *t;
        char        *key;
        char        *u;
        int         inverted=0;

        t=key=(char *)calloc(1,strlen(a->action)+1);

        for (u=a->action; *u != '\0' ; u++) {
          if (*u != ' ') {
            if (t==key && *u=='!') {
              inverted=!inverted;
            } else {
              *t++=*u;
            };
          };
        };

        *t='\0';


        predEntry=(PredEntry *)hash_get(Pname,key);
        a->predEntry=predEntry;
        if (predEntry != NULL) a->inverted=inverted;
    } else {
/* MR12c */      char  *strStart=a->action;
/* MR12c */      char  *strEnd;
/* MR12c */      strEnd=strStart+strlen(strStart)-1;
/* MR12c */      for ( ; strEnd >= strStart &&  isspace(*strEnd); strEnd--) *strEnd=0;
/* MR12c */      while (*strStart != '\0' && isspace(*strStart)) strStart++;
/* MR12c */      if (ci_strequ(strStart,"nohoist")) {
/* MR12c */        a->noHoist=1;
/* MR12c */      }
	}

	g.left = (Node *) j1; g.right = (Node *) j2;
	a->file = file;
	a->line = line;
	a->rname = CurRule;     /* MR10 */
	return g;
}

/*
 * Build a token node for the syntax diagram
 *
 * buildToken(TOKEN) ::= --o-->TOKEN-->o--
 *
 * Where o is a junction node.
 */
Graph
#ifdef __USE_PROTOS
buildToken( char *text )
#else
buildToken( text )
char *text;
#endif
{
	Junction *j1, *j2;
	Graph g;
	TokNode *t;
	require(text!=NULL, "buildToken: invalid token name");
	
	j1 = newJunction();
	j2 = newJunction();
	t = newTokNode();
	t->altstart = CurAltStart;
	if ( *text == '"' ) {t->label=FALSE; t->token = addTexpr( text );}
	else {t->label=TRUE; t->token = addTname( text );}
	j1->p1 = (Node *) t;
	t->next = (Node *) j2;
	g.left = (Node *) j1; g.right = (Node *) j2;
	return g;
}

/*
 * Build a wild-card node for the syntax diagram
 *
 * buildToken(TOKEN) ::= --o-->'.'-->o--
 *
 * Where o is a junction node.
 */
Graph
#ifdef __USE_PROTOS
buildWildCard( char *text )
#else
buildWildCard( text )
char *text;
#endif
{
	Junction *j1, *j2;
	Graph g;
	TokNode *t;
	TCnode *w;
	TermEntry *p;
	require(text!=NULL, "buildWildCard: invalid token name");
	
	j1 = newJunction();
	j2 = newJunction();
	t = newTokNode();

	/* If the ref a wild card, make a token class for it */
	if ( Tnum(WildCardString) == 0 )
	{
		w = newTCnode;
	  	w->tok = addTname( WildCardString );
		set_orel(w->tok, &imag_tokens);
		set_orel(w->tok, &tokclasses);
		WildCardToken = w->tok;
		require((p=(TermEntry *)hash_get(Tname, WildCardString)) != NULL,
				"hash table mechanism is broken");
		p->classname = 1;	/* entry is class name, not token */
		p->tclass = w;		/* save ptr to this tclass def */
		list_add(&tclasses, (char *)w);
	}
	else {
		p=(TermEntry *)hash_get(Tname, WildCardString);
		require( p!= NULL, "hash table mechanism is broken");
		w = p->tclass;
	}

	t->token = w->tok;
	t->wild_card = 1;
	t->tclass = w;

	t->altstart = CurAltStart;
	j1->p1 = (Node *) t;
	t->next = (Node *) j2;
	g.left = (Node *) j1; g.right = (Node *) j2;
	return g;
}

void
#ifdef __USE_PROTOS
setUpperRange(TokNode *t, char *text)
#else
setUpperRange(t, text)
TokNode *t;
char *text;
#endif
{
	require(t!=NULL, "setUpperRange: NULL token node");
	require(text!=NULL, "setUpperRange: NULL token string");

	if ( *text == '"' ) {t->upper_range = addTexpr( text );}
	else {t->upper_range = addTname( text );}
}

/*
 * Build a rule reference node of the syntax diagram
 *
 * buildRuleRef(RULE) ::= --o-->RULE-->o--
 *
 * Where o is a junction node.
 *
 * If rule 'text' has been defined already, don't alloc new space to store string.
 * Set r->text to point to old copy in string table.
 */
Graph
#ifdef __USE_PROTOS
buildRuleRef( char *text )
#else
buildRuleRef( text )
char *text;
#endif
{
	Junction *j1, *j2;
	Graph g;
	RuleRefNode *r;
	RuleEntry *p;
	require(text!=NULL, "buildRuleRef: invalid rule name");
	
	j1 = newJunction();
	j2 = newJunction();
	r = newRNode();
	r->altstart = CurAltStart;
	r->assign = NULL;
	if ( (p=(RuleEntry *)hash_get(Rname, text)) != NULL ) r->text = p->str;
	else r->text = mystrdup( text );
	j1->p1  = (Node *) r;
	r->next = (Node *) j2;
	g.left = (Node *) j1; g.right = (Node *) j2;
	return g;
}

/*
 * Or two subgraphs into one graph via:
 *
 * Or(G1, G2) ::= --o-G1-o--
 *                  |    ^
 *					v    |
 *                  o-G2-o
 *
 * Set the altnum of junction starting G2 to 1 + altnum of junction starting G1.
 * If, however, the G1 altnum is 0, make it 1 and then
 * make G2 altnum = G1 altnum + 1.
 */
Graph
#ifdef __USE_PROTOS
Or( Graph g1, Graph g2 )
#else
Or( g1, g2 )
Graph g1;
Graph g2;
#endif
{
	Graph g;
	require(g1.left != NULL, "Or: invalid graph");
	require(g2.left != NULL && g2.right != NULL, "Or: invalid graph");

	((Junction *)g1.left)->p2 = g2.left;
	((Junction *)g2.right)->p1 = g1.right;
	/* set altnums */
	if ( ((Junction *)g1.left)->altnum == 0 ) ((Junction *)g1.left)->altnum = 1;
	((Junction *)g2.left)->altnum = ((Junction *)g1.left)->altnum + 1;
	g.left = g2.left;
	g.right = g1.right;
	return g;
}

/*
 * Catenate two subgraphs
 *
 * Cat(G1, G2) ::= --o-G1-o-->o-G2-o--
 * Cat(NULL,G2)::= --o-G2-o--
 * Cat(G1,NULL)::= --o-G1-o--
 */
Graph
#ifdef __USE_PROTOS
Cat( Graph g1, Graph g2 )
#else
Cat( g1, g2 )
Graph g1;
Graph g2;
#endif
{
	Graph g;
	
	if ( g1.left == NULL && g1.right == NULL ) return g2;
	if ( g2.left == NULL && g2.right == NULL ) return g1;
	((Junction *)g1.right)->p1 = g2.left;
	g.left = g1.left;
	g.right = g2.right;
	return g;
}

/*
 * Make a subgraph an optional block
 *
 * makeOpt(G) ::= --o-->o-G-o-->o--
 *                      | 	    ^
 *						v  	    |
 *					    o-------o
 *
 * Note that this constructs {A|B|...|Z} as if (A|B|...|Z|) was found.
 *
 * The node on the far right is added so that every block owns its own
 * EndBlk node.
 */
Graph
#ifdef __USE_PROTOS
makeOpt( Graph g1, int approx, char * pFirstSetSymbol )
#else
makeOpt( g1, approx, pFirstSetSymbol )
Graph g1;
int approx;
char * pFirstSetSymbol;
#endif
{
	Junction *j1,*j2,*p;
	Graph g;
	require(g1.left != NULL && g1.right != NULL, "makeOpt: invalid graph");

	j1 = newJunction();
	j2 = newJunction();
	((Junction *)g1.right)->p1 = (Node *) j2;	/* add node to G at end */

    /*  MR21
     *
     *  There is code in genBlk which recognizes the node created
     *  by emptyAlt() as a special case and bypasses it.  We don't
     *  want this to happen for the optBlk.
     */

	g = emptyAlt3(); /* MR21 */
	if ( ((Junction *)g1.left)->altnum == 0 ) ((Junction *)g1.left)->altnum = 1;
	((Junction *)g.left)->altnum = ((Junction *)g1.left)->altnum + 1;
	for(p=(Junction *)g1.left; p->p2!=NULL; p=(Junction *)p->p2)
		{;}										/* find last alt */
	p->p2 = g.left;								/* add optional alternative */
	((Junction *)g.right)->p1 = (Node *)j2;		/* opt alt points to EndBlk */
	g1.right = (Node *)j2;
	SetBlk(g1, aOptBlk, approx, pFirstSetSymbol);
	j1->p1 = g1.left;							/* add generic node in front */
	g.left = (Node *) j1;
	g.right = g1.right;
	return g;
}

/*
 * Make a graph into subblock
 *
 * makeBlk(G) ::= --o-->o-G-o-->o--
 *
 * The node on the far right is added so that every block owns its own
 * EndBlk node.
 */
Graph
#ifdef __USE_PROTOS
makeBlk( Graph g1, int approx, char * pFirstSetSymbol )
#else
makeBlk( g1, approx, pFirstSetSymbol )
Graph g1;
int approx;
char * pFirstSetSymbol;
#endif
{
	Junction *j,*j2;
	Graph g;
	require(g1.left != NULL && g1.right != NULL, "makeBlk: invalid graph");

	j = newJunction();
	j2 = newJunction();
	((Junction *)g1.right)->p1 = (Node *) j2;	/* add node to G at end */
	g1.right = (Node *)j2;
	SetBlk(g1, aSubBlk, approx, pFirstSetSymbol);
	j->p1 = g1.left;							/* add node in front */
	g.left = (Node *) j;
	g.right = g1.right;

	return g;
}

/*
 * Make a subgraph into a loop (closure) block -- (...)*
 *
 * makeLoop(G) ::=       |---|
 *					     v   |
 *			   --o-->o-->o-G-o-->o--
 *                   |           ^
 *                   v           |
 *					 o-----------o
 *
 * After making loop, always place generic node out front.  It becomes
 * the start of enclosing block.  The aLoopBlk is the target of the loop.
 *
 * Loop blks have TWO EndBlk nodes--the far right and the node that loops back
 * to the aLoopBlk node.  Node with which we can branch past loop == aLoopBegin and
 * one which is loop target == aLoopBlk.
 * The branch-past (initial) aLoopBegin node has end
 * pointing to the last EndBlk node.  The loop-target node has end==NULL.
 *
 * Loop blocks have a set of locks (from 1..CLL_k) on the aLoopBlk node.
 */
Graph
#ifdef __USE_PROTOS
makeLoop( Graph g1, int approx, char * pFirstSetSymbol )
#else
makeLoop( g1, approx, pFirstSetSymbol)
Graph g1;
int approx;
char * pFirstSetSymbol;
#endif
{
	Junction *back, *front, *begin;
	Graph g;
	require(g1.left != NULL && g1.right != NULL, "makeLoop: invalid graph");

	back = newJunction();
	front = newJunction();
	begin = newJunction();
	g = emptyAlt3();
	((Junction *)g1.right)->p2 = g1.left;		/* add loop branch to G */
	((Junction *)g1.right)->p1 = (Node *) back;	/* add node to G at end */
	((Junction *)g1.right)->jtype = EndBlk;		/* mark 1st EndBlk node */
	((Junction *)g1.left)->jtype = aLoopBlk;	/* mark 2nd aLoopBlk node */
	((Junction *)g1.left)->end = (Junction *) g1.right;
	((Junction *)g1.left)->lock = makelocks();
	((Junction *)g1.left)->pred_lock = makelocks();
	g1.right = (Node *) back;
	begin->p1 = (Node *) g1.left;
	g1.left = (Node *) begin;
	begin->p2 = (Node *) g.left;				/* make bypass arc */
	((Junction *)g.right)->p1 = (Node *) back;
	SetBlk(g1, aLoopBegin, approx, pFirstSetSymbol);
	front->p1 = g1.left;						/* add node to front */
	g1.left = (Node *) front;

	return g1;
}

/*
 * Make a subgraph into a plus block -- (...)+ -- 1 or more times
 *
 * makePlus(G) ::=	 |---|
 *					 v   |
 *			   --o-->o-G-o-->o--
 *
 * After making loop, always place generic node out front.  It becomes
 * the start of enclosing block.  The aPlusBlk is the target of the loop.
 *
 * Plus blks have TWO EndBlk nodes--the far right and the node that loops back
 * to the aPlusBlk node.
 *
 * Plus blocks have a set of locks (from 1..CLL_k) on the aPlusBlk node.
 */
Graph
#ifdef __USE_PROTOS
makePlus( Graph g1, int approx, char * pFirstSetSymbol)
#else
makePlus( g1, approx, pFirstSetSymbol)
Graph g1;
int approx;
char * pFirstSetSymbol;
#endif
{
	int has_empty_alt_already = 0;
	Graph g;
	Junction *j2, *j3, *first_alt;
	Junction *last_alt=NULL, *p;
	require(g1.left != NULL && g1.right != NULL, "makePlus: invalid graph");

	first_alt = (Junction *)g1.left;
	j2 = newJunction();
	j3 = newJunction();
	if ( ((Junction *)g1.left)->altnum == 0 ) ((Junction *)g1.left)->altnum = 1;
	((Junction *)g1.right)->p2 = g1.left;		/* add loop branch to G */
	((Junction *)g1.right)->p1 = (Node *) j2;	/* add node to G at end */
	((Junction *)g1.right)->jtype = EndBlk;		/* mark 1st EndBlk node */
	g1.right = (Node *) j2;
	SetBlk(g1, aPlusBlk, approx, pFirstSetSymbol);
	((Junction *)g1.left)->lock = makelocks();
	((Junction *)g1.left)->pred_lock = makelocks();
	j3->p1 = g1.left;							/* add node to front */
	g1.left = (Node *) j3;

	/* add an optional branch which is the "exit" branch of loop */
	/* FIRST, check to ensure that there does not already exist
	 * an optional path.
	 */
	/* find last alt */
	for(p=first_alt; p!=NULL; p=(Junction *)p->p2)
	{
		if ( p->p1->ntype == nJunction &&
			 p->p1!=NULL &&
			 ((Junction *)p->p1)->jtype==Generic &&
			 ((Junction *)p->p1)->p1!=NULL &&
			 ((Junction *)((Junction *)p->p1)->p1)->jtype==EndBlk )
		{
			has_empty_alt_already = 1;
		}
		last_alt = p;
	}
	if ( !has_empty_alt_already )
	{
		require(last_alt!=NULL, "last_alt==NULL; bad (..)+");
		g = emptyAlt();
		last_alt->p2 = g.left;
		((Junction *)g.right)->p1 = (Node *) j2;

		/* make sure lookahead computation ignores this alt for
		* FIRST("(..)+"); but it's still used for computing the FIRST
		* of each alternative.
		*/
		((Junction *)g.left)->ignore = 1;
	}

	return g1;
}

/*
 * Return an optional path:  --o-->o--
 */

Graph
#ifdef __USE_PROTOS
emptyAlt( void )
#else
emptyAlt( )
#endif
{
	Junction *j1, *j2;
	Graph g;

	j1 = newJunction();
	j2 = newJunction();
	j1->p1 = (Node *) j2;
	g.left = (Node *) j1;
	g.right = (Node *) j2;
	
	return g;
}

/*  MR21
 *
 *  There is code in genBlk which recognizes the node created
 *  by emptyAlt() as a special case and bypasses it.  We don't
 *  want this to happen for the optBlk.
 */

Graph
#ifdef __USE_PROTOS
emptyAlt3( void )
#else
emptyAlt3( )
#endif
{
	Junction *j1, *j2, *j3;
	Graph g;

	j1 = newJunction();
	j2 = newJunction();
    j3 = newJunction();
	j1->p1 = (Node *) j2;
	j2->p1 = (Node *) j3;
	g.left = (Node *) j1;
	g.right = (Node *) j3;
	
	return g;
}

/* N o d e  A l l o c a t i o n */

TokNode *
#ifdef __USE_PROTOS
newTokNode( void )
#else
newTokNode( )
#endif
{
	static TokNode *FreeList = NULL;
	TokNode *p, *newblk;

	if ( FreeList == NULL )
	{
		newblk = (TokNode *)calloc(TokenBlockAllocSize, sizeof(TokNode));
		if ( newblk == NULL )
			fatal_internal(eMsg1("out of memory while building rule '%s'",CurRule));
		for (p=newblk; p<&(newblk[TokenBlockAllocSize]); p++)
		{
			p->next = (Node *)FreeList;	/* add all new token nodes to FreeList */
			FreeList = p;
		}
	}
	p = FreeList;
	FreeList = (TokNode *)FreeList->next;/* remove a TokNode node */
	p->next = NULL;						/* NULL the ptr we used */
    memset( (char *) p, 0, sizeof(TokNode));        /* MR10 */
	p->ntype = nToken;
	p->rname = CurRule;
	p->file = CurFile;
	p->line = zzline;
	p->altstart = NULL;

	return p;
}

RuleRefNode *
#ifdef __USE_PROTOS
newRNode( void )
#else
newRNode( )
#endif
{
	static RuleRefNode *FreeList = NULL;
	RuleRefNode *p, *newblk;

	if ( FreeList == NULL )
	{
		newblk = (RuleRefNode *)calloc(RRefBlockAllocSize, sizeof(RuleRefNode));
		if ( newblk == NULL )
			fatal_internal(eMsg1("out of memory while building rule '%s'",CurRule));
		for (p=newblk; p<&(newblk[RRefBlockAllocSize]); p++)
		{
			p->next = (Node *)FreeList;	/* add all new rref nodes to FreeList */
			FreeList = p;
		}
	}
	p = FreeList;
	FreeList = (RuleRefNode *)FreeList->next;/* remove a Junction node */
	p->next = NULL;						/* NULL the ptr we used */
    memset( (char *) p, 0, sizeof(RuleRefNode));        /* MR10 */
	p->ntype = nRuleRef;
	p->rname = CurRule;
	p->file = CurFile;
	p->line = zzline;
	p->astnode = ASTinclude;
	p->altstart = NULL;
	
	return p;
}

static int junctionSeqNumber=0;         /* MR10 */

Junction *
#ifdef __USE_PROTOS
newJunction( void )
#else
newJunction( )
#endif
{
	static Junction *FreeList = NULL;
	Junction *p, *newblk;

	if ( FreeList == NULL )
	{
		newblk = (Junction *)calloc(JunctionBlockAllocSize, sizeof(Junction));
		if ( newblk == NULL )
			fatal_internal(eMsg1("out of memory while building rule '%s'",CurRule));
		for (p=newblk; p<&(newblk[JunctionBlockAllocSize]); p++)
		{
			p->p1 = (Node *)FreeList;	/* add all new Junction nodes to FreeList */
			FreeList = p;
		}
	}
	p = FreeList;
	FreeList = (Junction *)FreeList->p1;/* remove a Junction node */
	p->p1 = NULL;						/* NULL the ptr we used */
    memset( (char *) p, 0, sizeof(Junction));       /* MR10 */
	p->ntype = nJunction;
	p->visited = 0;
	p->jtype = Generic;
	p->rname = CurRule;
	p->file = CurFile;
	p->line = zzline;
	p->exception_label = NULL;
	p->fset = (set *) calloc(CLL_k+1, sizeof(set));
	require(p->fset!=NULL, "cannot allocate fset in newJunction");
    p->seq=++junctionSeqNumber;     /* MR10 */

	return p;
}

ActionNode *
#ifdef __USE_PROTOS
newActionNode( void )
#else
newActionNode( )
#endif
{
	static ActionNode *FreeList = NULL;
	ActionNode *p, *newblk;

	if ( FreeList == NULL )
	{
		newblk = (ActionNode *)calloc(ActionBlockAllocSize, sizeof(ActionNode));
		if ( newblk == NULL )
			fatal_internal(eMsg1("out of memory while building rule '%s'",CurRule));
		for (p=newblk; p<&(newblk[ActionBlockAllocSize]); p++)
		{
			p->next = (Node *)FreeList;	/* add all new Action nodes to FreeList */
			FreeList = p;
		}
	}
	p = FreeList;
	FreeList = (ActionNode *)FreeList->next;/* remove an Action node */
    memset( (char *) p, 0, sizeof(ActionNode));     /* MR10 */
	p->ntype = nAction;
	p->next = NULL;						/* NULL the ptr we used */
	p->done = 0;
	p->pred_fail = NULL;
	p->guardpred = NULL;
    p->ampersandPred = NULL;
	return p;
}

/*
 * allocate the array of locks (1..CLL_k) used to inhibit infinite recursion.
 * Infinite recursion can occur in (..)* blocks, FIRST calcs and FOLLOW calcs.
 * Therefore, we need locks on aLoopBlk, RuleBlk, EndRule nodes.
 *
 * if ( lock[k]==TRUE ) then we have been here before looking for k tokens
 * of lookahead.
 */
char *
#ifdef __USE_PROTOS
makelocks( void )
#else
makelocks( )
#endif
{
	char *p = (char *) calloc(CLL_k+1, sizeof(char));
	require(p!=NULL, "cannot allocate lock array");
	
	return p;
}

#if 0
** #ifdef __USE_PROTOS
** void my_memset(char *p,char value,int count)
** #else
** void my_memset(p,value,count)
**   char      *p;
**   char      value;
**   int       count;
** #endif
** {
**    int      i;
**
**    for (i=0; i<count; i++) {
**     p[i]=value;
**   };
** }
#endif
