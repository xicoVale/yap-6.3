/*************************************************************************
*									 *
*	 YAP Prolog 							 *
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		mavar.c   						 *
* Last rev:								 *
* mods:									 *
* comments:	support from multiple assignment variables in YAP	 *
*									 *
*************************************************************************/

#include "Yap.h"

#ifdef MULTI_ASSIGNMENT_VARIABLES

#include "Yatom.h"
#include "Heap.h"
#include "eval.h"

STD_PROTO(static Int p_setarg, (void));
STD_PROTO(static Int p_create_mutable, (void));
STD_PROTO(static Int p_get_mutable, (void));
STD_PROTO(static Int p_update_mutable, (void));
STD_PROTO(static Int p_is_mutable, (void));

static Int
p_setarg(void)
{
  CELL ti = Deref(ARG1), ts = Deref(ARG2);
  Int i;
  if (IsVarTerm(ti)) {
    Error(INSTANTIATION_ERROR,ti,"setarg/3");
    return(FALSE);
  } else {
    if (IsIntTerm(ti))
      i = IntOfTerm(ti);
    else {
      union arith_ret v;
      if (Eval(ti, &v) == long_int_e) {
	i = v.Int;
      } else {
	Error(TYPE_ERROR_INTEGER,ti,"setarg/3");
	return(FALSE);
      }
    }
  }
  if (IsVarTerm(ts)) {
    Error(INSTANTIATION_ERROR,ts,"setarg/3");
  } else if(IsApplTerm(ts)) {
    CELL *pt;
    if (IsExtensionFunctor(FunctorOfTerm(ts))) {
      Error(TYPE_ERROR_COMPOUND,ts,"setarg/3");
      return(FALSE);
    }
    if (i < 0 || i > (Int)ArityOfFunctor(FunctorOfTerm(ts))) {
      if (i<0)
	Error(DOMAIN_ERROR_NOT_LESS_THAN_ZERO,ts,"setarg/3");
      return(FALSE);
    }
    pt = RepAppl(ts)+i;
    /* the evil deed is to be done now */
    MaBind(pt, Deref(ARG3));
  } else if(IsPairTerm(ts)) {
    CELL *pt;
    if (i != 1 || i != 2) {
      if (i<0)
	Error(DOMAIN_ERROR_NOT_LESS_THAN_ZERO,ts,"setarg/3");
      return(FALSE);
    }
    pt = RepPair(ts)+i-1;
    /* the evil deed is to be done now */
    MaBind(pt, Deref(ARG3));    
  } else {
    Error(TYPE_ERROR_COMPOUND,ts,"setarg/3");
    return(FALSE);
  }
  return(TRUE);
}


/* One problem with MAVars is that they you always trail on
   non-determinate bindings. This is not cool if you have a long
   determinate computation. One alternative could be to use
   timestamps.

   Because of !, the only timestamp one can trust is the trailpointer
   (ouch..). The trail is not reclaimed after cuts. Also, if there was
   a conditional binding, the trail is sure to have been increased
   since the last choicepoint. For maximum effect, we can actually
   store the current value of TR in the timestamp field, giving a way
   to actually follow a link of all trailings for these variables.

*/

/* create and initialise a new timed var. The problem is: how to set
   the clock?

   If I give it the current value of B->TR, we may have trouble if no
   non-determinate bindings are made before the next
   choice-point. Just to make sure this doesn't cause trouble, if (TR
   == B->TR) we will add a little something ;-).    
 */

#if FROZEN_STACKS
static void
CreateTimedVar(Term val)
{
  timed_var *tv = (timed_var *)H;
  tv->clock = MkIntegerTerm(B->cp_tr-(tr_fr_ptr)TrailBase);
  if (B->cp_tr == TR) {
    /* we run the risk of not making non-determinate bindings before
       the end of the night */
    /* so we just init a TR cell that will not harm anyone */
    Bind((CELL *)(TR+1),AbsAppl(H-1));
  }
  tv->value = val;
  H += sizeof(timed_var)/sizeof(CELL);
}

static void
CreateEmptyTimedVar(void)
{
  timed_var *tv = (timed_var *)H;
  tv->clock = MkIntegerTerm(B->cp_tr-(tr_fr_ptr)TrailBase);
  if (B->cp_tr == TR) {
    /* we run the risk of not making non-determinate bindings before
       the end of the night */
    /* so we just init a TR cell that will not harm anyone */
    Bind((CELL *)(TR+1),AbsAppl(H-1));
  }
  RESET_VARIABLE(&(tv->value));
  H += sizeof(timed_var)/sizeof(CELL);
}
#endif

Term NewTimedVar(CELL val)
{
  Term out = AbsAppl(H);
#if FROZEN_STACKS
  *H++ = (CELL)FunctorMutable;
  CreateTimedVar(val);
#else
  timed_var *tv;
  *H++ = (CELL)FunctorMutable;
  tv = (timed_var *)H;
  RESET_VARIABLE(&(tv->clock));
  tv->value = val;
  H += sizeof(timed_var)/sizeof(CELL);
#endif
  return(out);
}

