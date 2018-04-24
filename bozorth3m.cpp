#include "bozorth3m.h"

// ************************************************************* //
// ************************************************************* //
// *************                                  ************** //
// *************       Bozorth3 core library      ************** //
// *************                                  ************** //
// ************************************************************* //
// ************************************************************* //

Bozorth3_Core::Bozorth3_Core()
{
    this->max_minutiae = 150;
    this->stack_pointer = this->stack;
}

Bozorth3_Core::~Bozorth3_Core()
{
    // destructor
}


// other functions
int sort_x_y( const void * a, const void * b )
{
    struct minutiae_struct * af;
    struct minutiae_struct * bf;

    af = (struct minutiae_struct *) a;
    bf = (struct minutiae_struct *) b;

    if ( af->col[0] < bf->col[0] )
        return -1;
    if ( af->col[0] > bf->col[0] )
        return 1;

    if ( af->col[1] < bf->col[1] )
        return -1;
    if ( af->col[1] > bf->col[1] )
        return 1;

    return 0;
}

int sort_quality_decreasing( const void * a, const void * b )
{
    struct minutiae_struct * af;
    struct minutiae_struct * bf;

    af = (struct minutiae_struct *) a;
    bf = (struct minutiae_struct *) b;

    if ( af->col[3] > bf->col[3] )
        return -1;
    if ( af->col[3] < bf->col[3] )
        return 1;
    return 0;
}



/************************************************************************
Load a 3-4 column (X,Y,T[,Q]) set of minutiae from the specified file
and return a XYT sturcture.
Row 3's value is an angle which is normalized to the interval (-180,180].
A maximum of MAX_BOZORTH_MINUTIAE minutiae can be returned -- fewer if
"max_minutiae" is smaller.  If the file contains more minutiae than are
to be returned, the highest-quality minutiae are returned.
*************************************************************************/

/***********************************************************************/
struct xyt_struct * Bozorth3_Core::bz_load(QVector<MINUTIA> minutiae, bool useQuality)
{
    int nminutiae;
    int i;
    int nargs_expected;
    //FILE * fp;
    struct xyt_struct * xyt_s;
    struct xytq_struct * xytq_s;
    int xvals_lng[MAX_FILE_MINUTIAE];   /* Temporary lists to store all the minutaie from a file */
    int yvals_lng[MAX_FILE_MINUTIAE];
    int tvals_lng[MAX_FILE_MINUTIAE];
    int qvals_lng[MAX_FILE_MINUTIAE];

    nminutiae = 0;
    nargs_expected = 0;

    if (minutiae.size() == 0) return XYT_NULL;

    int angle;
    for (MINUTIA minutia : minutiae) {

        //???
        angle = (int)(minutia.angle * 180 / M_PI);
        angle = 360 - angle;

        xvals_lng[nminutiae] = minutia.xy.x();
        yvals_lng[nminutiae] = minutia.imgWH.y()-minutia.xy.y();
        tvals_lng[nminutiae] = angle;
        if (useQuality) qvals_lng[nminutiae] = minutia.quality;
        else qvals_lng[nminutiae] = 1;

        ++nminutiae;
        if ( nminutiae == MAX_FILE_MINUTIAE ) break;
    }

    xytq_s = (struct xytq_struct *)malloc(sizeof(struct xytq_struct));
    if ( xytq_s == XYTQ_NULL )
    {
        //fprintf( stderr, "ERROR: malloc() failure while loading minutiae buffer failed: %s\n",
        //         strerror(errno));
        return XYT_NULL;
    }

    xytq_s->nrows = nminutiae;
    for (i=0; i<nminutiae; i++)
    {
        xytq_s->xcol[i] = xvals_lng[i];
        xytq_s->ycol[i] = yvals_lng[i];
        xytq_s->thetacol[i] = tvals_lng[i];
        xytq_s->qualitycol[i] = qvals_lng[i];
    }

    xyt_s = bz_prune(xytq_s, 0);
    free(xytq_s);

    //fprintf( stdout, "Loaded %s\n", xyt_file );

    return xyt_s;
}

/************************************************************************
Load a XYTQ structure and return a XYT struct.
Row 3's value is an angle which is normalized to the interval (-180,180].
A maximum of MAX_BOZORTH_MINUTIAE minutiae can be returned -- fewer if
"max_minutiae" is smaller.  If the file contains more minutiae than are
to be returned, the highest-quality minutiae are returned.
*************************************************************************/
struct xyt_struct * Bozorth3_Core::bz_prune(struct xytq_struct *xytq_s, int verbose_load)
{
    int nminutiae;
    int j;
    struct xyt_struct * xyt_s;
    int * xptr;
    int * yptr;
    int * tptr;
    int * qptr;
    struct minutiae_struct c[MAX_FILE_MINUTIAE];
    int xvals_lng[MAX_FILE_MINUTIAE],
            yvals_lng[MAX_FILE_MINUTIAE],
            tvals_lng[MAX_FILE_MINUTIAE],
            qvals_lng[MAX_FILE_MINUTIAE];
    int order[MAX_FILE_MINUTIAE];
    int xvals[MAX_BOZORTH_MINUTIAE],
            yvals[MAX_BOZORTH_MINUTIAE],
            tvals[MAX_BOZORTH_MINUTIAE],
            qvals[MAX_BOZORTH_MINUTIAE];

#define C1 0
#define C2 1

    int i;
    nminutiae = xytq_s->nrows;
    for (i=0; i<nminutiae; i++)
    {
        xvals_lng[i] = xytq_s->xcol[i];
        yvals_lng[i] = xytq_s->ycol[i];

        if ( xytq_s->thetacol[i] > 180 )
            tvals_lng[i] = xytq_s->thetacol[i] - 360;
        else
            tvals_lng[i] = xytq_s->thetacol[i];

        qvals_lng[i] = xytq_s->qualitycol[i];
    }

    if ( nminutiae > max_minutiae )
    {
       // if ( verbose_load )
            //fprintf( stderr, "WARNING: bz_prune(): trimming minutiae to the %d of highest quality\n",
        //             max_minutiae );

        //fprintf( stderr, "Before quality sort:\n" );
        if ( sort_order_decreasing( qvals_lng, nminutiae, order ))
        {
            //fprintf( stderr, "ERROR: sort failed and returned on error\n");
            return XYT_NULL;
        }

        for ( j = 0; j < nminutiae; j++ )
        {
        //    if ( verbose_load )
                //fprintf( stderr, "   %3d: %3d %3d %3d ---> order = %3d\n",
        //                 j, xvals_lng[j], yvals_lng[j], qvals_lng[j], order[j] );

            if ( j == 0 )
                continue;
            if ( qvals_lng[order[j]] > qvals_lng[order[j-1]] ) {
                //fprintf( stderr, "ERROR: sort failed: j=%d; qvals_lng[%d] > qvals_lng[%d]\n",
                     //    j, order[j], order[j-1] );
                return XYT_NULL;
            }
        }

        //fprintf( stderr, "\nAfter quality sort:\n" );
        for ( j = 0; j < max_minutiae; j++ )
        {
            xvals[j] = xvals_lng[order[j]];
            yvals[j] = yvals_lng[order[j]];
            tvals[j] = tvals_lng[order[j]];
            qvals[j] = qvals_lng[order[j]];

            //fprintf( stderr, "   %3d: %3d %3d %3d\n", j, xvals[j], yvals[j], qvals[j] );
        }


        if ( C1 )
        {

            //fprintf( stderr, "\nAfter qsort():\n" );
            qsort( (void *) &c, (size_t) nminutiae, sizeof(struct minutiae_struct), sort_quality_decreasing );
            for ( j = 0; j < nminutiae; j++ )
            {
             //   if ( verbose_load )
                    //fprintf( stderr, "Q  %3d: %3d %3d %3d\n",
              //               j, c[j].col[0], c[j].col[1], c[j].col[3] );

                if ( j > 0 && c[j].col[3] > c[j-1].col[3] )
                {
                    //fprintf( stderr, "ERROR: sort failed: c[%d].col[3] > c[%d].col[3]\n",
                           //  j, j-1 );
                    return XYT_NULL;
                }
            }
        }

      //  if ( verbose_load )
            //fprintf( stderr, "\n" );

        xptr = xvals;
        yptr = yvals;
        tptr = tvals;
        qptr = qvals;

        nminutiae = max_minutiae;
    }
    else
    {
        xptr = xvals_lng;
        yptr = yvals_lng;
        tptr = tvals_lng;
        qptr = qvals_lng;
    }


