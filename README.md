# Matcher
Fingerprint matcher module for DBOX
  
This library is a continuation of [Preprocessing](https://github.com/stupel/Preprocessing) and [Extraction](https://github.com/stupel/Extraction) modules, it fully supports the outputs from that libraries.
  
**Dependencies:**
- [Qt5 / Qt Creator 4](https://www.qt.io/download)  
- [Suprema Scanner](http://www.suprema-id.com) - the Suprema library works only with connected fingerprint scanner  
  
*The mentioned or newer versions are recommended*   
  
  
**Getting Started:**
1. You need to provide valid paths to these libraries and their header files in ```.pro``` file.
2. Build and run the project to generate .so (.dll / .lib) files  
3. Include the library and header files to your own application  
    
  <br />
  
## API  
**Identification**
```cpp  
void identify(unsigned char* subjectISO, const QMultiMap<QString, unsigned char *> &dbISO);

void identify(const QVector<MINUTIA> &subject, const QMultiMap<QString, QVector<MINUTIA> > &db); 
```  
  
**Verification**
```cpp  
void verify(unsigned char *subjectISO, const QVector<unsigned char *> &dbISO);  
  
void verify(const QVector<MINUTIA> &subject, const QVector<QVector<MINUTIA> > &db);
```  
  
**Database Test**
```cpp  
int setDBTestParams(int numberOfSubject, int imgPerSubject);  
  
void testDatabase(QMap<QString, QVector<MINUTIA> > &db);  
  
void testDatabase(const QMap<QString, unsigned char *> &dbISO);
```  

**Optional**
```cpp  
int setMatcher(MATCHER matcher);  
```  

Usage:
- If you want set the matcher, you can choose from Bozorth3 (default), Suprema or MCC
- If you want to identify a person, just call ```identify(...)```
  - the subject is the person who you want to identify, the second parameter is a QMultiMap with persons againt whom you want to do the identification, the key each time is the persons name
- If you want to verify a person, just call ```verify(...)```
  - the subject is the person who you want to verify, the second parameter is a QVector with a person againt whom you want to do the verification
- If you want to test a database, just call ```testDatabase(...)```
  - first you must use the  ```setDBTestParams(...)``` function and set the number of subject in your database and the number of fingerprint for each person in the database
  - the keys have to be different and logically named, e.g Person 1: 1_1, 1_2, 1_3 etc.; Person 2: 2_1, 2_2, 2_3 etc., where the first number is the persons ID, the second number is the ordinal number of the fingerprint
  
<br />   
  
  
**SIGNALS:**  
```cpp  
identificationDoneSignal(bool success, QString bestSubject, float bestScore);
  
verificationDoneSignal(bool success);
  
dbTestDoneSignal(DBTEST_RESULT result);
  
matcherProgressSignal(int progress);
  
matcherErrorSignal(int errorCode); 
```  
Notice:  
- The ```identificationDoneSignal``` tells you the identification succeed or not, it each time gives you the best match and the best score as second and third parameters 
- The ```verificationDoneSignal``` tells you the verification succeed or not  
- The ```dbTestDoneSignal``` gives you FMR, FNMR, EER results what you can plot to a graph
- The ```matcherProgressSignal``` shows you the current progress during matching
- You get ```matcherErrorSignal``` with the error code if an error occured during matching
  
<br />  
<br />  
  
**A simple example how to use signals in your application**  
*yourclass.h:*
```cpp  
#include "matcher.h"

class YourClass: public QObject
{
    Q_OBJECT  
  
private:  
    Matcher m;  
    
private slots:
    void verificationDoneSlot(bool success);
}
```

*yourclass.cpp:*
```cpp 
#include "yourclass.h"
YourClass::YourClass()
{
    connect(&m, SIGNAL(verificationDoneSignal(bool success)), this, SLOT(verificationDoneSlot(bool success)));
}

void YourClass::verificationDoneSlot(bool success)
{
    if (success) qDebug() << Verification succeed
    else qDebug() << Verification failed
}
```
For more please visit [Qt Signals & Slots](http://doc.qt.io/archives/qt-4.8/signalsandslots.html).
