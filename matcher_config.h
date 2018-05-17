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

#include "UFMatcher.h" // Suprema

enum MATCHER {bozorth3, suprema, mcc};
enum MODE {identification, verification, dbtest};

typedef struct fingerprint_pair {
    QString leftFingerprint;
    QString rightFingerprint;
    float score;
} FINGERPRINT_PAIR;

typedef QVector<FINGERPRINT_PAIR> FINGERPRINT_PAIRS;

typedef struct plot_params {
    int min;
    int max;
    double sensitivity;
} PLOT_PARAMS;

typedef struct match_tresholds {
    int bozorthThr;
    float supremaThr;
} MATCH_TRESHOLDS;

#ifndef MINUTIA_DEFINED
typedef struct minutia {
    QPoint xy;
    int type; // 0-end, 1-bif
    qreal angle; // in radians
    int quality;
    QPoint imgWH; // image Width, Height
} MINUTIA;
#define MINUTIA_DEFINED
#endif

typedef struct dbtest_params {
    QMap<QString, QVector<MINUTIA>> db;
    QVector<QString> keys;
    FINGERPRINT_PAIRS genuinePairs;
    FINGERPRINT_PAIRS impostorPairs;
    int numberOfSubject;
    int imgPerSubject;
    bool genuineTestDone;
} DBTEST_PARAMS;

typedef struct dbtest_result {
    QVector<double> fnmrX;
    QVector<double> fnmrY;
    QVector<double> fmrX;
    QVector<double> fmrY;
    float eer;
    PLOT_PARAMS plotParams;
} DBTEST_RESULT;

typedef struct suprema_matcher {
    HUFMatcher matcher;
    bool loaded;
    QVector<float> scores;
} SUPREMA_MATCHER;

#endif // MATCHER_CONFIG_H