    for ( j=0; j < nminutiae; j++ )
    {
        c[j].col[0] = xptr[j];
        c[j].col[1] = yptr[j];
        c[j].col[2] = tptr[j];
        c[j].col[3] = qptr[j];
    }
    qsort( (void *) &c, (size_t) nminutiae, sizeof(struct minutiae_struct), sort_x_y );

    if ( verbose_load ) {
        //fprintf( stderr, "\nSorted on increasing x, then increasing y\n" );
        for ( j = 0; j < nminutiae; j++ )
        {
            //fprintf( stderr, "%d : %3d, %3d, %3d, %3d\n", j, c[j].col[0], c[j].col[1], c[j].col[2], c[j].col[3] );
            if ( j > 0 )
            {
                if ( c[j].col[0] < c[j-1].col[0] )
                {
                    //fprintf( stderr, "ERROR: sort failed: c[%d].col[0]=%d > c[%d].col[0]=%d\n",

                     //        j, c[j].col[0], j-1, c[j-1].col[0]
                     //       );
                    return XYT_NULL;
                }
                if ( c[j].col[0] == c[j-1].col[0] && c[j].col[1] < c[j-1].col[1] )
                {
                    //fprintf( stderr, "ERROR: sort failed: c[%d].col[0]=%d == c[%d].col[0]=%d; c[%d].col[0]=%d == c[%d].col[0]=%d\n",

                    //         j, c[j].col[0], j-1, c[j-1].col[0],
                     //       j, c[j].col[1], j-1, c[j-1].col[1]
                    //        );
                    return XYT_NULL;
                }
            }
        }
    }

    xyt_s = (struct xyt_struct *) malloc( sizeof( struct xyt_struct ) );
    if ( xyt_s == XYT_NULL )
    {
        //fprintf( stderr, "ERROR: malloc() failure of xyt_struct.");
        return XYT_NULL;
    }

    for ( j = 0; j < nminutiae; j++ )
    {
        xyt_s->xcol[j]     = c[j].col[0];
        xyt_s->ycol[j]     = c[j].col[1];
        xyt_s->thetacol[j] = c[j].col[2];
    }
    xyt_s->nrows = nminutiae;

    return xyt_s;
}


/* return values: 0 == successful, 1 == error */
int Bozorth3_Core::sort_order_decreasing(
        int values[],		/* INPUT:  the unsorted values themselves */
        int num,		/* INPUT:  the number of values */
        int order[]		/* OUTPUT: the order for each of the values if sorted */
        )
{
    int i;
    struct cell * cells;


    cells = (struct cell *) malloc( num * sizeof(struct cell) );
    if ( cells == (struct cell *) NULL ){
        //fprintf( stderr, "ERROR: malloc(): struct cell\n");
        return 1;
    }

    for( i = 0; i < num; i++ ) {
        cells[i].index = values[i];
        cells[i].item  = i;
    }

    if ( qsort_decreasing( cells, 0, num-1 ) < 0)
        return 2;

    for( i = 0; i < num; i++ ) {
        order[i] = cells[i].item;
    }

    free( (void *) cells );

    return 0;
}



/***********************************************************************/
/********************************************************
qsort_decreasing()
This procedure inputs a pointer to an index_struct, the subscript of an index array to be
sorted, a left subscript pointing to where the  sort is to begin in the index array, and a right
subscript where to end. This module invokes a  decreasing quick-sort sorting the index array  from l to r.
********************************************************/
/* return values: 0 == successful, 1 == error */

int Bozorth3_Core::qsort_decreasing( struct cell v[], int left, int right )
{
    int pivot;
    int llen, rlen;
    int lleft, lright, rleft, rright;


    if ( pushstack( left  ))
        return 1;
    if ( pushstack( right ))
        return 2;
    while ( stack_pointer != stack ) {
        if (popstack(&right))
            return 3;
        if (popstack(&left ))
            return 4;
        if ( right - left > 0 ) {
            pivot = select_pivot( v, left, right );
            partition_dec( v, &llen, &rlen, &lleft, &lright, &rleft, &rright, pivot, left, right );
            if ( llen > rlen ) {
                if ( pushstack( lleft  ))
                    return 5;
                if ( pushstack( lright ))
                    return 6;
                if ( pushstack( rleft  ))
                    return 7;
                if ( pushstack( rright ))
                    return 8;
            } else{
                if ( pushstack( rleft  ))
                    return 9;
                if ( pushstack( rright ))
                    return 10;
                if ( pushstack( lleft  ))
                    return 11;
                if ( pushstack( lright ))
                    return 12;
            }
        }
    }
    return 0;
}



/***********************************************************************/
/* return values: 0 == successful, 1 == error */
int Bozorth3_Core::pushstack( int position )
{
    *stack_pointer++ = position;
    if ( stack_pointer > ( stack + BZ_STACKSIZE ) ) {
        //fprintf( stderr, "ERROR: pushstack(): stack overflow\n");
        return 1;
    }
    return 0;
}


int Bozorth3_Core::popstack( int *popval )
{
    if ( --stack_pointer < stack ) {
        //fprintf( stderr, "ERROR: popstack(): stack underflow\n" );
        return 1;
    }
    *popval = *stack_pointer;
    return 0;
}





/***********************************************************************/
/********************************************************
partition_dec()
Inputs a pivot element making comparisons and swaps with other elements in a list,
until pivot resides at its correct position in the list.
********************************************************/
void Bozorth3_Core::partition_dec( struct cell v[], int *llen, int *rlen, int *ll, int *lr, int *rl, int *rr, int p, int l, int r )
{
#define iswap(a,b) { int itmp = (a); a = (b); b = itmp; }

    *ll = l;
    *rr = r;
    while ( 1 ) {
        if ( l < p ) {
            if ( v[l].index < v[p].index ) {
                iswap( v[l].index, v[p].index )
                        iswap( v[l].item,  v[p].item )
                        p = l;
            } else {
                l++;
            }
        } else {
            if ( r > p ) {
                if ( v[r].index > v[p].index ) {
                    iswap( v[r].index, v[p].index )
                            iswap( v[r].item,  v[p].item )
                            p = r;
                    l++;
                } else {
                    r--;
                }
            } else {
                *lr = p - 1;
                *rl = p + 1;
                *llen = *lr - *ll + 1;
                *rlen = *rr - *rl + 1;
                break;
            }
        }
    }
}




/***********************************************************************/
/*******************************************************************
select_pivot()
selects a pivot from a list being sorted using the Singleton Method.
*******************************************************************/
int Bozorth3_Core::select_pivot( struct cell v[], int left, int right )
{
    int midpoint;


    midpoint = ( left + right ) / 2;
    if ( v[left].index <= v[midpoint].index ) {
        if ( v[midpoint].index <= v[right].index ) {
            return midpoint;
        } else {
            if ( v[right].index > v[left].index ) {
                return right;
            } else {
                return left;
            }
        }
    } else {
        if ( v[left].index < v[right].index ) {
            return left;
        } else {
            if ( v[right].index < v[midpoint].index ) {
                return midpoint;
            } else {
                return right;
            }
        }
    }
}


