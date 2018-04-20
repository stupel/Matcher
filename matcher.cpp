#include "matcher.h"

Matcher::Matcher()
{
    this->matcher = bozorth3;
    this->thresholds = {50, 0.5, 0};  // ????????!!!!!!!

    this->numberOfSubject = 0;
    this->imgPerSubject = 0;

    connect(&this->bozorth3m, SIGNAL(bozorthThreadsFinished(int)), this, SLOT(bozorthMatchingDone(int)));
}


//SETTERS

void Matcher::setMatcher(MATCHER matcher)
{
    this->matcher = matcher;
}

void Matcher::setDBTestParams(int numberOfSubject, int imgPerSubject)
{
    this->numberOfSubject = numberOfSubject;
    this->imgPerSubject = imgPerSubject;
}


// PAIR GENERATORS

void Matcher::generatePairs()
{
    this->fingerprintPairs.clear();

    for (auto i = this->alternativeNames.begin(); i != this->alternativeNames.end(); ++i) {
        this->fingerprintPairs.push_back(FINGERPRINT_PAIR{"SUBJECT", i.key(), 0});
    }
}

void Matcher::generateGenuinePairs(const QVector<QPair<QString, QVector<MINUTIA>>> &db)
{
    this->fingerprintPairs.clear();

    for (int subject = 0; subject < this->numberOfSubject; subject++) {
        for(int image1 = subject * this->imgPerSubject; image1 < subject * this->imgPerSubject + this->imgPerSubject; image1++) {
            for(int image2 = image1+1; image2 < subject * this->imgPerSubject + this->imgPerSubject; image2++) {

                this->fingerprintPairs.push_back(FINGERPRINT_PAIR{db[image1].first, db[image2].first, 0});

            }
        }
    }
}

void Matcher::generateImpostorPairs(const QVector<QPair<QString, QVector<MINUTIA>>> &db)
{   
    this->fingerprintPairs.clear();

    for (int subject = 0; subject < this->numberOfSubject-1; subject++) {
        for (int image1 = subject * this->imgPerSubject; image1 < subject * this->imgPerSubject + this->imgPerSubject; image1++) {
            for (int image2 = (subject+1) * this->imgPerSubject; image2 < this->numberOfSubject*this->imgPerSubject; image2++) {

                this->fingerprintPairs.push_back(FINGERPRINT_PAIR{db[image1].first, db[image2].first, 0});

            }
        }
    }
}





//IDENTIFICATION, VERIFICATION, DBTEST

void Matcher::identify(const unsigned char* &subjectISO, const QMultiMap<QString, unsigned char*> &dbISO)
{
    this->mode = identification;

    if (this->matcher == bozorth3) {
        //ALTERNATIVE_NAMES, BOZORTH TEMPLATES
        this->alternativeNames.clear();
        this->bozorthTemplates.clear();

        this->bozorthTemplates.insert("SUBJECT", this->isoConverter.convertFromISO(subjectISO));

        for (auto i = dbISO.begin(); i != dbISO.end(); ++i) {
            if (this->alternativeNames.contains(i.key())) {
                int cnt = 0;
                while (this->alternativeNames.contains(i.key() + "_" + QString::number(cnt))) {
                    cnt++;
                }
                this->alternativeNames.insert(i.key() + "_" + QString::number(cnt), i.key());
                this->bozorthTemplates.insert(i.key() + "_" + QString::number(cnt), this->isoConverter.convertFromISO(i.value()));
            }
            else {
                this->alternativeNames.insert(i.key(), i.key());
                this->bozorthTemplates.insert(i.key(), this->isoConverter.convertFromISO(i.value()));
            }
        }
        this->generatePairs();
        this->bozorth3m.setParameters(QThread::idealThreadCount(), this->bozorthTemplates, this->fingerprintPairs);
        this->bozorth3m.matchAll();
    }
}

