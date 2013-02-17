/* Web construction code for GNU compiler.
   Contributed by Jan Hubicka.
   Copyright (C) 2001, 2002, 2004 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

/* Simple optimization pass that splits independent uses of each pseudo,
   increasing effectiveness of other optimizations.  The optimization can
   serve as an example of use for the dataflow module.

   We don't split registers with REG_USERVAR set unless -fmessy-debugging
   is specified, because debugging information about such split variables
   is almost unusable.

   TODO
    - Add code to keep debugging up-to-date after splitting user variable
      pseudos.  This can be done by keeping track of all the pseudos used
      for the variable and using life analysis information before reload
      to determine which one is live and, in case more than one are live,
      choose the one with the latest definition.

      Other optimization passes can benefit from the infrastructure too.

    - We may use profile information and ignore infrequent use for the
      purpose of web unifying, inserting the compensation code later to
      implement full induction variable expansion for loops (currently
      we expand only if the induction variable is dead afterward, which
      is often the case).  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "toplev.h"

#include "rtl.h"
#include "hard-reg-set.h"
#include "flags.h"
#include "obstack.h"
#include "basic-block.h"
#include "output.h"
#include "df.h"
#include "function.h"
#include "timevar.h"
#include "tree-pass.h"


/* This entry is allocated for each reference in the insn stream.  */
struct web_entry
{
  /* Pointer to the parent in the union/find tree.  */
  struct web_entry *pred;
  /* Newly assigned register to the entry.  Set only for roots.  */
  rtx reg;
};

static struct web_entry *unionfind_root (struct web_entry *);
static void unionfind_union (struct web_entry *, struct web_entry *);
static void union_defs (struct df *, struct ref *, struct web_entry *, 
                        struct web_entry *);
static rtx entry_register (struct web_entry *, struct ref *, char *);
static void replace_ref (struct ref *, rtx);

/* Find the root of unionfind tree (the representative of set).  */

static struct web_entry *
unionfind_root (struct web_entry *element)
{
  struct web_entry *element1 = element, *element2;

  while (element->pred)
    element = element->pred;
  while (element1->pred)
    {
      element2 = element1->pred;
      element1->pred = element;
      element1 = element2;
    }
  return element;
}

/* Union sets.  */

static void
unionfind_union (struct web_entry *first, struct web_entry *second)
{
  first = unionfind_root (first);
  second = unionfind_root (second);
  if (first == second)
    return;
  second->pred = first;
}

/* For each use, all possible defs reaching it must come in the same
   register, union them.  */

static void
union_defs (struct df *df, struct ref *use, struct web_entry *def_entry,
            struct web_entry *use_entry)
{
  rtx insn = DF_REF_INSN (use);
  struct df_link *link = DF_REF_CHAIN (use);
  struct df_link *use_link = DF_INSN_USES (df, insn);
  struct df_link *def_link = DF_INSN_DEFS (df, insn);
  rtx set = single_set (insn);

  /* Some instructions may use match_dup for their operands.  In case the
     operands are dead, we will assign them different pseudos, creating
     invalid instructions, so union all uses of the same operand for each
     insn.  */

  while (use_link)
    {
      if (use != use_link->ref
	  && DF_REF_REAL_REG (use) == DF_REF_REAL_REG (use_link->ref))
	unionfind_union (use_entry + DF_REF_ID (use),
		         use_entry + DF_REF_ID (use_link->ref));
      use_link = use_link->next;
    }

  /* Recognize trivial noop moves and attempt to keep them as noop.
     While most of noop moves should be removed, we still keep some
     of them at libcall boundaries and such.  */

  if (set
      && SET_SRC (set) == DF_REF_REG (use)
      && SET_SRC (set) == SET_DEST (set))
    {
      while (def_link)
	{
	  if (DF_REF_REAL_REG (use) == DF_REF_REAL_REG (def_link->ref))
	    unionfind_union (use_entry + DF_REF_ID (use),
			     def_entry + DF_REF_ID (def_link->ref));
	  def_link = def_link->next;
	}
    }
  while (link)
    {
      unionfind_union (use_entry + DF_REF_ID (use),
		       def_entry + DF_REF_ID (link->ref));
      link = link->next;
    }

  /* A READ_WRITE use requires the corresponding def to be in the same
     register.  Find it and union.  */
  if (use->flags & DF_REF_READ_WRITE)
    {
      struct df_link *link = DF_INSN_DEFS (df, DF_REF_INSN (use));

      while (link)
	{
	  if (DF_REF_REAL_REG (link->ref) == DF_REF_REAL_REG (use))
	    unionfind_union (use_entry + DF_REF_ID (use),
			     def_entry + DF_REF_ID (link->ref));
	  link = link->next;
	}
    }
}

