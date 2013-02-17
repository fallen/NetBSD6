/*	$NetBSD: boolean.c,v 1.1.1.1 2009/12/13 16:55:10 kardel Exp $	*/


/*
 *  Id: 329b43154b88d78564d8f960a00a83ec7d8baee0
 * Time-stamp:      "2008-08-03 13:06:02 bkorb"
 *
 *   Automated Options Paged Usage module.
 *
 *  This routine will run run-on options through a pager so the
 *  user may examine, print or edit them at their leisure.
 *
 *  This file is part of AutoOpts, a companion to AutoGen.
 *  AutoOpts is free software.
 *  AutoOpts is copyright (c) 1992-2009 by Bruce Korb - all rights reserved
 *
 *  AutoOpts is available under any one of two licenses.  The license
 *  in use must be one of these two and the choice is under the control
 *  of the user of the license.
 *
 *   The GNU Lesser General Public License, version 3 or later
 *      See the files "COPYING.lgplv3" and "COPYING.gplv3"
 *
 *   The Modified Berkeley Software Distribution License
 *      See the file "COPYING.mbsd"
 *
 *  These files have the following md5sums:
 *
 *  43b91e8ca915626ed3818ffb1b71248b pkg/libopts/COPYING.gplv3
 *  06a1a2e4760c90ea5e1dad8dfaac4d39 pkg/libopts/COPYING.lgplv3
 *  66a5cedaf62c4b2637025f049f9b826f pkg/libopts/COPYING.mbsd
 */

/*=export_func  optionBooleanVal
 * private:
 *
 * what:  Decipher a boolean value
 * arg:   + tOptions* + pOpts    + program options descriptor +
 * arg:   + tOptDesc* + pOptDesc + the descriptor for this arg +
 *
 * doc:
 *  Decipher a true or false value for a boolean valued option argument.
 *  The value is true, unless it starts with 'n' or 'f' or "#f" or
 *  it is an empty string or it is a number that evaluates to zero.
=*/
void
optionBooleanVal( tOptions* pOpts, tOptDesc* pOD )
{
    char* pz;
    ag_bool  res = AG_TRUE;

    if ((pOD->fOptState & OPTST_RESET) != 0)
        return;

    if (pOD->optArg.argString == NULL) {
        pOD->optArg.argBool = AG_FALSE;
        return;
    }

    switch (*(pOD->optArg.argString)) {
    case '0':
    {
        long  val = strtol( pOD->optArg.argString, &pz, 0 );
        if ((val != 0) || (*pz != NUL))
            break;
        /* FALLTHROUGH */
    }
    case 'N':
    case 'n':
    case 'F':
    case 'f':
    case NUL:
        res = AG_FALSE;
        break;
    case '#':
        if (pOD->optArg.argString[1] != 'f')
            break;
        res = AG_FALSE;
    }

    if (pOD->fOptState & OPTST_ALLOC_ARG) {
        AGFREE(pOD->optArg.argString);
        pOD->fOptState &= ~OPTST_ALLOC_ARG;
    }
    pOD->optArg.argBool = res;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/boolean.c */
