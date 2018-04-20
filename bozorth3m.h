#ifndef BOZORTH3M_H
#define BOZORTH3M_H

#include "matcher_config.h"

/*************    general     **************/

#define DEFAULT_BOZORTH_MINUTIAE	150
#define MAX_BOZORTH_MINUTIAE		200
#define MIN_BOZORTH_MINUTIAE		0
#define MIN_COMPUTABLE_BOZORTH_MINUTIAE	10
#define MAX_LINE_LENGTH 1024
#define CNULL  ((char *) NULL)
#define RR_SIZE     100
#define AVN_SIZE      5
#define AVV_SIZE_1 2000
#define AVV_SIZE_2    5
#define DEFAULT_MAX_MATCH_SCORE		400
#define ZERO_MATCH_SCORE		0
#define YL_SIZE_1    2
#define YL_SIZE_2 2000
#define SC_SIZE 20000
#define CP_SIZE 20000
#define RP_SIZE 20000
#define RQ_SIZE 20000
#define TQ_SIZE 20000
#define ZZ_SIZE 20000
#define FD	5625
#define RX_SIZE 100
#define MM_SIZE 100
#define NN_SIZE 20
#define RK_SIZE 20000
#define RR_SIZE 100
#define RF_SIZE_1 100
#define RF_SIZE_2  10
#define CF_SIZE_1 100
#define CF_SIZE_2  10
#define Y_SIZE 20000
#define COLP_SIZE_1 20000
#define COLP_SIZE_2 5
#define COLS_SIZE_2 6
#define SCOLS_SIZE_1 20000
#define FCOLS_SIZE_1 20000
#define SCOLPT_SIZE 20000
#define FCOLPT_SIZE 20000
#define YL_SIZE_1    2
#define YL_SIZE_2 2000
#define YY_SIZE_1 1000
#define YY_SIZE_2    2
#define YY_SIZE_3 2000
#define CT_SIZE    2000
#define GCT_SIZE   2000
#define CTT_SIZE   2000
#define MMSTR	8
#define WWIM	10
#define DM	125
#define TK	0.05F
#define QQ_SIZE 4000
#define QQ_OVERFLOW_SCORE QQ_SIZE
#define MSTR	3
#define TXS	121
#define CTXS	121801
#define FDD	500
#define Y_SIZE 20000

#define SENSE_NEG_POS(a,b)	( (a) < (b) ? (-1) : 1 )

#define IANGLE180(deg)		( ( (deg) > 180 ) ? ( (deg) - 360 ) : ( (deg) <= -180 ? ( (deg) + 360 ) : (deg) ) )

#define SQUARED(n)		( (n) * (n) )

#define INT_SET(dst,count,value) { \
        int * int_set_dst   = (dst); \
        int   int_set_count = (count); \
        int   int_set_value = (value); \
        while ( int_set_count-- > 0 ) \
            *int_set_dst++ = int_set_value; \
        }

/* The code that calls it assumed dst gets bumped, so don't assign to a local variable */
#define INT_COPY(dst,src,count) { \
        int * int_copy_src = (src); \
        int int_copy_count = (count); \
        while ( int_copy_count-- > 0 ) \
            *dst++ = *int_copy_src++; \
        }

#define SENSE(a,b)		( (a) < (b) ? (-1) : ( ( (a) == (b) ) ? 0 : 1 ) )

#ifdef ROUND_USING_LIBRARY
/* These functions should be declared in math.h:
    extern float  roundf( float  );
    extern double round(  double );
*/
#define ROUND(f) (roundf(f))
#else
#define ROUND(f) ( ( (f) < 0.0F ) ? ( (int) ( (f) - 0.5F ) ) : ( (int) ( (f) + 0.5F ) ) )
#endif

#define CTP_SIZE_1 2000

#ifdef BAD_BOUNDS
#define CTP_SIZE_2 1000
#else
#define CTP_SIZE_2 2500
#endif

#define SCT_SIZE_2 1000
#ifdef BAD_BOUNDS
#define SCT_SIZE_1 1000
#else
#define SCT_SIZE_1 2500
#endif


/* PI is used in: bozorth3.c, comp.c */
#ifdef M_PI
#define PI		M_PI
#define PI_SINGLE	( (float) PI )
#else
#define PI		3.14159
#define PI_SINGLE	3.14159F
#endif

/*************    bz_load()   **************/

#define MAX_FILE_MINUTIAE       1000

struct xyt_struct {
    int nrows;
    int xcol[     MAX_BOZORTH_MINUTIAE ];
    int ycol[     MAX_BOZORTH_MINUTIAE ];
    int thetacol[ MAX_BOZORTH_MINUTIAE ];
};