/**************************************************************************/
int Bozorth3_Core::bz_match_score(
        int np,
        struct xyt_struct * pstruct,
        struct xyt_struct * gstruct
        )
{
    int kx, kq;
    int ftt;
    int tot;
    int qh;
    int tp;
    int ll, jj, kk, n, t, b;
    int k, i, j, ii, z;
    int kz, l;
    int p1, p2;
    int dw, ww;
    int match_score;
    int qq_overflow = 0;
    float fi;

    /* These next 3 arrays originally declared global, but moved here */
    /* locally because they are only used herein */
    int rr[ RR_SIZE ];
    int avn[ AVN_SIZE ];
    int avv[ AVV_SIZE_1 ][ AVV_SIZE_2 ];

    /* These now externally defined in bozorth.h */
    /* extern FILE * errorfp; */
    /* extern char * get_progname( void ); */
    /* extern char * get_probe_filename( void ); */
    /* extern char * get_gallery_filename( void ); */






    if ( pstruct->nrows < min_computable_minutiae ) {
#ifndef NOVERBOSE
        if ( gstruct->nrows < min_computable_minutiae ) {

            //fprintf( stderr, "bz_match_score(): both probe and gallery file have too few minutiae (%d,%d) to compute a real Bozorth match score; min. is %d\n",

           //          pstruct->nrows, gstruct->nrows, min_computable_minutiae
              //       );
        } else {

            //fprintf( stderr, "bz_match_score(): probe file has too few minutiae (%d) to compute a real Bozorth match score; min. is %d\n",

              //       pstruct->nrows, min_computable_minutiae
              //       );
        }
#endif
        return ZERO_MATCH_SCORE;
    }



    if ( gstruct->nrows < min_computable_minutiae ) {
#ifndef NOVERBOSE

        //fprintf( stderr, "bz_match_score(): gallery file has too few minutiae (%d) to compute a real Bozorth match score; min. is %d\n",

             //    gstruct->nrows, min_computable_minutiae
              //   );
#endif
        return ZERO_MATCH_SCORE;
    }


    /* initialize tables to 0's */

//    int set_count = YL_SIZE_1 * YL_SIZE_2;
//    for(int indd =0; indd<YL_SIZE_1;indd++){
//        for(int indd2 =0; indd2<YL_SIZE_2;indd2++){
//            yl[indd][indd2] = 0;
//        }
//    }
    INT_SET( (int *) &yl, YL_SIZE_1 * YL_SIZE_2, 0 );



//    for(int indd=0; indd<SC_SIZE; indd++){
//        sc[indd] = 0;
//    }
     INT_SET( (int *) &sc, SC_SIZE, 0 );


//    for(int indd=0; indd<CP_SIZE; indd++){
//        cp[indd] = 0;
//    }
     INT_SET( (int *) &cp, CP_SIZE, 0 );


//    for(int indd=0; indd<RP_SIZE; indd++){
//        rp[indd] = 0;
//    }
     INT_SET( (int *) &rp, RP_SIZE, 0 );


//    for(int indd=0; indd<TQ_SIZE; indd++){
//        tq[indd] = 0;
//    }
     INT_SET( (int *) &tq, TQ_SIZE, 0 );


//    for(int indd=0; indd<RQ_SIZE; indd++){
//        rq[indd] = 0;
//    }
    INT_SET( (int *) &rq, RQ_SIZE, 0 );


//    for(int indd=0; indd<ZZ_SIZE; indd++){
//        zz[indd] = 1000;
//    }
    INT_SET( (int *) &zz, ZZ_SIZE, 1000 );				/* zz[] initialized to 1000's */


//    for(int indd=0; indd<AVN_SIZE; indd++){
//        avn[indd] = 0;
//    }
    INT_SET( (int *) &avn, AVN_SIZE, 0 );				/* avn[0...4] <== 0; */





    tp  = 0;
    p1  = 0;
    tot = 0;
    ftt = 0;
    kx  = 0;
    match_score = 0;

    for ( k = 0; k < np - 1; k++ ) {
        /* printf( "compute(): looping with k=%d\n", k ); */

        if ( sc[k] )			/* If SC counter for current pair already incremented ... */
            continue;		/*		Skip to next pair */


        i = colp[k][1];
        t = colp[k][3];




        qq[0]   = i;
        rq[t-1] = i;
        tq[i-1] = t;


        ww = 0;
        dw = 0;

        do {
            ftt++;
            tot = 0;
            qh  = 1;
            kx  = k;




            do {









                kz = colp[kx][2];
                l  = colp[kx][4];
                kx++;
                bz_sift( &ww, kz, &qh, l, kx, ftt, &tot, &qq_overflow );
                if ( qq_overflow ) {
                    //fprintf( stderr, "WARNING: bz_match_score(): qq[] overflow from bz_sift() #1\n"
                          //   );
                    return QQ_OVERFLOW_SCORE;
                }

#ifndef NOVERBOSE

               // printf( "x1 %d %d %d %d %d %d\n", kx, colp[kx][0], colp[kx][1], colp[kx][2], colp[kx][3], colp[kx][4] );
#endif

            } while ( colp[kx][3] == colp[k][3] && colp[kx][1] == colp[k][1] );
            /* While the startpoints of lookahead edge pairs are the same as the starting points of the */
            /* current pair, set KQ to lookahead edge pair index where above bz_sift() loop left off */

            kq = kx;

            for ( j = 1; j < qh; j++ ) {
                for ( i = kq; i < np; i++ ) {

                    for ( z = 1; z < 3; z++ ) {
                        if ( z == 1 ) {
                            if ( (j+1) > QQ_SIZE ) {
                                //fprintf( stderr, "WARNING: bz_match_score(): qq[] overflow #1 in bozorth3(); j-1 is %d \n",
                                       //  j-1);
                                return QQ_OVERFLOW_SCORE;
                            }
                            p1 = qq[j];
                        } else {
                            p1 = tq[p1-1];

                        }






                        if ( colp[i][2*z] != p1 )
                            break;
                    }


                    if ( z == 3 ) {
                        z = colp[i][1];
                        l = colp[i][3];



                        if ( z != colp[k][1] && l != colp[k][3] ) {
                            kx = i + 1;
                            bz_sift( &ww, z, &qh, l, kx, ftt, &tot, &qq_overflow );
                            if ( qq_overflow ) {
                                //fprintf( stderr, "WARNING: bz_match_score(): qq[] overflow from bz_sift() #2\n"
                                      //   );
                                return QQ_OVERFLOW_SCORE;
                            }
                        }
                    }
                } /* END for i */



                /* Done looking ahead for current j */





                l = 1;
                t = np + 1;
                b = kq;

                while ( t - b > 1 ) {
                    l = ( b + t ) / 2;

                    for ( i = 1; i < 3; i++ ) {

                        if ( i == 1 ) {
                            if ( (j+1) > QQ_SIZE ) {
                                //fprintf( stderr, "WARNING: bz_match_score(): qq[] overflow #2 in bozorth3(); j-1 is %d\n",
                                      //   j-1 );
                                return QQ_OVERFLOW_SCORE;
                            }
                            p1 = qq[j];
                        } else {
                            p1 = tq[p1-1];
                        }



                        p2 = colp[l-1][i*2-1];

                        n = SENSE(p1,p2);

                        if ( n < 0 ) {
                            t = l;
                            break;
                        }
                        if ( n > 0 ) {
                            b = l;
                            break;
                        }
                    }

                    if ( n == 0 ) {






                        /* Locates the head of consecutive sequence of edge pairs all having the same starting Subject and On-File edgepoints */
                        while ( colp[l-2][3] == p2 && colp[l-2][1] == colp[l-1][1] )
                            l--;

                        kx = l - 1;


                        do {
                            kz = colp[kx][2];
                            l  = colp[kx][4];
                            kx++;
                            bz_sift( &ww, kz, &qh, l, kx, ftt, &tot, &qq_overflow );
                            if ( qq_overflow ) {
                                //fprintf( stderr, "WARNING: bz_match_score(): qq[] overflow from bz_sift() #3\n"
                                       //  );
                                return QQ_OVERFLOW_SCORE;
                            }
                        } while ( colp[kx][3] == p2 && colp[kx][1] == colp[kx-1][1] );

                        break;
                    } /* END if ( n == 0 ) */

                } /* END while */

            } /* END for j */




            if ( tot >= MSTR ) {
                jj = 0;
                kk = 0;
                n  = 0;
                l  = 0;

                for ( i = 0; i < tot; i++ ) {


                    int colp_value = colp[ y[i]-1 ][0];
                    if ( colp_value < 0 ) {
                        kk += colp_value;
                        n++;
                    } else {
                        jj += colp_value;
                        l++;
                    }
                }


                if ( n == 0 ) {
                    n = 1;
                } else if ( l == 0 ) {
                    l = 1;
                }



                fi = (float) jj / (float) l - (float) kk / (float) n;

                if ( fi > 180.0F ) {
                    fi = ( jj + kk + n * 360 ) / (float) tot;
                    if ( fi > 180.0F )
                        fi -= 360.0F;
                } else {
                    fi = ( jj + kk ) / (float) tot;
                }

                jj = ROUND(fi);
                if ( jj <= -180 )
                    jj += 360;



                kk = 0;
                for ( i = 0; i < tot; i++ ) {
                    int diff = colp[ y[i]-1 ][0] - jj;
                    j = SQUARED( diff );




                    if ( j > TXS && j < CTXS )
                        kk++;
                    else
                        y[i-kk] = y[i];
                } /* END FOR i */

                tot -= kk;				/* Adjust the total edge pairs TOT based on # of edge pairs skipped */

            } /* END if ( tot >= MSTR ) */




            if ( tot < MSTR ) {




                for ( i = tot-1 ; i >= 0; i-- ) {
                    int idx = y[i] - 1;
                    if ( rk[idx] == 0 ) {
                        sc[idx] = -1;
                    } else {
                        sc[idx] = rk[idx];
                    }
                }
                ftt--;

            } else {		/* tot >= MSTR */
                /* Otherwise size of TOT group (seq. of TOT indices stored in Y) is large enough to analyze */

                int pa = 0;
                int pb = 0;
                int pc = 0;
                int pd = 0;

                for ( i = 0; i < tot; i++ ) {
                    int idx = y[i] - 1;
                    for ( ii = 1; ii < 4; ii++ ) {




                        kk = ( SQUARED(ii) - ii + 2 ) / 2 - 1;




                        jj = colp[idx][kk];

                        switch ( ii ) {
                        case 1:
                            if ( colp[idx][0] < 0 ) {
                                pd += colp[idx][0];
                                pb++;
                            } else {
                                pa += colp[idx][0];
                                pc++;
                            }
                            break;
                        case 2:
                            avn[ii-1] += pstruct->xcol[jj-1];
                            avn[ii] += pstruct->ycol[jj-1];
                            break;
                        default:
                            avn[ii] += gstruct->xcol[jj-1];
                            avn[ii+1] += gstruct->ycol[jj-1];
                            break;
                        } /* switch */
                    } /* END for ii = [1..3] */

                    for ( ii = 0; ii < 2; ii++ ) {
                        n = -1;
                        l = 1;

                        for ( jj = 1; jj < 3; jj++ ) {










                            p1 = colp[idx][ 2 * ii + jj ];


                            b = 0;
                            t = yl[ii][tp] + 1;

                            while ( t - b > 1 ) {
                                l  = ( b + t ) / 2;
                                p2 = yy[l-1][ii][tp];
                                n  = SENSE(p1,p2);

                                if ( n < 0 ) {
                                    t = l;
                                } else {
                                    if ( n > 0 ) {
                                        b = l;
                                    } else {
                                        break;
                                    }
                                }
                            } /* END WHILE */

                            if ( n != 0 ) {
                                if ( n == 1 )
                                    ++l;

                                for ( kk = yl[ii][tp]; kk >= l; --kk ) {
                                    yy[kk][ii][tp] = yy[kk-1][ii][tp];
                                }

                                ++yl[ii][tp];
                                yy[l-1][ii][tp] = p1;


                            } /* END if ( n != 0 ) */

                            /* Otherwise, edgepoint already stored in YY */

                        } /* END FOR jj in [1,2] */
                    } /* END FOR ii in [0,1] */
                } /* END FOR i */

                if ( pb == 0 ) {
                    pb = 1;
                } else if ( pc == 0 ) {
                    pc = 1;
                }



                fi = (float) pa / (float) pc - (float) pd / (float) pb;
                if ( fi > 180.0F ) {

                    fi = ( pa + pd + pb * 360 ) / (float) tot;
                    if ( fi > 180.0F )
                        fi -= 360.0F;
                } else {
                    fi = ( pa + pd ) / (float) tot;
                }

                pa = ROUND(fi);
                if ( pa <= -180 )
                    pa += 360;



                avv[tp][0] = pa;

                for ( ii = 1; ii < 5; ii++ ) {
                    avv[tp][ii] = avn[ii] / tot;
                    avn[ii] = 0;
                }

                ct[tp]  = tot;
                gct[tp] = tot;

                if ( tot > match_score )		/* If current TOT > match_score ... */
                    match_score = tot;		/*	Keep track of max TOT in match_score */

                ctt[tp]    = 0;		/* Init CTT[TP] to 0 */
                ctp[tp][0] = tp;	/* Store TP into CTP */

                for ( ii = 0; ii < tp; ii++ ) {
                    int found;
                    int diff;

                    int * avv_tp_ptr = &avv[tp][0];
                    int * avv_ii_ptr = &avv[ii][0];
                    diff = *avv_tp_ptr++ - *avv_ii_ptr++;
                    j = SQUARED( diff );






                    if ( j > TXS && j < CTXS )
                        continue;









                    ll = *avv_tp_ptr++ - *avv_ii_ptr++;
                    jj = *avv_tp_ptr++ - *avv_ii_ptr++;
                    kk = *avv_tp_ptr++ - *avv_ii_ptr++;
                    j  = *avv_tp_ptr++ - *avv_ii_ptr++;

                    {
                        float tt, ai, dz;

                        tt = (float) (SQUARED(ll) + SQUARED(jj));
                        ai = (float) (SQUARED(j)  + SQUARED(kk));

                        fi = ( 2.0F * TK ) * ( tt + ai );
                        dz = tt - ai;


                        if ( SQUARED(dz) > SQUARED(fi) )
                            continue;
                    }



                    if ( ll ) {

                        if ( m1_xyt )
                            fi = ( 180.0F / PI_SINGLE ) * atanf( (float) -jj / (float) ll );
                        else
                            fi = ( 180.0F / PI_SINGLE ) * atanf( (float) jj / (float) ll );
                        if ( fi < 0.0F ) {
                            if ( ll < 0 )
                                fi += 180.5F;
                            else
                                fi -= 0.5F;
                        } else {
                            if ( ll < 0 )
                                fi -= 180.5F;
                            else
                                fi += 0.5F;
                        }
                        jj = (int) fi;
                        if ( jj <= -180 )
                            jj += 360;
                    } else {

                        if ( m1_xyt ) {
                            if ( jj > 0 )
                                jj = -90;
                            else
                                jj = 90;
                        } else {
                            if ( jj > 0 )
                                jj = 90;
                            else
                                jj = -90;
                        }
                    }



                    if ( kk ) {

                        if ( m1_xyt )
                            fi = ( 180.0F / PI_SINGLE ) * atanf( (float) -j / (float) kk );
                        else
                            fi = ( 180.0F / PI_SINGLE ) * atanf( (float) j / (float) kk );
                        if ( fi < 0.0F ) {
                            if ( kk < 0 )
                                fi += 180.5F;
                            else
                                fi -= 0.5F;
                        } else {
                            if ( kk < 0 )
                                fi -= 180.5F;
                            else
                                fi += 0.5F;
                        }
                        j = (int) fi;
                        if ( j <= -180 )
                            j += 360;
                    } else {

                        if ( m1_xyt ) {
                            if ( j > 0 )
                                j = -90;
                            else
                                j = 90;
                        } else {
                            if ( j > 0 )
                                j = 90;
                            else
                                j = -90;
                        }
                    }





                    pa = 0;
                    pb = 0;
                    pc = 0;
                    pd = 0;

                    if ( avv[tp][0] < 0 ) {
                        pd += avv[tp][0];
                        pb++;
                    } else {
                        pa += avv[tp][0];
                        pc++;
                    }

                    if ( avv[ii][0] < 0 ) {
                        pd += avv[ii][0];
                        pb++;
                    } else {
                        pa += avv[ii][0];
                        pc++;
                    }

                    if ( pb == 0 ) {
                        pb = 1;
                    } else if ( pc == 0 ) {
                        pc = 1;
                    }



                    fi = (float) pa / (float) pc - (float) pd / (float) pb;

                    if ( fi > 180.0F ) {
                        fi = ( pa + pd + pb * 360 ) / 2.0F;
                        if ( fi > 180.0F )
                            fi -= 360.0F;
                    } else {
                        fi = ( pa + pd ) / 2.0F;
                    }

                    pb = ROUND(fi);
                    if ( pb <= -180 )
                        pb += 360;





                    pa = jj - j;
                    pa = IANGLE180(pa);
                    kk = SQUARED(pb-pa);




                    /* Was: if ( SQUARED(kk) > TXS && kk < CTXS ) : assume typo */
                    if ( kk > TXS && kk < CTXS )
                        continue;


                    found = 0;
                    for ( kk = 0; kk < 2; kk++ ) {
                        jj = 0;
                        ll = 0;

                        do {
                            while ( yy[jj][kk][ii] < yy[ll][kk][tp] && jj < yl[kk][ii] ) {

                                jj++;
                            }




                            while ( yy[jj][kk][ii] > yy[ll][kk][tp] && ll < yl[kk][tp] ) {

                                ll++;
                            }




                            if ( yy[jj][kk][ii] == yy[ll][kk][tp] && jj < yl[kk][ii] && ll < yl[kk][tp] ) {
                                found = 1;
                                break;
                            }


                        } while ( jj < yl[kk][ii] && ll < yl[kk][tp] );
                        if ( found )
                            break;
                    } /* END for kk */

                    if ( ! found ) {			/* If we didn't find what we were searching for ... */
                        gct[ii] += ct[tp];
                        if ( gct[ii] > match_score )
                            match_score = gct[ii];
                        ++ctt[ii];
                        ctp[ii][ctt[ii]] = tp;
                    }

                } /* END for ii in [0,TP-1] prior TP group */

                tp++;			/* Bump TP counter */


            } /* END ELSE if ( tot == MSTR ) */



            if ( qh > QQ_SIZE ) {
                //fprintf( stderr, "WARNING: bz_match_score(): qq[] overflow #3 in bozorth3(); qh-1 is %d\n",
                        // qh-1  );
                return QQ_OVERFLOW_SCORE;
            }
            for ( i = qh - 1; i > 0; i-- ) {
                n = qq[i] - 1;
                if ( ( tq[n] - 1 ) >= 0 ) {
                    rq[tq[n]-1] = 0;
                    tq[n]       = 0;
                    zz[n]       = 1000;
                }
            }

            for ( i = dw - 1; i >= 0; i-- ) {
                n = rr[i] - 1;
                if ( tq[n] ) {
                    rq[tq[n]-1] = 0;
                    tq[n]       = 0;
                }
            }

            i = 0;
            j = ww - 1;
            while ( i >= 0 && j >= 0 ) {
                if ( nn[j] < mm[j] ) {
                    ++nn[j];

                    for ( i = ww - 1; i >= 0; i-- ) {
                        int rt = rx[i];
                        if ( rt < 0 ) {
                            rt = - rt;
                            rt--;
                            z  = rf[i][nn[i]-1]-1;



                            if (( tq[z] != (rt+1) && tq[z] ) || ( rq[rt] != (z+1) && rq[rt] ))
                                break;


                            tq[z]  = rt+1;
                            rq[rt] = z+1;
                            rr[i]  = z+1;
                        } else {
                            rt--;
                            z = cf[i][nn[i]-1]-1;


                            if (( tq[rt] != (z+1) && tq[rt] ) || ( rq[z] != (rt+1) && rq[z] ))
                                break;


                            tq[rt] = z+1;
                            rq[z]  = rt+1;
                            rr[i]  = rt+1;
                        }
                    } /* END for i */

                    if ( i >= 0 ) {
                        for ( z = i + 1; z < ww; z++) {
                            n = rr[z] - 1;
                            if ( tq[n] - 1 >= 0 ) {
                                rq[tq[n]-1] = 0;
                                tq[n]       = 0;
                            }
                        }
                        j = ww - 1;
                    }

                } else {
                    nn[j] = 1;
                    j--;
                }

            }

            if ( tp > 1999 )
                break;

            dw = ww;


        } while ( j >= 0 ); /* END while endpoint group remain ... */


        if ( tp > 1999 )
            break;




        n = qq[0] - 1;
        if ( tq[n] - 1 >= 0 ) {
            rq[tq[n]-1] = 0;
            tq[n]       = 0;
        }

        for ( i = ww-1; i >= 0; i-- ) {
            n = rx[i];
            if ( n < 0 ) {
                n = - n;
                rp[n-1] = 0;
            } else {
                cp[n-1] = 0;
            }

        }

    } /* END FOR each edge pair */



    if ( match_score < MMSTR ) {
        return match_score;
    }

    match_score = bz_final_loop( tp );
    return match_score;
}



