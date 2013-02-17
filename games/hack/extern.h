/*	$NetBSD: extern.h,v 1.18 2011/08/27 23:42:33 joerg Exp $	*/

/*-
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Christos Zoulas.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _EXTERN_H_
#define _EXTERN_H_
#include <stdarg.h>
#include <stdio.h>

/* alloc.c */
void *alloc(size_t);

/* hack.apply.c */
int doapply(void);
int holetime(void);
void dighole(void);

/* hack.bones.c */
void savebones(void);
int getbones(void);

/* hack.c */
void unsee(void);
void seeoff(int);
void domove(void);
int dopickup(void);
void pickup(int);
void lookaround(void);
int monster_nearby(void);
int rroom(int, int);
int cansee(xchar, xchar);
int sgn(int);
void setsee(void);
void nomul(int);
int abon(void);
int dbon(void);
void losestr(int);
void losehp(int, const char *);
void losehp_m(int, struct monst *);
void losexp(void);
int inv_weight(void);
long newuexp(void);

/* hack.cmd.c */
void rhack(const char *);
int movecmd(int);
int getdir(boolean);
void confdir(void);
int finddir(void);
int isroom(int, int);
int isok(int, int);

/* hack.do.c */
int dodrop(void);
void dropx(struct obj *);
int doddrop(void);
int dodown(void);
int doup(void);
void goto_level(int, boolean);
int donull(void);
int dopray(void);
int dothrow(void);
struct obj *splitobj(struct obj *, int);
void more_experienced(int, int);
void set_wounded_legs(long, int);
void heal_legs(void);

/* hack.do_name.c */
coord getpos(int, const char *);
int do_mname(void);
int ddocall(void);
void docall(struct obj *);
char *monnam(struct monst *);
char *Monnam(struct monst *);
char *amonnam(struct monst *, const char *);
char *Amonnam(struct monst *, const char *);
char *Xmonnam(struct monst *);

/* hack.do_wear.c */
int doremarm(void);
int doremring(void);
int armoroff(struct obj *);
int doweararm(void);
int dowearring(void);
void ringoff(struct obj *);
void find_ac(void);
void glibr(void);
struct obj *some_armor(void);
void corrode_armor(void);

/* hack.dog.c */
void makedog(void);
void losedogs(void);
void keepdogs(void);
void fall_down(struct monst *);
int dog_move(struct monst *, int);
int inroom(xchar, xchar);
int tamedog(struct monst *, struct obj *);

/* hack.eat.c */
void init_uhunger(void);
int doeat(void);
void gethungry(void);
void morehungry(int);
void lesshungry(int);
int poisonous(struct obj *);

/* hack.end.c */
int dodone(void);
void done1(int);
void done_in_by(struct monst *);
void done(const char *);
void clearlocks(void);
void hang_up(int) __dead;
char *eos(char *);
void charcat(char *, int);
void prscore(int, char **);

/* hack.engrave.c */
int sengr_at(const char *, xchar, xchar);
void u_wipe_engr(int);
void wipe_engr_at(xchar, xchar, xchar);
void read_engr_at(int, int);
void make_engr_at(int, int, const char *);
int doengrave(void);
void save_engravings(int);
void rest_engravings(int);

/* hack.fight.c */
int hitmm(struct monst *, struct monst *);
void mondied(struct monst *);
int fightm(struct monst *);
int thitu(int, int, const char *);
boolean hmon(struct monst *, struct obj *, int);
int attack(struct monst *);

/* hack.invent.c */
struct obj *addinv(struct obj *);
void useup(struct obj *);
void freeinv(struct obj *);
void delobj(struct obj *);
void freeobj(struct obj *);
void freegold(struct gold *);
void deltrap(struct trap *);
struct monst *m_at(int, int);
struct obj *o_at(int, int);
struct obj *sobj_at(int, int, int);
int carried(struct obj *);
int carrying(int);
struct obj *o_on(unsigned int, struct obj *);
struct trap *t_at(int, int);
struct gold *g_at(int, int);
struct obj *getobj(const char *, const char *);
int ggetobj(const char *, int (*fn)(struct obj *), int);
int askchain(struct obj *, char *, int, int (*)(struct obj *), 
    int (*)(struct obj *), int);
void prinv(struct obj *);
int ddoinv(void);
int dotypeinv(void);
int dolook(void);
void stackobj(struct obj *);
int doprgold(void);
int doprwep(void);
int doprarm(void);
int doprring(void);
int digit(int);