struct xytq_struct {
        int nrows;
        int xcol[     MAX_FILE_MINUTIAE ];
        int ycol[     MAX_FILE_MINUTIAE ];
        int thetacol[ MAX_FILE_MINUTIAE ];
        int qualitycol[ MAX_FILE_MINUTIAE ];
};

#define XYT_NULL ( (struct xyt_struct *) NULL )
#define XYTQ_NULL ( (struct xytq_struct *) NULL )

/*************    bz_prune()   **************/

/* Used by call to stdlib qsort() */
struct minutiae_struct {
    int col[4];
};


/*************    sort_order_decreasing()   **************/

/* Used by custom quicksort */
#define BZ_STACKSIZE    1000
struct cell {
    int		index;	/* pointer to an array of pointers to index arrays */
    int		item;	/* pointer to an item array */
};


// ************************************************************* //
// ************************************************************* //
// *************                                  ************** //
// *************       Bozorth3 core library      ************** //
// *************                                  ************** //
// ************************************************************* //
// ************************************************************* //

class Bozorth3_Core : public QObject
{

public:
    Bozorth3_Core();
    ~Bozorth3_Core();

    struct xyt_struct * bz_load(QVector<MINUTIA> minutiae, bool useQuality);
    struct xyt_struct * bz_prune(struct xytq_struct *xytq_s, int verbose_load);
    int sort_order_decreasing(int [], int, int []);
    int qsort_decreasing( struct cell v[], int left, int right );
    int pushstack( int position );
    int popstack( int *popval );
    void partition_dec( struct cell v[], int *llen, int *rlen, int *ll, int *lr, int *rl, int *rr, int p, int l, int r );
    int select_pivot( struct cell v[], int left, int right );
    int bz_match_score(int np, struct xyt_struct * pstruct, struct xyt_struct * gstruct);
    int bz_match(
        int probe_ptrlist_len,		/* INPUT:  pruned length of Subject's pointer list */
        int gallery_ptrlist_len		/* INPUT:  pruned length of On-File Record's pointer list */
    );
    void bz_sift(
        int * ww,		/* INPUT and OUTPUT; endpoint groups index; *ww may be bumped by one or by two */
        int   kz,		/* INPUT only;       endpoint of lookahead Subject edge */
        int * qh,		/* INPUT and OUTPUT; the value is an index into qq[] and is stored in zz[]; *qh may be bumped by one */
        int   l,		/* INPUT only;       endpoint of lookahead On-File edge */
        int   kx,		/* INPUT only -- index */
        int   ftt,		/* INPUT only */
        int * tot,		/* OUTPUT -- counter is incremented by one, sometimes */
        int * qq_overflow	/* OUTPUT -- flag is set only if qq[] overflows */
    );
    int bz_final_loop( int tp );
    int bozorth_probe_init( struct xyt_struct * pstruct );
    int bozorth_gallery_init( struct xyt_struct * gstruct );
    void bz_comp(int npoints,				/* INPUT: # of points */
        int xcol[     MAX_BOZORTH_MINUTIAE ],	/* INPUT: x cordinates */
        int ycol[     MAX_BOZORTH_MINUTIAE ],	/* INPUT: y cordinates */
        int thetacol[ MAX_BOZORTH_MINUTIAE ],	/* INPUT: theta values */

        int * ncomparisons,			/* OUTPUT: number of pointwise comparisons */
        int cols[][COLS_SIZE_2],		/* OUTPUT: pointwise comparison table */
        int * colptrs[]				/* INPUT and OUTPUT: sorted list of pointers to rows in cols[] */
    );
    void bz_find(
        int * xlim,		/* INPUT:  number of pointwise comparisons in table */
                    /* OUTPUT: determined insertion location (NOT ALWAYS SET) */
        int * colpt[]		/* INOUT:  sorted list of pointers to rows in the pointwise comparison table */
    );
    void rtp_insert( int * rtp[], int l, int idx, int * ptr );
    int match(struct xyt_struct * template1, struct xyt_struct * template2);
private:
    // Warning: watch the stack size
    int m1_xyt;
    int max_minutiae; // maximalny pocet markantov
    int min_computable_minutiae;
    int yl[ YL_SIZE_1 ][ YL_SIZE_2 ]; // 2
    int rx[ RX_SIZE ]; // 100
    int mm[ MM_SIZE ]; // 100
    int nn[ NN_SIZE ]; // 20
    int qq[ QQ_SIZE ]; // 4000
    int rf[RF_SIZE_1][RF_SIZE_2]; // 10 x 100
    int cf[CF_SIZE_1][CF_SIZE_2]; // 10 x 100