void Bozorth3_Core::bz_sift(
        int * ww,		/* INPUT and OUTPUT; endpoint groups index; *ww may be bumped by one or by two */
        int   kz,		/* INPUT only;       endpoint of lookahead Subject edge */
        int * qh,		/* INPUT and OUTPUT; the value is an index into qq[] and is stored in zz[]; *qh may be bumped by one */
        int   l,		/* INPUT only;       endpoint of lookahead On-File edge */
        int   kx,		/* INPUT only -- index */
        int   ftt,		/* INPUT only */
        int * tot,		/* OUTPUT -- counter is incremented by one, sometimes */
        int * qq_overflow	/* OUTPUT -- flag is set only if qq[] overflows */
        )
{
    int n;
    int t;

    /* These now externally defined in bozorth.h */
    /* extern FILE * errorfp; */
    /* extern char * get_progname( void ); */
    /* extern char * get_probe_filename( void ); */
    /* extern char * get_gallery_filename( void ); */



    n = tq[ kz - 1];	/* Lookup On-File edgepoint stored in TQ at index of endpoint of lookahead Subject edge */
    t = rq[ l  - 1];	/* Lookup Subject edgepoint stored in RQ at index of endpoint of lookahead On-File edge */

    if ( n == 0 && t == 0 ) {


        if ( sc[kx-1] != ftt ) {
            y[ (*tot)++ ] = kx;
            rk[kx-1] = sc[kx-1];
            sc[kx-1] = ftt;
        }

        if ( *qh >= QQ_SIZE ) {
            //fprintf( stderr, "ERROR: bz_sift(): qq[] overflow #1; the index [*qh] is %d\n",

                  //   *qh );
            *qq_overflow = 1;
            return;
        }
        qq[ *qh ]  = kz;
        zz[ kz-1 ] = (*qh)++;


        /* The TQ and RQ locations are set, so set them ... */
        tq[ kz-1 ] = l;
        rq[ l-1 ] = kz;

        return;
    } /* END if ( n == 0 && t == 0 ) */









    if ( n == l ) {

        if ( sc[kx-1] != ftt ) {
            if ( zz[kx-1] == 1000 ) {
                if ( *qh >= QQ_SIZE ) {
                    //fprintf( stderr, "ERROR: bz_sift(): qq[] overflow #2; the index [*qh] is %d\n",

                         //    *qh
                         //    );
                    *qq_overflow = 1;
                    return;
                }
                qq[*qh]  = kz;
                zz[kz-1] = (*qh)++;
            }
            y[(*tot)++] = kx;
            rk[kx-1] = sc[kx-1];
            sc[kx-1] = ftt;
        }

        return;
    } /* END if ( n == l ) */





    if ( *ww >= WWIM )	/* This limits the number of endpoint groups that can be constructed */
        return;


    {
        int b;
        int b_index;
        register int i;
        int notfound;
        int lim;
        register int * lptr;

        /* If lookahead Subject endpoint previously assigned to TQ but not paired with lookahead On-File endpoint ... */

        if ( n ) {
            b = cp[ kz - 1 ];
            if ( b == 0 ) {
                b              = ++*ww;
                b_index        = b - 1;
                cp[kz-1]       = b;
                cf[b_index][0] = n;
                mm[b_index]    = 1;
                nn[b_index]    = 1;
                rx[b_index]    = kz;

            } else {
                b_index = b - 1;
            }

            lim = mm[b_index];
            lptr = &cf[b_index][0];
            notfound = 1;

#ifndef NOVERBOSE

           /* int * llptr = lptr;
            //printf( "bz_sift(): n: looking for l=%d in [", l );
            for ( i = 0; i < lim; i++ ) {
               // printf( " %d", *llptr++ );
            }*/
           // printf( " ]\n" );

#endif

            for ( i = 0; i < lim; i++ ) {
                if ( *lptr++ == l ) {
                    notfound = 0;
                    break;
                }
            }
            if ( notfound ) {		/* If lookahead On-File endpoint not in list ... */
                cf[b_index][i] = l;
                ++mm[b_index];
            }
        } /* END if ( n ) */


        /* If lookahead On-File endpoint previously assigned to RQ but not paired with lookahead Subject endpoint... */

        if ( t ) {
            //print_array(rp,RP_SIZE);
            b = rp[ l - 1 ];
            if ( b == 0 ) {
                b              = ++*ww;
                b_index        = b - 1;
                rp[l-1]        = b;
                rf[b_index][0] = t;
                mm[b_index]    = 1;
                nn[b_index]    = 1;
                rx[b_index]    = -l;


            } else {
                b_index = b - 1;
            }

            lim = mm[b_index];
            lptr = &rf[b_index][0];
            notfound = 1;

#ifndef NOVERBOSE

            /*int * llptr = lptr;
            //printf( "bz_sift(): t: looking for kz=%d in [", kz );
            for ( i = 0; i < lim; i++ ) {
            //    printf( " %d", *llptr++ );
            }*/
          //  printf( " ]\n" );

#endif

            for ( i = 0; i < lim; i++ ) {
                if ( *lptr++ == kz ) {
                    notfound = 0;
                    break;
                }
            }
            if ( notfound ) {		/* If lookahead Subject endpoint not in list ... */
                rf[b_index][i] = kz;
                ++mm[b_index];
            }
        } /* END if ( t ) */

    }

}