/* hack.ioctl.c */
void getioctls(void);
void setioctls(void);
int dosuspend(void);

/* hack.lev.c */
void savelev(int, xchar);
void bwrite(int, const void *, size_t);
void saveobjchn(int, struct obj *);
void savemonchn(int, struct monst *);
void getlev(int, int, xchar);
void mread(int, void *, size_t);
void mklev(void);

/* hack.main.c */
void glo(int);
void askname(void);
void impossible(const char *, ...) __printflike(1, 2);
void stop_occupation(void);

/* hack.makemon.c */
struct monst *makemon(const struct permonst *, int, int);
coord enexto(xchar, xchar);
int goodpos(int, int);
void rloc(struct monst *);
struct monst *mkmon_at(int, int, int);

/* hack.mhitu.c */
int mhitu(struct monst *);
int hitu(struct monst *, int);

/* hack.mklev.c */
struct mkroom;
void makelevel(void);
void mktrap(int, int, struct mkroom *);

/* hack.mkmaze.c */
void makemaz(void);
coord mazexy(void);

/* hack.mkobj.c */
struct obj *mkobj_at(int, int, int);
void mksobj_at(int, int, int);
struct obj *mkobj(int);
struct obj *mksobj(int);
int letter(int);
int weight(struct obj *);
void mkgold(long, int, int);

/* hack.mkshop.c */
void mkshop(void);
void mkzoo(int);
void mkswamp(void);

/* hack.mon.c */
void movemon(void);
void justswld(struct monst *, const char *);
void youswld(struct monst *, int, unsigned int, const char *);
int dochug(struct monst *);
int m_move(struct monst *, int);
int mfndpos(struct monst *, coord[9 ], int[9 ], int);
int dist(int, int);
void poisoned(const char *, const char *);
void mondead(struct monst *);
void replmon(struct monst *, struct monst *);
void relmon(struct monst *);
void monfree(struct monst *);
void unstuck(struct monst *);
void killed(struct monst *);
void kludge(const char *, const char *);
void rescham(void);
int newcham(struct monst *, const struct permonst *);
void mnexto(struct monst *);
void setmangry(struct monst *);
int canseemon(struct monst *);

/* hack.monst.c */

/* hack.o_init.c */
int letindex(int);
void init_objects(void);
int probtype(int);
void oinit(void);
void savenames(int);
void restnames(int);
int dodiscovered(void);

/* hack.objnam.c */
char *typename(int);
char *xname(struct obj *);
char *doname(struct obj *);
void setan(const char *, char *, size_t);
char *aobjnam(struct obj *, const char *);
char *Doname(struct obj *);
struct obj *readobjnam(char *);

/* hack.options.c */
void initoptions(void);
int doset(void);

/* hack.pager.c */
int dowhatis(void);
void set_whole_screen(void);
int readnews(void);
void set_pager(int);
int page_line(const char *);
void cornline(int, const char *);
int dohelp(void);
int dosh(void);

/* hack.potion.c */
int dodrink(void);
void pluslvl(void);
void strange_feeling(struct obj *, const char *);
void potionhit(struct monst *, struct obj *);
void potionbreathe(struct obj *);
int dodip(void);

/* hack.pri.c */
void swallowed(void);
void panic(const char *, ...) __printflike(1, 2);
void atl(int, int, int);
void on_scr(int, int);
void tmp_at(schar, schar);
void Tmp_at(schar, schar);
void setclipped(void) __dead;
void at(xchar, xchar, int);
void prme(void);
int doredraw(void);
void docrt(void);
void docorner(int, int);
void curs_on_u(void);
void pru(void);
void prl(int, int);
char news0(xchar, xchar);
void newsym(int, int);
void mnewsym(int, int);
void nosee(int, int);
void prl1(int, int);
void nose1(int, int);
int vism_at(int, int);
void pobj(struct obj *);
void unpobj(struct obj *);
void seeobjs(void);
void seemons(void);
void pmon(struct monst *);
void unpmon(struct monst *);
void nscr(void);
void bot(void);
void mstatusline(struct monst *);
void cls(void);

/* hack.read.c */
int doread(void);
void litroom(boolean);

/* hack.rip.c */
void outrip(void);

/* hack.rumors.c */
void outrumor(void);

/* hack.save.c */
int dosave(void);
int dorecover(int);
struct obj *restobjchn(int);
struct monst *restmonchn(int);