    int sc[ SC_SIZE ]; // 20 000
    int rq[ RQ_SIZE ]; // 20 000
    int tq[ TQ_SIZE ]; // 20 000
    int zz[ ZZ_SIZE ]; // 20 000
    int rk[ RK_SIZE ]; // 20 000
    int cp[ CP_SIZE ]; // 20 000
    int rp[ RP_SIZE ]; // 20 000
    int y[ Y_SIZE ]; // 20 000
    int yy[ YY_SIZE_1 ][ YY_SIZE_2 ][ YY_SIZE_3 ]; // 1000 x 2 x 2000
    int ct[ CT_SIZE ]; // 2000
    int gct[ GCT_SIZE ]; // 2000
    int ctt[ CTT_SIZE ]; // 2000
    int ctp[ CTP_SIZE_1 ][ CTP_SIZE_2 ]; // 2000 x 1000
    int colp[ COLP_SIZE_1 ][ COLP_SIZE_2 ]; // 20 000 x 5
    int scols[ SCOLS_SIZE_1 ][ COLS_SIZE_2 ]; // 20 000 x 6
    int fcols[ FCOLS_SIZE_1 ][ COLS_SIZE_2 ]; // 20 000 x 6
    int * scolpt[ SCOLPT_SIZE ]; // 20 000
    int * fcolpt[ FCOLPT_SIZE ]; // 20 000

    /* Used by custom quicksort code below */
    int   stack[BZ_STACKSIZE]; // 1000
    int * stack_pointer;
};


// ************************************************************* //
// ************************************************************* //
// *************                                  ************** //
// *************   Bozorth3 Multi-thread Manager  ************** //
// *************                                  ************** //
// ************************************************************* //
// ************************************************************* //

class BozorthThread;

// registracia typov
Q_DECLARE_METATYPE(FINGERPRINT_PAIRS)

class BozorthMultiThreadManager : public QObject
{
    Q_OBJECT

public:
    explicit BozorthMultiThreadManager(QObject *parent = nullptr);
    void setParameters(int _numThreads, QMap<QString, QVector<MINUTIA>> _minDataAll, FINGERPRINT_PAIRS _fingerprint_pairs);
    void matchAll();
    bool distributeFingerprintPairs();

    QVector<FINGERPRINT_PAIRS> getThread_fingerprint_pairs() const;

    FINGERPRINT_PAIRS getOutputFingerprintPairs() const;

signals:
    void bozorthThreadsFinished(int); // this signal is emitted after all threads finished
    void stateSignal(int state);

public slots:
    void oneBozorthThreadFinished(FINGERPRINT_PAIRS fp);

private:

    QMap<QString, QVector<MINUTIA>> minDataAll; // minutiae data for all fingerprints
    FINGERPRINT_PAIRS fingerprint_pairs; // all fingerprint pairs to match
    FINGERPRINT_PAIRS outputFingerprintPairs; // output fingerprint pairs with score assigned to it
    QVector<FINGERPRINT_PAIRS> thread_fingerprint_pairs; // each element in this vector represents a vector of fingerprint pairs per thread

    QVector<QThread*> threads; // vector of threads executing Bozorth matching in parallel
    int numThreads; // number of threads
    int threadsFinished; // number of finished threads
    QTime timer; // for measuring time

private slots:
    void stateSlot(int state);

};






// ************************************************************* //
// ************************************************************* //
// *************                                  ************** //
// *************        Bozorth3 thread           ************** //
// *************                                  ************** //
// ************************************************************* //
// ************************************************************* //

typedef QMap<QString,xyt_struct*> XYT_DATA;

class BozorthThread : public QObject
{
    Q_OBJECT
public:
    explicit BozorthThread(QObject *parent = nullptr);
    BozorthThread(QMap<QString, QVector<MINUTIA>> _minDataAll, FINGERPRINT_PAIRS _thread_fingerprint_pairs);
    FINGERPRINT_PAIRS getPairs_for_thread() const;

public slots:
    void matchSlot();
    void matchingDoneSlot(FINGERPRINT_PAIRS);

signals:
    void matchSignal();
    void matchingDoneSignal(FINGERPRINT_PAIRS);
    void stateSignal(int state);

private:
    XYT_DATA xytData;
    FINGERPRINT_PAIRS pairs_for_thread; // fingerprint pairs to match by this thread
    QMap<QString, QVector<MINUTIA>> minDataAll;
    Bozorth3_Core matcher;
    void loadXYT();
};

#endif // BOZORTH3M_H