int Bozorth3_Core::bz_final_loop( int tp )
{
    int ii, i, t, b, n, k, j, kk, jj;
    int lim;
    int match_score;

    /* This array originally declared global, but moved here */
    /* locally because it is only used herein.  The use of   */
    /* "static" is required as the array will exceed the     */
    /* stack allocation on our local systems otherwise.      */
    int sct[ SCT_SIZE_1 ][ SCT_SIZE_2 ];

    match_score = 0;
    for ( ii = 0; ii < tp; ii++ ) {				/* For each index up to the current value of TP ... */

        if ( match_score >= gct[ii] )		/* if next group total not bigger than current match_score.. */
            continue;			/*		skip to next TP index */

        lim = ctt[ii] + 1;
        for ( i = 0; i < lim; i++ ) {
            sct[i][0] = ctp[ii][i];
        }

        t     = 0;
        y[0]  = lim;
        cp[0] = 1;
        b     = 0;
        n     = 1;
        do {					/* looping until T < 0 ... */
            if ( y[t] - cp[t] > 1 ) {
                k = sct[cp[t]][t];
                j = ctt[k] + 1;
                for ( i = 0; i < j; i++ ) {
                    rp[i] = ctp[k][i];
                }
                k  = 0;
                kk = cp[t];
                jj = 0;

                do {
                    while ( rp[jj] < sct[kk][t] && jj < j )
                        jj++;
                    while ( rp[jj] > sct[kk][t] && kk < y[t] )
                        kk++;
                    while ( rp[jj] == sct[kk][t] && kk < y[t] && jj < j ) {
                        sct[k][t+1] = sct[kk][t];
                        k++;
                        kk++;
                        jj++;
                    }
                } while ( kk < y[t] && jj < j );

                t++;
                cp[t] = 1;
                y[t]  = k;
                b     = t;
                n     = 1;
            } else {
                int tot = 0;

                lim = y[t];
                for ( i = n-1; i < lim; i++ ) {
                    tot += ct[ sct[i][t] ];
                }

                for ( i = 0; i < b; i++ ) {
                    tot += ct[ sct[0][i] ];
                }

                if ( tot > match_score ) {		/* If the current total is larger than the running total ... */
                    match_score = tot;		/*	then set match_score to the new total */
                    for ( i = 0; i < b; i++ ) {
                        rk[i] = sct[0][i];
                    }

                    {
                        int rk_index = b;
                        lim = y[t];
                        for ( i = n-1; i < lim; ) {
                            rk[ rk_index++ ] = sct[ i++ ][ t ];
                        }
                    }
                }
                b = t;
                t--;
                if ( t >= 0 ) {
                    ++cp[t];
                    n = y[t];
                }
            } /* END IF */

        } while ( t >= 0 );

    } /* END FOR ii */

    return match_score;

} /* END bz_final_loop() */




