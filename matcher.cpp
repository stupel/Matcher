#include "matcher.h"

Matcher::Matcher()
{
    this->matcher = bozorth3;
    this->thresholds = {50, 0.5, 0};  // ????????!!!!!!!

    this->dbtestParams.numberOfSubject = 0;
    this->dbtestParams.imgPerSubject = 0;
    this->dbtestParams.genuineTestDone = false;

    connect(&this->bozorth3m, SIGNAL(bozorthThreadsFinished(int)), this, SLOT(bozorthMatchingDone(int)));
}


//SETTERS

void Matcher::setMatcher(MATCHER matcher)
{
    this->matcher = matcher;
}

void Matcher::setDBTestParams(int numberOfSubject, int imgPerSubject)
{
    this->dbtestParams.numberOfSubject = numberOfSubject;
    this->dbtestParams.imgPerSubject = imgPerSubject;
}


// PAIR GENERATORS

void Matcher::generatePairs()
{
    this->fingerprintPairs.clear();

    for (auto i = this->alternativeNames.begin(); i != this->alternativeNames.end(); ++i) {
        this->fingerprintPairs.push_back(FINGERPRINT_PAIR{"SUBJECT", i.key(), 0});
    }
}

void Matcher::generateGenuinePairs()
{
    this->dbtestParams.genuinePairs.clear();

    for (int subject = 0; subject < this->dbtestParams.numberOfSubject; subject++) {
        for(int image1 = subject * this->dbtestParams.imgPerSubject; image1 < subject * this->dbtestParams.imgPerSubject + this->dbtestParams.imgPerSubject; image1++) {
            for(int image2 = image1+1; image2 < subject * this->dbtestParams.imgPerSubject + this->dbtestParams.imgPerSubject; image2++) {

                this->dbtestParams.genuinePairs.push_back(FINGERPRINT_PAIR{this->dbtestParams.keys[image1], this->dbtestParams.keys[image2], 0});

            }
        }
    }
}

void Matcher::generateImpostorPairs()
{   
    this->dbtestParams.impostorPairs.clear();

    for (int image1 = 0; image1 < (this->dbtestParams.numberOfSubject-1) * this->dbtestParams.imgPerSubject; image1 += this->dbtestParams.imgPerSubject) {
        for (int image2 = image1 + this->dbtestParams.imgPerSubject; image2 < this->dbtestParams.numberOfSubject * this->dbtestParams.imgPerSubject; image2 += this->dbtestParams.imgPerSubject) {

            this->dbtestParams.impostorPairs.push_back(FINGERPRINT_PAIR{this->dbtestParams.keys[image1], this->dbtestParams.keys[image2], 0});

        }
    }
}





//IDENTIFICATION, VERIFICATION, DBTEST

void Matcher::identify(unsigned char* subjectISO, const QMultiMap<QString, unsigned char*> &dbISO)
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

void Matcher::verify(const unsigned char* subjectISO, const QVector<unsigned char *> &dbISO)
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

void Matcher::testDatabase(const QMap<QString, QVector<MINUTIA>> &db)
{
    this->mode = dbtest;
    this->dbtestParams.genuineTestDone = false;
    this->dbtestResult.fnmrX.clear(); this->dbtestResult.fnmrY.clear();
    this->dbtestResult.fmrX.clear(); this->dbtestResult.fmrY.clear();

    if (this->matcher == bozorth3) {
        // GENUINES
        this->bozorthTemplates.clear();

        for (auto i = db.begin(); i != db.end(); ++i) {
            this->dbtestParams.keys.push_back(i.key());
            this->bozorthTemplates.insert(i.key(), i.value());
        }
        this->generateGenuinePairs();
        this->generateImpostorPairs();

        this->bozorth3m.setParameters(QThread::idealThreadCount(), this->bozorthTemplates, this->dbtestParams.genuinePairs);
        this->bozorth3m.matchAll();
    }
}



//SLOTS
void Matcher::bozorthMatchingDone(int duration)
{
    if (this->mode == identification) {
        this->fingerprintPairs = this->bozorth3m.getOutputFingerprintPairs();

        int bestMatch = this->findMaxScoreItem();
        if (this->fingerprintPairs[bestMatch].score >= this->thresholds.bozorthThr)
            emit identificationDoneSignal(true, this->alternativeNames.value(this->fingerprintPairs[bestMatch].rightFingerprint), this->fingerprintPairs[bestMatch].score);
        else emit identificationDoneSignal(false, "", -1);
    }
    else if (this->mode == verification) {
        this->fingerprintPairs = this->bozorth3m.getOutputFingerprintPairs();

        int bestMatch = this->findMaxScoreItem();
        if (this->fingerprintPairs[bestMatch].score >= this->thresholds.bozorthThr)
            emit verificationDoneSignal(true);
        else emit verificationDoneSignal(false);
    }
    else if (this->mode == dbtest) {

        double error;

        if (!this->dbtestParams.genuineTestDone) {
            this->dbtestParams.genuinePairs = this->bozorth3m.getOutputFingerprintPairs();
            for (int threshold = 0; threshold < 500; threshold += 1) {
                this->dbtestResult.fnmrX.push_back(threshold);
                error = std::count_if(this->dbtestParams.genuinePairs.begin(), this->dbtestParams.genuinePairs.end(), [=](FINGERPRINT_PAIR fp) {return fp.score < threshold;});
                this->dbtestResult.fnmrY.push_back(error/this->dbtestParams.genuinePairs.size()*100);
            }
            this->dbtestParams.genuineTestDone = true;

            // IMPOSTORS
            this->bozorth3m.setParameters(QThread::idealThreadCount(), this->bozorthTemplates, this->dbtestParams.impostorPairs);
            this->bozorth3m.matchAll();
        }
        else {
            this->dbtestParams.impostorPairs = this->bozorth3m.getOutputFingerprintPairs();
            for (int threshold = 0; threshold < 500; threshold += 1) {
                this->dbtestResult.fmrX.push_back(threshold);
                error = std::count_if(this->dbtestParams.impostorPairs.begin(), this->dbtestParams.impostorPairs.end(), [=](FINGERPRINT_PAIR fp) {return fp.score < threshold;});
                this->dbtestResult.fmrY.push_back(error/this->dbtestParams.impostorPairs.size()*100);
            }
            this->dbtestResult.eer = this->computeEERValue();

            emit dbTestDoneSignal(dbtestResult);
        }
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

double Matcher::computeEERValue()
{
    QVector<double> absDiff;
    for(int i=0; i < this->dbtestResult.fmrY.size(); i++){
        absDiff.push_back(qAbs(this->dbtestResult.fmrY[i] - this->dbtestResult.fnmrY[i]));
    }
    double smallestDiff = *std::min_element(absDiff.begin(), absDiff.end());

    for(int i=0; i< absDiff.size(); i++){
        if(absDiff[i] == smallestDiff) {
            return (this->dbtestResult.fmrY[i] + this->dbtestResult.fnmrY[i])/2.0;
        }
    }

    return 0;
}