/* Find the corresponding register for the given entry.  */

static rtx
entry_register (struct web_entry *entry, struct ref *ref, char *used)
{
  struct web_entry *root;
  rtx reg, newreg;

  /* Find the corresponding web and see if it has been visited.  */
  root = unionfind_root (entry);
  if (root->reg)
    return root->reg;

  /* We are seeing this web for the first time, do the assignment.  */
  reg = DF_REF_REAL_REG (ref);

  /* In case the original register is already assigned, generate new one.  */
  if (!used[REGNO (reg)])
    newreg = reg, used[REGNO (reg)] = 1;
  else if (REG_USERVAR_P (reg) && 0/*&& !flag_messy_debugging*/)
    {
      newreg = reg;
      if (dump_file)
	fprintf (dump_file,
		 "New web forced to keep reg=%i (user variable)\n",
		 REGNO (reg));
    }
  else
    {
      newreg = gen_reg_rtx (GET_MODE (reg));
      REG_USERVAR_P (newreg) = REG_USERVAR_P (reg);
      REG_POINTER (newreg) = REG_POINTER (reg);
      REG_ATTRS (newreg) = REG_ATTRS (reg);
      if (dump_file)
	fprintf (dump_file, "Web oldreg=%i newreg=%i\n", REGNO (reg),
		 REGNO (newreg));
    }

  root->reg = newreg;
  return newreg;
}

/* Replace the reference by REG.  */

static void
replace_ref (struct ref *ref, rtx reg)
{
  rtx oldreg = DF_REF_REAL_REG (ref);
  rtx *loc = DF_REF_REAL_LOC (ref);

  if (oldreg == reg)
    return;
  if (dump_file)
    fprintf (dump_file, "Updating insn %i (%i->%i)\n",
	     INSN_UID (DF_REF_INSN (ref)), REGNO (oldreg), REGNO (reg)); 
  *loc = reg;
}

/* Main entry point.  */

void
web_main (void)
{
  struct df *df;
  struct web_entry *def_entry;
  struct web_entry *use_entry;
  unsigned int i;
  int max = max_reg_num ();
  char *used;

  df = df_init ();
  df_analyze (df, 0, DF_UD_CHAIN | DF_EQUIV_NOTES);

  def_entry = xcalloc (df->n_defs, sizeof (struct web_entry));
  use_entry = xcalloc (df->n_uses, sizeof (struct web_entry));
  used = xcalloc (max, sizeof (char));

  if (dump_file)
    df_dump (df, DF_UD_CHAIN | DF_DU_CHAIN, dump_file);

  /* Produce the web.  */
  for (i = 0; i < df->n_uses; i++)
    union_defs (df, df->uses[i], def_entry, use_entry);

  /* Update the instruction stream, allocating new registers for split pseudos
     in progress.  */
  for (i = 0; i < df->n_uses; i++)
    replace_ref (df->uses[i], entry_register (use_entry + i, df->uses[i],
					      used));
  for (i = 0; i < df->n_defs; i++)
    replace_ref (df->defs[i], entry_register (def_entry + i, df->defs[i],
					      used));

  /* Dataflow information is corrupt here, but it can be easily updated
     by creating new entries for new registers and updates or calling
     df_insns_modify.  */
  free (def_entry);
  free (use_entry);
  free (used);
  df_finish (df);
}

static bool
gate_handle_web (void)
{
  return (optimize > 0 && flag_web);
}

static void
rest_of_handle_web (void)
{
  web_main ();
  delete_trivially_dead_insns (get_insns (), max_reg_num ());
  cleanup_cfg (CLEANUP_EXPENSIVE);
  reg_scan (get_insns (), max_reg_num ());
}

struct tree_opt_pass pass_web =
{
  "web",                                /* name */
  gate_handle_web,                      /* gate */
  rest_of_handle_web,                   /* execute */
  NULL,                                 /* sub */
  NULL,                                 /* next */
  0,                                    /* static_pass_number */
  TV_WEB,                               /* tv_id */
  0,                                    /* properties_required */
  0,                                    /* properties_provided */
  0,                                    /* properties_destroyed */
  0,                                    /* todo_flags_start */
  TODO_dump_func,                       /* todo_flags_finish */
  'Z'                                   /* letter */
};

