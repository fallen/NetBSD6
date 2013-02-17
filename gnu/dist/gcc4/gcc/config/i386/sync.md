;; GCC machine description for i386 synchronization instructions.
;; Copyright (C) 2005
;; Free Software Foundation, Inc.
;;
;; This file is part of GCC.
;;
;; GCC is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.
;;
;; GCC is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING.  If not, write to
;; the Free Software Foundation, 51 Franklin Street, Fifth Floor,
;; Boston, MA 02110-1301, USA.

(define_mode_macro IMODE [QI HI SI (DI "TARGET_64BIT")])
(define_mode_attr modesuffix [(QI "b") (HI "w") (SI "l") (DI "q")])
(define_mode_attr modeconstraint [(QI "q") (HI "r") (SI "r") (DI "r")])
(define_mode_attr immconstraint [(QI "i") (HI "i") (SI "i") (DI "e")])

;; ??? It would be possible to use cmpxchg8b on pentium for DImode
;; changes.  It's complicated because the insn uses ecx:ebx as the
;; new value; note that the registers are reversed from the order
;; that they'd be in with (reg:DI 2 ecx).  Similarly for TImode 
;; data in 64-bit mode.

(define_insn "sync_compare_and_swap<mode>"
  [(set (match_operand:IMODE 0 "register_operand" "=a")
	(match_operand:IMODE 1 "memory_operand" "+m"))
   (set (match_dup 1)
	(unspec_volatile:IMODE
	  [(match_dup 1)
	   (match_operand:IMODE 2 "register_operand" "a")
	   (match_operand:IMODE 3 "register_operand" "<modeconstraint>")]
	  UNSPECV_CMPXCHG_1))
   (clobber (reg:CC FLAGS_REG))]
  "TARGET_CMPXCHG"
  "lock\;cmpxchg{<modesuffix>}\t{%3, %1|%1, %3}")

(define_expand "sync_compare_and_swap_cc<mode>"
  [(parallel
    [(set (match_operand:IMODE 0 "register_operand" "")
	  (match_operand:IMODE 1 "memory_operand" ""))
     (set (match_dup 1)
	  (unspec_volatile:IMODE
	    [(match_dup 1)
	     (match_operand:IMODE 2 "register_operand" "")
	     (match_operand:IMODE 3 "register_operand" "")]
	    UNSPECV_CMPXCHG_1))
     (set (match_dup 4)
	  (compare:CCZ
	    (unspec_volatile:IMODE
	      [(match_dup 1) (match_dup 2) (match_dup 3)] UNSPECV_CMPXCHG_2)
	    (match_dup 2)))])]
  "TARGET_CMPXCHG"
{
  operands[4] = gen_rtx_REG (CCZmode, FLAGS_REG);
  ix86_compare_op0 = operands[3];
  ix86_compare_op1 = NULL;
  ix86_compare_emitted = operands[4];
})

(define_insn "*sync_compare_and_swap_cc<mode>"
  [(set (match_operand:IMODE 0 "register_operand" "=a")
	(match_operand:IMODE 1 "memory_operand" "+m"))
   (set (match_dup 1)
	(unspec_volatile:IMODE
	  [(match_dup 1)
	   (match_operand:IMODE 2 "register_operand" "a")
	   (match_operand:IMODE 3 "register_operand" "<modeconstraint>")]
	  UNSPECV_CMPXCHG_1))
   (set (reg:CCZ FLAGS_REG)
	(compare:CCZ
	  (unspec_volatile:IMODE
	    [(match_dup 1) (match_dup 2) (match_dup 3)] UNSPECV_CMPXCHG_2)
	  (match_dup 2)))]
  "TARGET_CMPXCHG"
  "lock\;cmpxchg{<modesuffix>}\t{%3, %1|%1, %3}")

(define_insn "sync_old_add<mode>"
  [(set (match_operand:IMODE 0 "register_operand" "=<modeconstraint>")
	(unspec_volatile:IMODE
	  [(match_operand:IMODE 1 "memory_operand" "+m")] UNSPECV_XCHG))
   (set (match_dup 1)
	(plus:IMODE (match_dup 1)
		    (match_operand:IMODE 2 "register_operand" "0")))
   (clobber (reg:CC FLAGS_REG))]
  "TARGET_XADD"
  "lock\;xadd{<modesuffix>}\t{%0, %1|%1, %0}")

;; Recall that xchg implicitly sets LOCK#, so adding it again wastes space.
(define_insn "sync_lock_test_and_set<mode>"
  [(set (match_operand:IMODE 0 "register_operand" "=<modeconstraint>")
	(unspec_volatile:IMODE
	  [(match_operand:IMODE 1 "memory_operand" "+m")] UNSPECV_XCHG))
   (set (match_dup 1)
	(match_operand:IMODE 2 "register_operand" "0"))]
  ""
  "xchg{<modesuffix>}\t{%1, %0|%0, %1}")

(define_insn "sync_add<mode>"
  [(set (match_operand:IMODE 0 "memory_operand" "=m")
	(unspec_volatile:IMODE
	  [(plus:IMODE (match_dup 0)
	     (match_operand:IMODE 1 "nonmemory_operand" "<modeconstraint><immconstraint>"))]
	  UNSPECV_LOCK))
   (clobber (reg:CC FLAGS_REG))]
  ""
  "lock\;add{<modesuffix>}\t{%1, %0|%0, %1}")

(define_insn "sync_sub<mode>"
  [(set (match_operand:IMODE 0 "memory_operand" "=m")
	(unspec_volatile:IMODE
	  [(minus:IMODE (match_dup 0)
	     (match_operand:IMODE 1 "nonmemory_operand" "<modeconstraint><immconstraint>"))]
	  UNSPECV_LOCK))
   (clobber (reg:CC FLAGS_REG))]
  ""
  "lock\;sub{<modesuffix>}\t{%1, %0|%0, %1}")

(define_insn "sync_ior<mode>"
  [(set (match_operand:IMODE 0 "memory_operand" "=m")
	(unspec_volatile:IMODE
	  [(ior:IMODE (match_dup 0)
	     (match_operand:IMODE 1 "nonmemory_operand" "<modeconstraint><immconstraint>"))]
	  UNSPECV_LOCK))
   (clobber (reg:CC FLAGS_REG))]
  ""
  "lock\;or{<modesuffix>}\t{%1, %0|%0, %1}")

(define_insn "sync_and<mode>"
  [(set (match_operand:IMODE 0 "memory_operand" "=m")
	(unspec_volatile:IMODE
	  [(and:IMODE (match_dup 0)
	     (match_operand:IMODE 1 "nonmemory_operand" "<modeconstraint><immconstraint>"))]
	  UNSPECV_LOCK))
   (clobber (reg:CC FLAGS_REG))]
  ""
  "lock\;and{<modesuffix>}\t{%1, %0|%0, %1}")

(define_insn "sync_xor<mode>"
  [(set (match_operand:IMODE 0 "memory_operand" "=m")
	(unspec_volatile:IMODE
	  [(xor:IMODE (match_dup 0)
	     (match_operand:IMODE 1 "nonmemory_operand" "<modeconstraint><immconstraint>"))]
	  UNSPECV_LOCK))
   (clobber (reg:CC FLAGS_REG))]
  ""
  "lock\;xor{<modesuffix>}\t{%1, %0|%0, %1}")