int Bozorth3_Core::bozorth_probe_init( struct xyt_struct * pstruct )
{
    int sim;	/* number of pointwise comparisons for Subject's record*/
    int msim;	/* Pruned length of Subject's comparison pointer list */



    /* Take Subject's points and compute pointwise comparison statistics table and sorted row-pointer list. */
    /* This builds a "Web" of relative edge statistics between points. */
    bz_comp(
                pstruct->nrows,
                pstruct->xcol,
                pstruct->ycol,
                pstruct->thetacol,
                &sim,
                scols,
                scolpt );

    msim = sim;	/* Init search to end of Subject's pointwise comparison table (last edge in Web) */



    bz_find( &msim, scolpt );



    if ( msim < FDD )	/* Makes sure there are a reasonable number of edges (at least 500, if possible) to analyze in the Web */
        msim = ( sim > FDD ) ? FDD : sim;

    return msim;
}


void Bozorth3_Core::bz_comp(int npoints,				/* INPUT: # of points */
        int xcol[     MAX_BOZORTH_MINUTIAE ],	/* INPUT: x cordinates */
        int ycol[     MAX_BOZORTH_MINUTIAE ],	/* INPUT: y cordinates */
        int thetacol[ MAX_BOZORTH_MINUTIAE ],	/* INPUT: theta values */

        int * ncomparisons,			/* OUTPUT: number of pointwise comparisons */
        int cols[][COLS_SIZE_2],		/* OUTPUT: pointwise comparison table */
        int * colptrs[]				/* INPUT and OUTPUT: sorted list of pointers to rows in cols[] */
        )
{
    int i, j, k;

    int b;
    int t;
    int n;
    int l;

    int table_index;

    int dx;
    int dy;
    int distance;

    int theta_kj;
    int beta_j;
    int beta_k;

    int * c;



    c = &(cols[0][0]);


    table_index = 0;
    for ( k = 0; k < npoints - 1; k++ ) {
        for ( j = k + 1; j < npoints; j++ ) {


            if ( thetacol[j] > 0 ) {

                if ( thetacol[k] == thetacol[j] - 180 )
                    continue;
            } else {

                if ( thetacol[k] == thetacol[j] + 180 )
                    continue;
            }


            dx = xcol[j] - xcol[k];
            dy = ycol[j] - ycol[k];
            distance = SQUARED(dx) + SQUARED(dy);
            if ( distance > SQUARED(DM) ) {
                if ( dx > DM )
                    break;
                else
                    continue;

            }

            /* The distance is in the range [ 0, 125^2 ] */
            if ( dx == 0 )
                theta_kj = 90;
            else {
                double dz;

                if ( m1_xyt )
                    dz = ( 180.0F / PI_SINGLE ) * atanf( (float) -dy / (float) dx );
                else
                    dz = ( 180.0F / PI_SINGLE ) * atanf( (float) dy / (float) dx );
                if ( dz < 0.0F )
                    dz -= 0.5F;
                else
                    dz += 0.5F;
                theta_kj = (int) dz;
            }


            beta_k = theta_kj - thetacol[k];
            beta_k = IANGLE180(beta_k);

            beta_j = theta_kj - thetacol[j] + 180;
            beta_j = IANGLE180(beta_j);


            if ( beta_k < beta_j ) {
                *c++ = distance;
                *c++ = beta_k;
                *c++ = beta_j;
                *c++ = k+1;
                *c++ = j+1;
                *c++ = theta_kj;
            } else {
                *c++ = distance;
                *c++ = beta_j;
                *c++ = beta_k;
                *c++ = k+1;
                *c++ = j+1;
                *c++ = theta_kj + 400;

            }






            b = 0;
            t = table_index + 1;
            l = 1;
            n = -1;			/* Init binary search state ... */




            while ( t - b > 1 ) {
                int * midpoint;

                l = ( b + t ) / 2;
                midpoint = colptrs[l-1];




                for ( i=0; i < 3; i++ ) {
                    int dd, ff;

                    dd = cols[table_index][i];

                    ff = midpoint[i];


                    n = SENSE(dd,ff);


                    if ( n < 0 ) {
                        t = l;
                        break;
                    }
                    if ( n > 0 ) {
                        b = l;
                        break;
                    }
                }

                if ( n == 0 ) {
                    n = 1;
                    b = l;
                }
            } /* END while */

            if ( n == 1 )
                ++l;




            for ( i = table_index; i >= l; --i )
                colptrs[i] = colptrs[i-1];


            colptrs[l-1] = &cols[table_index][0];
            ++table_index;


            if ( table_index == 19999 ) {
#ifndef NOVERBOSE

              //  printf( "bz_comp(): breaking loop to avoid table overflow\n" );
#endif
                goto COMP_END;
            }

        } /* END for j */

    } /* END for k */

COMP_END:
    *ncomparisons = table_index;

}




