#ifndef MATCHERISOCONVERTER_H
#define MATCHERISOCONVERTER_H

#include "matcher_config.h"

#define HEADER_LENGHT 24 // velkost hlavicky suboru je [24 B]
#define FINGER_VIEW_HEADER_LENGTH 4 // velkost hlavicky pre pole FINGER VIEW [4 B]
#define ISO_MINUTIA_LENGTH 6 // velkost jedneho markantu [6 B]
#define EXTENDED_DATA_BLOCK_LENGTH 2 // velkost bloku, ktory informuje o pocte extended bajtov [2 B]

class MatcherISOConverter : public QObject
{
    Q_OBJECT
public:
    explicit MatcherISOConverter(QObject *parent = nullptr);
    // vyska, sirka, kvalita odtlacku, markanty (suradnice, typ, uhol, kvalita)
    MatcherISOConverter(int _fpHeight, int _fpWidth, int _fpQuality, QVector<MINUTIA> _minData);
    ~MatcherISOConverter();

    unsigned char* convertToISO(); // funkcia na vytvorenie sablony v ISO/IEC 19794-2:2005 formate
    QVector<MINUTIA> convertFromISO(const unsigned char *ISOtemplate);
    void saveISOToFile(const QString&); // funkcia na zapis sablony do suboru
    void load(int _fpHeight, int _fpWidth, int _fpQuality, QVector<MINUTIA> _minData); // nacitanie vsetkych potrebnych udajov


private:
    int fpHeight; // vyska odtlacku
    int fpWidth;  // sirka odtlacku
    int fpQuality; // kvalita <0,100>
    QVector<MINUTIA> minutiaeData; // vektor markantov
    // struktura < (x,y), type, angle(0-360), quality(0-100)>
    // kvalita 0 znamena quality not reported
    unsigned char* _ISO_template; // sablona v ISO formate
    int templateSize; // velkost aktualnej sablony

    int bitsetToInt(const unsigned char *ISOtemplate, int byte_offset, int byte, bool type);
};
#endif // MATCHERISOCONVERTER_H
