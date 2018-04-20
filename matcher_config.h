#ifndef MATCHER_CONFIG_H
#define MATCHER_CONFIG_H

#include <QObject>
#include <QThread>
#include <QPoint>
#include <QVector>
#include <QTime>
#include <QDebug>
#include <QFile>
#include <QDataStream>
#include <QMap>
#include <QMultiMap>
#include <QString>
#include <QMetaType>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <bitset>
#include <string>
#include <sys/resource.h>

enum MATCHER {bozorth3, suprema, mcc};
enum MODE {identification, verification, dbtest};

typedef struct fingerprint_pair {
    QString leftFingerprint;
    QString rightFingerprint;
    float score;
} FINGERPRINT_PAIR;

typedef QVector<FINGERPRINT_PAIR> FINGERPRINT_PAIRS;

typedef struct matchtresholds {
    int bozorthThr;
    float supremaThr;
    int mccThr;
} MATCH_TRESHOLDS;

#ifndef MINUTIA_DEFINED
typedef struct minutia {
    QPoint xy;
    int type; // 0-end, 1-bif
    qreal angle; // in radians
    int quality;
} MINUTIA;
#define MINUTIA_DEFINED
#endif

#endif // MATCHER_CONFIG_H