Term NewEmptyTimedVar(void)
{
  Term out = AbsAppl(H);
#if FROZEN_STACKS
  *H++ = (CELL)FunctorMutable;
  CreateEmptyTimedVar();
#else
  timed_var *tv;
  *H++ = (CELL)FunctorMutable;
  tv = (timed_var *)H;
  RESET_VARIABLE(&(tv->clock));
  RESET_VARIABLE(&(tv->value));
  H += sizeof(timed_var)/sizeof(CELL);
#endif
  return(out);
}

Term ReadTimedVar(Term inv)
{
  timed_var *tv = (timed_var *)(RepAppl(inv)+1);
  return(tv->value);
}


/* update a timed var with a new value */
Term UpdateTimedVar(Term inv, Term new)
{
  timed_var *tv = (timed_var *)(RepAppl(inv)+1);
  CELL t = tv->value;
#if FROZEN_STACKS
  tr_fr_ptr timestmp = (tr_fr_ptr)TrailBase + IntegerOfTerm(tv->clock);

  if (B->cp_tr <= timestmp && timestmp <= TR) {
    /* last assignment more recent than last B */
#if SBA
    if (Unsigned((Int)(tv)-(Int)(H_FZ)) >
	Unsigned((Int)(B_FZ)-(Int)(H_FZ)))
      *STACK_TO_SBA(&(tv->value)) = new;
    else
#endif
      tv->value = new;
    if (Unsigned((Int)(tv)-(Int)(HBREG)) >
	Unsigned(BBREG)-(Int)(HBREG))
      TrailVal(timestmp-1) = new;
  } else {
    Term nclock;
    MaBind(&(tv->value), new);
    nclock = MkIntegerTerm(TR-(tr_fr_ptr)TrailBase);
    MaBind(&(tv->clock), nclock);
  }
#else
  CELL* timestmp = (CELL *)(tv->clock);

  if (B->cp_h <= timestmp) {
    /* last assignment more recent than last B */
#if SBA
    if (Unsigned((Int)(tv)-(Int)(H_FZ)) >
	Unsigned((Int)(B_FZ)-(Int)(H_FZ)))
      *STACK_TO_SBA(&(tv->value)) = new;
    else
#endif
      tv->value = new;
  } else {
    Term nclock = (Term)H;
    MaBind(&(tv->value), new);
    *H++ = TermFoundVar;
    MaBind(&(tv->clock), nclock);
  }
#endif
  return(t);
}

static Int
p_create_mutable(void)
{
  Term t = NewTimedVar(Deref(ARG1));
  return(unify(ARG2,t));
}

static Int
p_get_mutable(void)
{
  Term t = Deref(ARG2);
  if (IsVarTerm(t)) {
    Error(INSTANTIATION_ERROR, t, "get_mutable/3");
    return(FALSE);
  }
  if (!IsApplTerm(t)) {
    Error(TYPE_ERROR_COMPOUND,t,"get_mutable/3");
    return(FALSE);
  }
  if (FunctorOfTerm(t) != FunctorMutable) { 
    Error(DOMAIN_ERROR_MUTABLE,t,"get_mutable/3");
    return(FALSE);
  }
  t = ReadTimedVar(t);
  return(unify(ARG1, t));
}

static Int
p_update_mutable(void)
{
  Term t = Deref(ARG2);
  if (IsVarTerm(t)) {
    Error(INSTANTIATION_ERROR, t, "update_mutable/3");
    return(FALSE);
  }
  if (!IsApplTerm(t)) {
    Error(TYPE_ERROR_COMPOUND,t,"update_mutable/3");
    return(FALSE);
  }
  if (FunctorOfTerm(t) != FunctorMutable) { 
    Error(DOMAIN_ERROR_MUTABLE,t,"update_mutable/3");
    return(FALSE);
  }
  UpdateTimedVar(t, Deref(ARG1));
  return(TRUE);
}

static Int
p_is_mutable(void)
{
  Term t = Deref(ARG1);
  if (IsVarTerm(t)) {
    return(FALSE);
  }
  if (!IsApplTerm(t)) {
    return(FALSE);
  }
  if (FunctorOfTerm(t) != FunctorMutable) { 
    return(FALSE);
  }
  return(TRUE);
}

#endif

void
InitMaVarCPreds(void)
{
#ifdef MULTI_ASSIGNMENT_VARIABLES
  /* The most famous contributions of SICStus to the Prolog language */
  InitCPred("setarg", 3, p_setarg, SafePredFlag);  
  InitCPred("create_mutable", 2, p_create_mutable, SafePredFlag);  
  InitCPred("get_mutable", 2, p_get_mutable, SafePredFlag);  
  InitCPred("update_mutable", 2, p_update_mutable, SafePredFlag);  
  InitCPred("is_mutable", 1, p_is_mutable, SafePredFlag);  
#endif
}