void Bozorth3_Core::bz_find(
        int * xlim,		/* INPUT:  number of pointwise comparisons in table */
        /* OUTPUT: determined insertion location (NOT ALWAYS SET) */
        int * colpt[]		/* INOUT:  sorted list of pointers to rows in the pointwise comparison table */
        )
{
    int midpoint;
    int top;
    int bottom;
    int state;
    int distance;



    /* binary search to locate the insertion location of a predefined distance in list of sorted distances */


    bottom   = 0;
    top      = *xlim + 1;
    midpoint = 1;
    state    = -1;

    while ( top - bottom > 1 ) {
        midpoint = ( bottom + top ) / 2;
        distance = *colpt[ midpoint-1 ];
        state = SENSE_NEG_POS(FD,distance);
        if ( state < 0 )
            top = midpoint;
        else {
            bottom = midpoint;
        }
    }

    if ( state > -1 )
        ++midpoint;

    if ( midpoint < *xlim )
        *xlim = midpoint;



}



int Bozorth3_Core::bozorth_gallery_init( struct xyt_struct * gstruct )
{
    int fim;	/* number of pointwise comparisons for On-File record*/
    int mfim;	/* Pruned length of On-File Record's pointer list */


    /* Take On-File Record's points and compute pointwise comparison statistics table and sorted row-pointer list. */
    /* This builds a "Web" of relative edge statistics between points. */
    bz_comp(
                gstruct->nrows,
                gstruct->xcol,
                gstruct->ycol,
                gstruct->thetacol,
                &fim,
                fcols,
                fcolpt );

    mfim = fim;	/* Init search to end of On-File Record's pointwise comparison table (last edge in Web) */



    bz_find( &mfim, fcolpt );



    if ( mfim < FDD )	/* Makes sure there are a reasonable number of edges (at least 500, if possible) to analyze in the Web */
        mfim = ( fim > FDD ) ? FDD : fim;


    return mfim;
}


/***********************************************************************/
/* Builds list of compatible edge pairs between the 2 Webs. */
/* The Edge pair DeltaThetaKJs and endpoints are sorted     */
/*	first on Subject's K,                               */
/*	then On-File's J or K (depending),                  */
/*	and lastly on Subject's J point index.              */
/* Return value is the # of compatible edge pairs           */
/***********************************************************************/
int Bozorth3_Core::bz_match(
        int probe_ptrlist_len,		/* INPUT:  pruned length of Subject's pointer list */
        int gallery_ptrlist_len		/* INPUT:  pruned length of On-File Record's pointer list */
        )
{
    int i;			/* Temp index */
    int ii;			/* Temp index */
    int edge_pair_index;	/* Compatible edge pair index */
    float dz;		/* Delta difference and delta angle stats */
    float fi;		/* Distance limit based on factor TK */
    int * ss;		/* Subject's comparison stats row */
    int * ff;		/* On-File Record's comparison stats row */
    int j;			/* On-File Record's row index */
    int k;			/* Subject's row index */
    int st;			/* Starting On-File Record's row index */
    int p1;			/* Adjusted Subject's ThetaKJ, DeltaThetaKJs, K or J point index */
    int p2;			/* Adjusted On-File's ThetaKJ, RTP point index */
    int n;			/* ThetaKJ and binary search state variable */
    int l;			/* Midpoint of binary search */
    int b;			/* ThetaKJ state variable, and bottom of search range */
    int t;			/* Top of search range */

    register int * rotptr;


#define ROT_SIZE_1 20000
#define ROT_SIZE_2 5

    int rot[ ROT_SIZE_1 ][ ROT_SIZE_2 ];


    int * rtp[ ROT_SIZE_1 ];




    /* These now externally defined in bozorth.h */
    /* extern int * scolpt[ SCOLPT_SIZE ];			 INPUT */
    /* extern int * fcolpt[ FCOLPT_SIZE ];			 INPUT */
    /* extern int   colp[ COLP_SIZE_1 ][ COLP_SIZE_2 ];	 OUTPUT */
    /* extern int verbose_bozorth; */
    /* extern FILE * errorfp; */
    /* extern char * get_progname( void ); */
    /* extern char * get_probe_filename( void ); */
    /* extern char * get_gallery_filename( void ); */





    st = 1;
    edge_pair_index = 0;
    rotptr = &rot[0][0];

    /* Foreach sorted edge in Subject's Web ... */

    for ( k = 1; k < probe_ptrlist_len; k++ ) {
        ss = scolpt[k-1];

        /* Foreach sorted edge in On-File Record's Web ... */

        for ( j = st; j <= gallery_ptrlist_len; j++ ) {
            ff = fcolpt[j-1];
            dz = *ff - *ss;

            fi = ( 2.0F * TK ) * ( *ff + *ss );








            if ( SQUARED(dz) > SQUARED(fi) ) {
                if ( dz < 0 ) {

                    st = j + 1;

                    continue;
                } else
                    break;


            }



            for ( i = 1; i < 3; i++ ) {
                float dz_squared;

                dz = *(ss+i) - *(ff+i);
                dz_squared = SQUARED(dz);




                if ( dz_squared > TXS && dz_squared < CTXS )
                    break;
            }

            if ( i < 3 )
                continue;






            if ( *(ss+5) >= 220 ) {
                p1 = *(ss+5) - 580;
                n  = 1;
            } else {
                p1 = *(ss+5);
                n  = 0;
            }


            if ( *(ff+5) >= 220 ) {
                p2 = *(ff+5) - 580;
                b  = 1;
            } else {
                p2 = *(ff+5);
                b  = 0;
            }

            p1 -= p2;
            p1 = IANGLE180(p1);



            if ( n != b ) {

                *rotptr++ = p1;
                *rotptr++ = *(ss+3);
                *rotptr++ = *(ss+4);

                *rotptr++ = *(ff+4);
                *rotptr++ = *(ff+3);
            } else {
                *rotptr++ = p1;
                *rotptr++ = *(ss+3);
                *rotptr++ = *(ss+4);

                *rotptr++ = *(ff+3);
                *rotptr++ = *(ff+4);
            }






            n = -1;
            l = 1;
            b = 0;
            t = edge_pair_index + 1;
            while ( t - b > 1 ) {
                l = ( b + t ) / 2;

                for ( i = 0; i < 3; i++ ) {
                    int ii_table[] = { 1, 3, 2 };

                    /*	1 = Subject's Kth, */
                    /*	3 = On-File's Jth or Kth (depending), */
                    /*	2 = Subject's Jth */

                    ii = ii_table[i];
                    p1 = rot[edge_pair_index][ii];
                    p2 = *( rtp[l-1] + ii );

                    n = SENSE(p1,p2);

                    if ( n < 0 ) {
                        t = l;
                        break;
                    }
                    if ( n > 0 ) {
                        b = l;
                        break;
                    }
                }

                if ( n == 0 ) {
                    n = 1;
                    b = l;
                }
            } /* END while() for binary search */


            if ( n == 1 )
                ++l;

            rtp_insert( rtp, l, edge_pair_index, &rot[edge_pair_index][0] );
            ++edge_pair_index;

            if ( edge_pair_index == 19999 ) {
#ifndef NOVERBOSE
                //fprintf( stderr, "bz_match(): WARNING: list is full, breaking loop early\n"
                        // );
#endif
                goto END;		/* break out if list exceeded */
            }

        } /* END FOR On-File (edge) distance */

    } /* END FOR Subject (edge) distance */



END:
    {
        int * colp_ptr = &colp[0][0];

        for ( i = 0; i < edge_pair_index; i++ ) {
            INT_COPY( colp_ptr, rtp[i], COLP_SIZE_2 );


        }
    }



    return edge_pair_index;			/* Return the number of compatible edge pairs stored into colp[][] */
}


void Bozorth3_Core::rtp_insert( int * rtp[], int l, int idx, int * ptr )
{
    int shiftcount;
    int ** r1;
    int ** r2;


    r1 = &rtp[idx];
    r2 = r1 - 1;

    shiftcount = ( idx - l ) + 1;
    while ( shiftcount-- > 0 ) {
        *r1-- = *r2--;
    }
    *r1 = ptr;
}



int Bozorth3_Core::match(struct xyt_struct * template1,
                    struct xyt_struct * template2
                    )
{
    int ms;
    int np;
    int probe_len;
    int gallery_len;

    if (template1 == XYT_NULL || template2 == XYT_NULL) return 0;

    probe_len   = bozorth_probe_init( template1 );
    gallery_len = bozorth_gallery_init( template2 );
    np = bz_match( probe_len, gallery_len );
    ms = bz_match_score( np, template1, template2 );

    return ms;
}


// ************************************************************* //
// ************************************************************* //
// *************                                  ************** //
// *************   Bozorth3 Multi-thread Manager  ************** //
// *************                                  ************** //
// ************************************************************* //
// ************************************************************* //


