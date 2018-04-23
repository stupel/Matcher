#ifndef MATCHER_H
#define MATCHER_H

#include "matcher_global.h"
#include "matcher_config.h"

#include "matcher_isoconverter.h"
#include "bozorth3m.h"

class MATCHERSHARED_EXPORT Matcher : public QObject
{
    Q_OBJECT

public:
    Matcher();

    BozorthMultiThreadManager bozorth3m;

    void setMatcher(MATCHER matcher);
    void setDBTestParams(int numberOfSubject, int imgPerSubject);

    void identify(unsigned char* subjectISO, const QMultiMap<QString, unsigned char*> &dbISO);
    void identify(const QVector<MINUTIA> &subject, const QMultiMap<QString, QVector<MINUTIA> > &db);

    void verify(const unsigned char* subjectISO, const QVector<unsigned char*> &dbISO);
    void verify(const QVector<MINUTIA> &subject, const QVector<QVector<MINUTIA> > &db);

    void testDatabase(const QMap<QString, QVector<MINUTIA> > &db);


private:

    MATCHER matcher;
    MatcherISOConverter isoConverter;


    MODE mode;
    MATCH_TRESHOLDS thresholds;

    QMap<QString, QVector<MINUTIA>> bozorthTemplates;
    QMap<QString, QVector<unsigned char*>> supremaTemplates;

    FINGERPRINT_PAIRS fingerprintPairs;
    QMap<QString, QString> alternativeNames;

    DBTEST_PARAMS dbtestParams;
    DBTEST_RESULT dbtestResult;


    void generatePairs();
    void generateGenuinePairs();
    void generateImpostorPairs();

    int findMaxScoreItem();
    double computeEERValue();


private slots:
    void bozorthMatchingDone(int duration);


signals:
    void matcherErrorSignal(int errorCode);
    void identificationDoneSignal(bool success, QString subject, float score);
    void verificationDoneSignal(bool success);
    void dbTestDoneSignal(DBTEST_RESULT result);

};

#endif // MATCHER_H
