/* ----------------------- **
**      Struct worker      **
** ----------------------- */

extern struct worker{
  void *worker_area[MAX_WORKERS];
  long worker_offset[MAX_WORKERS];
} WORKER;

#define worker_area(W)    (WORKER.worker_area[W])
#define worker_offset(W)  (WORKER.worker_offset[W])



/* ------------------------- **
**      Struct or_frame      **
** ------------------------- */

typedef struct or_frame {
  lockvar lock;
  yamop *alternative;
  volatile bitmap members;
  choiceptr node;
  struct or_frame *nearest_livenode;
  /* cut support */
  int depth;
  choiceptr pending_prune_cp;
  volatile int pending_prune_ltt;
  struct or_frame *nearest_leftnode;
  struct query_goal_solution_frame *query_solutions;
#ifdef TABLING_INNER_CUTS
  struct table_subgoal_solution_frame *table_solutions;
#endif /* TABLING_INNER_CUTS */
#ifdef TABLING
  /* tabling support */
  volatile int number_owners;
  struct or_frame *next_on_stack;
  struct suspension_frame *suspensions;
  struct or_frame *nearest_suspension_node;
#endif /* TABLING */
  struct or_frame *next;
} *or_fr_ptr;

#define OrFr_lock(X)              ((X)->lock)
#define OrFr_alternative(X)       ((X)->alternative)
#define OrFr_members(X)           ((X)->members)
#define OrFr_node(X)              ((X)->node)
#define OrFr_nearest_livenode(X)  ((X)->nearest_livenode)
#define OrFr_depth(X)             ((X)->depth)
#define OrFr_pend_prune_cp(X)     ((X)->pending_prune_cp)
#define OrFr_pend_prune_ltt(X)    ((X)->pending_prune_ltt)
#define OrFr_nearest_leftnode(X)  ((X)->nearest_leftnode)
#define OrFr_qg_solutions(X)      ((X)->query_solutions)
#define OrFr_tg_solutions(X)      ((X)->table_solutions)
#define OrFr_owners(X)            ((X)->number_owners)
#ifdef TABLING
#define OrFr_next_on_stack(X)     ((X)->next_on_stack)
#else
#define OrFr_next_on_stack(X)     ((X)->next)
#endif /* TABLING */
#define OrFr_suspensions(X)       ((X)->suspensions)
#define OrFr_nearest_suspnode(X)  ((X)->nearest_suspension_node)
#define OrFr_next(X)              ((X)->next)



/* ------------------------------------------ **
**      Struct query_goal_solution_frame      **
** ------------------------------------------ */

typedef struct query_goal_solution_frame{
  volatile int ltt;
  struct query_goal_answer_frame *first;
  struct query_goal_answer_frame *last;
  struct query_goal_solution_frame *next;
} *qg_sol_fr_ptr;

#define SolFr_ltt(X)    ((X)->ltt)
#define SolFr_first(X)  ((X)->first)
#define SolFr_last(X)   ((X)->last)
#define SolFr_next(X)   ((X)->next)



/* ---------------------------------------- **
**      Struct query_goal_answer_frame      **
** ---------------------------------------- */

typedef struct query_goal_answer_frame{
  char answer[MAX_LENGTH_ANSWER];
  struct query_goal_answer_frame *next;
} *qg_ans_fr_ptr;

#define AnsFr_answer(X)  ((X)->answer)
#define AnsFr_next(X)    ((X)->next)



#ifdef TABLING_INNER_CUTS
/* --------------------------------------------- **
**      Struct table_subgoal_solution_frame      **
** --------------------------------------------- */

typedef struct table_subgoal_solution_frame{
  choiceptr generator_choice_point;  
  volatile int ltt;
  struct table_subgoal_answer_frame *first_answer_frame;
  struct table_subgoal_answer_frame *last_answer_frame;
  struct table_subgoal_solution_frame *ltt_next;
  struct table_subgoal_solution_frame *next;
} *tg_sol_fr_ptr;

#define TgSolFr_gen_cp(X)    ((X)->generator_choice_point)
#define TgSolFr_ltt(X)       ((X)->ltt)
#define TgSolFr_first(X)     ((X)->first_answer_frame)
#define TgSolFr_last(X)      ((X)->last_answer_frame)
#define TgSolFr_ltt_next(X)  ((X)->ltt_next)
#define TgSolFr_next(X)      ((X)->next)



/* ------------------------------------------- **
**      Struct table_subgoal_answer_frame      **
** ------------------------------------------- */

typedef struct table_subgoal_answer_frame{
  volatile int next_free_slot;
  struct answer_trie_node *answer[TG_ANSWER_SLOTS];
  struct table_subgoal_answer_frame *next;
} *tg_ans_fr_ptr;

#define TgAnsFr_free_slot(X)  ((X)->next_free_slot)
#define TgAnsFr_answer(X,N)   ((X)->answer[N])
#define TgAnsFr_next(X)       ((X)->next)
#endif /* TABLING_INNER_CUTS */