BozorthMultiThreadManager::BozorthMultiThreadManager(QObject *parent) : QObject(parent)
{
    this->threadsFinished = 0;
    qRegisterMetaType<FINGERPRINT_PAIRS>();
    //qDebug() << "Bozorth3 manager created.";

    //struct rlimit lim = {1024, 6000000};

    //setrlimit(RLIMIT_STACK, &lim);
}

void BozorthMultiThreadManager::setParameters(int _numThreads, QMap<QString, QVector<MINUTIA>> _minDataAll, FINGERPRINT_PAIRS _fingerprint_pairs)
{
    this->numThreads = _numThreads;
    this->minDataAll = _minDataAll;
    this->fingerprint_pairs = _fingerprint_pairs;
}

void BozorthMultiThreadManager::matchAll()
{
    this->threadsFinished = 0; // at beginning, this value is set to 0
    this->threads.clear(); // previous threads are cleared

    this->outputFingerprintPairs.clear();

    // parallel matching
    this->timer.start();

    // Bozorth workers
    QVector<BozorthThread*> bozorthThreads;

    // prepare data for threads in a way that threads cannot access any data simultaneously
    if(!this->distributeFingerprintPairs()){
        qDebug() << "Error: Vector of fingerprint pairs is empty.";
        return;
    }

    int tmpNumThreads = this->numThreads;
    if(this->fingerprint_pairs.size() < tmpNumThreads){
        tmpNumThreads = this->fingerprint_pairs.size();
    }

    for(int i = 0; i<tmpNumThreads; i++){
        this->threads.push_back(new QThread());

        bozorthThreads.push_back(new BozorthThread(this->minDataAll, this->thread_fingerprint_pairs[i]));
        bozorthThreads.last()->moveToThread(this->threads.last()); // adding to separate thread

        connect(bozorthThreads.last(), SIGNAL(stateSignal(int)), this, SLOT(stateSlot(int)));
        connect(this->threads.last(), SIGNAL(finished()), bozorthThreads.last(), SLOT(deleteLater()));
        connect(this->threads.last(), SIGNAL(finished()), this->threads.last(), SLOT(deleteLater()));
        connect(bozorthThreads.last(), SIGNAL(matchSignal()), bozorthThreads.last(), SLOT(matchSlot()));
        connect(bozorthThreads.last(), SIGNAL(matchingDoneSignal(FINGERPRINT_PAIRS)), this, SLOT(oneBozorthThreadFinished(FINGERPRINT_PAIRS)));
        connect(bozorthThreads.last(), SIGNAL(matchingDoneSignal(FINGERPRINT_PAIRS)), bozorthThreads.last(), SLOT(matchingDoneSlot(FINGERPRINT_PAIRS)));
        this->threads.last()->start(); // thread enters the event loop

        emit bozorthThreads.last()->matchSignal(); // emitting signal to start matching
    }
}

bool BozorthMultiThreadManager::distributeFingerprintPairs()
{
    if(this->fingerprint_pairs.empty()){
        return false;
    }

    this->thread_fingerprint_pairs.clear();
    // shuffling a vector of comparisons to maximize even distribution of fingerprints with many minutiae among
    // all threads
    std::random_shuffle(this->fingerprint_pairs.begin(), this->fingerprint_pairs.end());

    FINGERPRINT_PAIRS local_fingerprint_pairs;

    int THREAD_BATCH = this->fingerprint_pairs.size()/this->numThreads; // number of jobs per thread

    // if number of jobs is less than number of threads
    if(THREAD_BATCH == 0){
        for(int i=0; i< this->fingerprint_pairs.size();i++){
            local_fingerprint_pairs.clear();
            local_fingerprint_pairs.push_back(this->fingerprint_pairs[i]);
            this->thread_fingerprint_pairs.push_back(local_fingerprint_pairs);
        }
    }
    else{
        // distribute comparisons among threads
        for(int i=0; i<this->numThreads-1; i++){
            local_fingerprint_pairs.clear();
            for(int j=i*THREAD_BATCH;j<i*THREAD_BATCH + THREAD_BATCH; j++){
                local_fingerprint_pairs.push_back(this->fingerprint_pairs[j]);
            }
            this->thread_fingerprint_pairs.push_back(local_fingerprint_pairs);
        }

        // for last thread
        local_fingerprint_pairs.clear();
        for(int j=THREAD_BATCH*(this->numThreads-1);j<this->fingerprint_pairs.size(); j++){
            local_fingerprint_pairs.push_back(this->fingerprint_pairs[j]);
        }
        this->thread_fingerprint_pairs.push_back(local_fingerprint_pairs);
    }
    local_fingerprint_pairs.clear();

//    qDebug() << "To distribute: " << this->fingerprint_pairs.size() << " job(s).";
//    qDebug() << "Number of threads: " << this->numThreads;
//    qDebug() << "Thread batch: " << THREAD_BATCH << " job(s).";
//    int counter =0;
//    for(FINGERPRINT_PAIRS fp : this->thread_fingerprint_pairs){
//        qDebug() << "Thread " << ++counter << ": " << fp.size() << " job(s).";
//    }
//    qDebug() << "******************";

    return true;

}

void BozorthMultiThreadManager::oneBozorthThreadFinished(FINGERPRINT_PAIRS fp)
{
    // if all threads finished
   // qDebug() << this->threadsFinished+1 << "threads finished.";

    foreach (FINGERPRINT_PAIR fPair, fp) {
        this->outputFingerprintPairs.push_back(fPair);
    }

    if(++this->threadsFinished == this->numThreads || this->threadsFinished == this->fingerprint_pairs.size()){
        foreach (QThread* t, this->threads) {
            t->quit();
            t->wait();
        }
        this->fingerprint_pairs.clear();

        //qDebug() << "********   ALL JOBS COMPLETED   ********";
        emit bozorthThreadsFinished(this->timer.elapsed());
    }
}

void BozorthMultiThreadManager::stateSlot(int state)
{
    emit stateSignal(state);
}

FINGERPRINT_PAIRS BozorthMultiThreadManager::getOutputFingerprintPairs() const
{
    return outputFingerprintPairs;
}

QVector<FINGERPRINT_PAIRS> BozorthMultiThreadManager::getThread_fingerprint_pairs() const
{
    return thread_fingerprint_pairs;
}





// ************************************************************* //
// ************************************************************* //
// *************                                  ************** //
// *************        Bozorth3 thread           ************** //
// *************                                  ************** //
// ************************************************************* //
// ************************************************************* //


BozorthThread::BozorthThread(QObject *parent) : QObject(parent)
{

}

BozorthThread::BozorthThread(QMap<QString, QVector<MINUTIA> > _minDataAll, FINGERPRINT_PAIRS _thread_fingerprint_pairs):
    minDataAll(_minDataAll), pairs_for_thread(_thread_fingerprint_pairs)
{
  //  qDebug() << "Bozorth3 thread created.";
}

void BozorthThread::matchSlot()
{
    //qDebug() << "Thread ID: " + QString::number((int)QThread::currentThreadId());
    //qDebug() << "Thread batch:" << this->pairs_for_thread.size();
    this->loadXYT(); // load xyt data structures

    int cnt = 0;
    QTime timer;
    timer.start();

    for (FINGERPRINT_PAIR& onePair : this->pairs_for_thread) {

        onePair.score = this->matcher.match(this->xytData.value(onePair.leftFingerprint), this->xytData.value(onePair.rightFingerprint));
        //qDebug() << "[Score:" << onePair.score << "]" << onePair.leftFingerprint << " vs. " << onePair.rightFingerprint;

        if (cnt % 100 == 0) {
            //qDebug() << cnt;
            emit stateSignal((int)(((1.0*cnt)/pairs_for_thread.size())*100));
        }
        cnt++;
    }
    emit matchingDoneSignal(this->pairs_for_thread);

    //qDebug() << timer.elapsed()/1000;
}

void BozorthThread::matchingDoneSlot(FINGERPRINT_PAIRS)
{
    //qDebug() << "Thread ID: " + QString::number((int)QThread::currentThreadId()) + " done.";
}

FINGERPRINT_PAIRS BozorthThread::getPairs_for_thread() const
{
    return pairs_for_thread;
}

void BozorthThread::loadXYT()
{

    this->xytData.clear();
    QMapIterator<QString, QVector<MINUTIA>> iter(this->minDataAll);
    while (iter.hasNext()) {
        iter.next();
        this->xytData.insert(iter.key(),this->matcher.bz_load(iter.value(),false));
    }

}