void Matcher::identify(const QVector<MINUTIA> &subject, const QMultiMap<QString, QVector<MINUTIA>> &db)
{
    this->mode = identification;

    if (this->matcher == bozorth3) {
        //ALTERNATIVE_NAMES, BOZORTH TEMPLATES
        this->alternativeNames.clear();
        this->bozorthTemplates.clear();

        this->bozorthTemplates.insert("SUBJECT", subject);

        for (auto i = db.begin(); i != db.end(); ++i) {
            if (this->alternativeNames.contains(i.key())) {
                int cnt = 0;
                while (this->alternativeNames.contains(i.key() + "_" + QString::number(cnt))) {
                    cnt++;
                }
                this->alternativeNames.insert(i.key() + "_" + QString::number(cnt), i.key());
                this->bozorthTemplates.insert(i.key() + "_" + QString::number(cnt), i.value());
            }
            else {
                this->alternativeNames.insert(i.key(), i.key());
                this->bozorthTemplates.insert(i.key(), i.value());
            }
        }
        this->generatePairs();
        this->bozorth3m.setParameters(QThread::idealThreadCount(), this->bozorthTemplates, this->fingerprintPairs);
        this->bozorth3m.matchAll();
    }
}

void Matcher::verify(const unsigned char* &subjectISO, const QVector<unsigned char *> &dbISO)
{
    this->mode = verification;

    if (this->matcher == bozorth3) {
        // ALTERNATIVE NAMES
        this->alternativeNames.clear();
        this->bozorthTemplates.clear();

        this->bozorthTemplates.insert("SUBJECT", this->isoConverter.convertFromISO(subjectISO));

        for (int i = 0; i < dbISO.size(); i++) {
            this->alternativeNames.insert(QString::number(i), QString::number(i));
            this->bozorthTemplates.insert(QString::number(i), this->isoConverter.convertFromISO(dbISO[i]));
        }
        this->generatePairs();
        this->bozorth3m.setParameters(QThread::idealThreadCount(), this->bozorthTemplates, this->fingerprintPairs);
        this->bozorth3m.matchAll();
    }
}

void Matcher::verify(const QVector<MINUTIA> &subject, const QVector<QVector<MINUTIA> > &db)
{
    this->mode = verification;

    if (this->matcher == bozorth3) {
        // ALTERNATIVE NAMES
        this->alternativeNames.clear();
        this->bozorthTemplates.clear();

        this->bozorthTemplates.insert("SUBJECT", subject);

        for (int i = 0; i < db.size(); i++) {
            this->alternativeNames.insert(QString::number(i), QString::number(i));
            this->bozorthTemplates.insert(QString::number(i), db[i]);
        }
        this->generatePairs();
        this->bozorth3m.setParameters(QThread::idealThreadCount(), this->bozorthTemplates, this->fingerprintPairs);
        this->bozorth3m.matchAll();
    }
}

void Matcher::testDatabase(const QVector<QPair<QString, QVector<MINUTIA>>> &db)
{
    this->mode = dbtest;

    this->fingerprintPairs.clear();
    this->bozorthTemplates.clear();

    this->generateGenuinePairs(db);

    if (this->matcher == bozorth3) {
        for (int i = 0; i < db.size(); i++) {
            this->bozorthTemplates.insert(db[i].first, db[i].second);
        }
    }
}



//SLOTS
void Matcher::bozorthMatchingDone(int duration)
{
    this->fingerprintPairs = this->bozorth3m.getOutputFingerprintPairs();

    if (this->mode == identification) {
        int bestMatch = this->findMaxScoreItem();
        if (this->fingerprintPairs[bestMatch].score >= this->thresholds.bozorthThr)
            emit identificationDoneSignal(true, this->alternativeNames.value(this->fingerprintPairs[bestMatch].rightFingerprint), this->fingerprintPairs[bestMatch].score);
        else emit identificationDoneSignal(false, "", -1);
    }
    else if (this->mode == verification) {
        int bestMatch = this->findMaxScoreItem();
        if (this->fingerprintPairs[bestMatch].score >= this->thresholds.bozorthThr)
            emit verificationDoneSignal(true);
        else emit verificationDoneSignal(false);
    }
    else if (this->mode == dbtest) {

    }
}


//OTHER
int Matcher::findMaxScoreItem()
{
    int maxItemNum = 0;
    float max = this->fingerprintPairs[0].score;

    for (int i = 1; i < this->fingerprintPairs.size(); i++) {
        //qDebug() << this->fingerprintPairs[i].score;
        if (this->fingerprintPairs[i].score > max) maxItemNum = i;
    }

    return maxItemNum;
}
