typedef union {  char *str;
          int number;
       } YYSTYPE;
#define	STR	258
#define	GSTR	259
#define	VAR	260
#define	NUMBER	261
#define	WINDOWTITLE	262
#define	WINDOWSIZE	263
#define	WINDOWPOSITION	264
#define	FONT	265
#define	FORECOLOR	266
#define	BACKCOLOR	267
#define	SHADCOLOR	268
#define	LICOLOR	269
#define	COLORSET	270
#define	OBJECT	271
#define	INIT	272
#define	PERIODICTASK	273
#define	MAIN	274
#define	END	275
#define	PROP	276
#define	TYPE	277
#define	SIZE	278
#define	POSITION	279
#define	VALUE	280
#define	VALUEMIN	281
#define	VALUEMAX	282
#define	TITLE	283
#define	SWALLOWEXEC	284
#define	ICON	285
#define	FLAGS	286
#define	WARP	287
#define	WRITETOFILE	288
#define	HIDDEN	289
#define	CANBESELECTED	290
#define	NORELIEFSTRING	291
#define	CASE	292
#define	SINGLECLIC	293
#define	DOUBLECLIC	294
#define	BEG	295
#define	POINT	296
#define	EXEC	297
#define	HIDE	298
#define	SHOW	299
#define	CHFORECOLOR	300
#define	CHBACKCOLOR	301
#define	CHCOLORSET	302
#define	GETVALUE	303
#define	CHVALUE	304
#define	CHVALUEMAX	305
#define	CHVALUEMIN	306
#define	ADD	307
#define	DIV	308
#define	MULT	309
#define	GETTITLE	310
#define	GETOUTPUT	311
#define	STRCOPY	312
#define	NUMTOHEX	313
#define	HEXTONUM	314
#define	QUIT	315
#define	LAUNCHSCRIPT	316
#define	GETSCRIPTFATHER	317
#define	SENDTOSCRIPT	318
#define	RECEIVFROMSCRIPT	319
#define	GET	320
#define	SET	321
#define	SENDSIGN	322
#define	REMAINDEROFDIV	323
#define	GETTIME	324
#define	GETSCRIPTARG	325
#define	IF	326
#define	THEN	327
#define	ELSE	328
#define	FOR	329
#define	TO	330
#define	DO	331
#define	WHILE	332
#define	BEGF	333
#define	ENDF	334
#define	EQUAL	335
#define	INFEQ	336
#define	SUPEQ	337
#define	INF	338
#define	SUP	339
#define	DIFF	340


extern YYSTYPE yylval;