/* hack.search.c */
int findit(void);
int dosearch(void);
int doidtrap(void);
void wakeup(struct monst *);
void seemimic(struct monst *);

/* hack.shk.c */
void obfree(struct obj *, struct obj *);
void paybill(void);
char *shkname(struct monst *);
void shkdead(struct monst *);
void replshk(struct monst *, struct monst *);
int inshop(void);
int dopay(void);
struct bill_x;
void addtobill(struct obj *);
void splitbill(struct obj *, struct obj *);
void subfrombill(struct obj *);
int doinvbill(int);
int shkcatch(struct obj *);
int shk_move(struct monst *);
void shopdig(int);
int online(int, int);
int follower(struct monst *);

/* hack.shknam.c */
void findname(char *, int);

/* hack.steal.c */
long somegold(void);
void stealgold(struct monst *);
int steal(struct monst *);
void mpickobj(struct monst *, struct obj *);
int stealamulet(struct monst *);
void relobj(struct monst *, int);

/* hack.termcap.c */
void startup(void);
void startscreen(void);
void endscreen(void);
void curs(int, int);
void cl_end(void);
void clearscreen(void);
void home(void);
void standoutbeg(void);
void standoutend(void);
void backsp(void);
void sound_bell(void);
void delay_output(void);
void cl_eos(void);

/* hack.timeout.c */
void timeout(void);

/* hack.topl.c */
int doredotopl(void);
void remember_topl(void);
void addtopl(const char *);
void more(void);
void cmore(const char *);
void clrlin(void);
void pline(const char *, ...) __printflike(1, 2);
void vpline(const char *, va_list) __printflike(1, 0);
void putsym(int);
void putstr(const char *);

/* hack.track.c */
void initrack(void);
void settrack(void);
coord *gettrack(int, int);

/* hack.trap.c */
struct trap *maketrap(int, int, int);
void dotrap(struct trap *);
int mintrap(struct monst *);
void selftouch(const char *);
void float_up(void);
void float_down(void);
void tele(void);
int dotele(void);
void placebc(int);
void unplacebc(void);
void level_tele(void);
void drown(void);

/* hack.tty.c */
void gettty(void);
void settty(const char *);
void setftty(void);
void error(const char *, ...) __printflike(1, 2) __dead;
void getlin(char *);
void getret(void);
void cgetret(const char *);
void xwaitforspace(const char *);
char *parse(void);
char readchar(void);
void end_of_input(void) __dead;

/* hack.u_init.c */
void u_init(void);
struct trobj;
void plnamesuffix(void);

/* hack.unix.c */
void setrandom(void);
int getyear(void);
char *getdatestr(void);
int phase_of_the_moon(void);
int night(void);
int midnight(void);
void gethdate(char *);
int uptodate(int);
void getlock(void);
void getmailstatus(void);
void ckmailstatus(void);
void newmail(void);
void mdrush(struct monst *, boolean);
void readmail(void);
void regularize(char *);

/* hack.vault.c */
void setgd(void);
int gd_move(void);
void gddead(void);
void replgd(struct monst *, struct monst *);
void invault(void);

/* hack.version.c */
int doversion(void);

/* hack.wield.c */
void setuwep(struct obj *);
int dowield(void);
void corrode_weapon(void);
int chwepon(struct obj *, int);

/* hack.wizard.c */
void amulet(void);
int wiz_hit(struct monst *);
void inrange(struct monst *);

/* hack.worm.c */
#ifndef NOWORM
int getwn(struct monst *);
void initworm(struct monst *);
void worm_move(struct monst *);
void worm_nomove(struct monst *);
void wormdead(struct monst *);
void wormhit(struct monst *);
void wormsee(unsigned);
struct wseg;
void pwseg(struct wseg *);
void cutworm(struct monst *, xchar, xchar, uchar);
#endif

/* hack.worn.c */
void setworn(struct obj *, long);
void setnotworn(struct obj *);

/* hack.zap.c */
int dozap(void);
const char *exclam(int);
void hit(const char *, struct monst *, const char *);
void miss(const char *, struct monst *);
struct monst *bhit(int, int, int, int,
    void (*)(struct monst *, struct obj *),
    int (*)(struct obj *, struct obj *),
    struct obj *);
struct monst *boomhit(int, int);
void buzz(int, xchar, xchar, int, int);
void fracture_rock(struct obj *);

/* rnd.c */
int rn1(int, int);
int rn2(int);
int rnd(int);
int d(int, int);
#endif /* _EXTERN_H_ */
