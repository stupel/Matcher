#include "matcher_isoconverter.h"

MatcherISOConverter::MatcherISOConverter(QObject *parent) : QObject(parent)
{
    this->_ISO_template = nullptr;
    this->templateSize = 0;
}

MatcherISOConverter::MatcherISOConverter(int _fpHeight, int _fpWidth, int _fpQuality, QVector<MINUTIA> _minData) :
    fpHeight(_fpHeight),
    fpWidth(_fpWidth),
    fpQuality(_fpQuality),
    minutiaeData(_minData),
    _ISO_template(nullptr),
    templateSize(0)
{ }

MatcherISOConverter::~MatcherISOConverter()
{

}


// funkcia, ktora prekonvertuje informacie o odtlacku do ISO formatu
// funkcia vrati unsigned char*
unsigned char * MatcherISOConverter::convertToISO()
{
    if(this->minutiaeData.size() == 0){
        qDebug() << "Load data first.";
        return nullptr;
    }
    int numberOfMinutiae = this->minutiaeData.size();
    this->templateSize = HEADER_LENGHT + (FINGER_VIEW_HEADER_LENGTH + (numberOfMinutiae * ISO_MINUTIA_LENGTH) + EXTENDED_DATA_BLOCK_LENGTH);
    int byte_offset=0;

    /* ------------------ ISO HEADER --------------------*/

    // FMR block 4B
    unsigned char fmr_block_4B[4]  = {'F','M','R',0};
    memcpy(_ISO_template+byte_offset, fmr_block_4B, sizeof(unsigned char)*4);
    byte_offset +=4;

    // Standard version block 4B
    unsigned char standard_block_4B[4]  = {0x20,0x32,0x30,0};
    memcpy(_ISO_template+byte_offset, standard_block_4B, sizeof(unsigned char)*4);
    byte_offset +=4;

    // Total record length block 4B
    int total_record_length = this->templateSize;
    *(_ISO_template+byte_offset) = ((unsigned char)(total_record_length >> 24)  & 0xff);
    byte_offset++;
    *(_ISO_template+byte_offset) = ((unsigned char)(total_record_length >> 16)  & 0xff);
    byte_offset++;
    *(_ISO_template+byte_offset) = ((unsigned char)(total_record_length >> 8)  & 0xff);
    byte_offset++;
    *(_ISO_template+byte_offset) = ((unsigned char)(total_record_length)  & 0xff);
    byte_offset++;

    // Capture device 2B
    unsigned char capture_block_2B[2]  = {0x00,0x00};
    memcpy(_ISO_template+byte_offset, capture_block_2B, sizeof(unsigned char)*2);
    byte_offset +=2;

    // Image size X 2B
    int image_size_X = this->fpWidth;
    *(_ISO_template+byte_offset) = ((unsigned char)(image_size_X >> 8)  & 0xff);
    byte_offset++;
    *(_ISO_template+byte_offset) = ((unsigned char)(image_size_X )  & 0xff);
    byte_offset++;

    // Image size Y 2B
    int image_size_Y = this->fpHeight;
    *(_ISO_template+byte_offset) = ((unsigned char)(image_size_Y >> 8)  & 0xff);
    byte_offset++;
    *(_ISO_template+byte_offset) = ((unsigned char)(image_size_Y )  & 0xff);
    byte_offset++;

    // Resolution X 2B
    int resolution_X = 197; // 500 ppi = 197 pixels/cm
    *(_ISO_template+byte_offset) = ((unsigned char)(resolution_X >> 8)  & 0xff);
    byte_offset++;
    *(_ISO_template+byte_offset) = ((unsigned char)(resolution_X )  & 0xff);
    byte_offset++;

    // Resolution Y 2B
    int resolution_Y = 197; // 500 ppi = 197 pixels/cm
    *(_ISO_template+byte_offset) = ((unsigned char)(resolution_Y >> 8)  & 0xff);
    byte_offset++;
    *(_ISO_template+byte_offset) = ((unsigned char)(resolution_Y )  & 0xff);
    byte_offset++;

    // Number of finger views 1B
    int numFingViews = 1;
    *(_ISO_template+byte_offset) = ((unsigned char)(numFingViews)  & 0xff);
    byte_offset++;

    // Reserved byte 1B
    int reservedByte = 0;
    *(_ISO_template+byte_offset) = ((unsigned char)(reservedByte)  & 0xff);
    byte_offset++;


    /* ------------------ FINGER VIEW --------------------*/


    /* ----- FINGER VIEW HEADER -----*/

    // Finger position 1B
    int fingerPosition = 0; // 0 az 10
    *(_ISO_template+byte_offset) = ((unsigned char)(fingerPosition)  & 0xff);
    byte_offset++;

    // View number and impression type 1B
    // view number: 0-15
    // impression type: 0-3 alebo 8
    int view_and_impression = 0;
    *(_ISO_template+byte_offset) = ((unsigned char)(view_and_impression)  & 0xff);
    byte_offset++;

    // Finger quality 1B
    int fingerQuality = this->fpQuality; // 0 az 100
    *(_ISO_template+byte_offset) = ((unsigned char)(fingerQuality)  & 0xff);
    byte_offset++;

    // Number of minutiae 1B
    int numMinutiae = numberOfMinutiae;
    *(_ISO_template+byte_offset) = ((unsigned char)(numMinutiae)  & 0xff);
    byte_offset++;


    /* ----- MINUTIAE DATA -----*/

    int minutiaX = -1; // 2 bajty (prve 2 bity su minutia type)
    int minutiaY = -1; // 2 bajty (prve 2 bity su rezervovane)
    int minutiaAngle = -1; // 1 bajt (0-255)
    int minutiaQuality = -1; // 1 bajt (0-100, 0 quality not reported)

    for(int m = 0; m < numberOfMinutiae; m++){
        // kuzlo
        minutiaX = std::bitset<16>(
                    std::bitset<2>(minutiaeData[m].type).to_string() +
                    std::bitset<14>(minutiaeData[m].xy.x()).to_string()
                   ).to_ulong();

        *(_ISO_template+byte_offset) = ((unsigned char)(minutiaX >> 8) & 0xff);
        byte_offset++;
        *(_ISO_template+byte_offset) = ((unsigned char)minutiaX & 0xff);
        byte_offset++;
        minutiaY = minutiaeData[m].xy.y();
        *(_ISO_template+byte_offset) = ((unsigned char)(minutiaY >> 8) & 0xff);
        byte_offset++;
        *(_ISO_template+byte_offset) = ((unsigned char)minutiaY & 0xff);
        byte_offset++;
        minutiaAngle = minutiaeData[m].angle * 180 / M_PI * (255/360);
        *(_ISO_template+byte_offset) = ((unsigned char)minutiaAngle & 0xff);
        byte_offset++;
        minutiaQuality = minutiaeData[m].quality;
        *(_ISO_template+byte_offset) = ((unsigned char)minutiaQuality & 0xff);
        byte_offset++;
    }

    /* ----- EXTENDED DATA -----*/

    // Extended Data Block Length 2B
    int extendedLength = 0;
    *(_ISO_template+byte_offset) = ((unsigned char)(extendedLength >> 8) & 0xff);
    byte_offset++;
    *(_ISO_template+byte_offset) = ((unsigned char)(extendedLength) & 0xff);
    byte_offset++;

    return _ISO_template;
}

int MatcherISOConverter::bitsetToInt(const unsigned char * ISOtemplate, int byte_offset, int byte, bool type)
{
    int bin[6] = {256, 512, 1024, 2048, 4096, 8192};
    int result = 0;

    std::bitset<8> bs = *(ISOtemplate + byte_offset);

    if (type == true) {
        if (bs[6]) return 1;
        else return 0;
    }
    else if (byte == 2) {
        for (int i = 0; i < 6; i++)
            if (bs[i]) result += bin[i];
    }
    else {
        result = bs.to_ulong();
    }

    return result;
}

QVector<MINUTIA> MatcherISOConverter::convertFromISO(const unsigned char * ISOtemplate)
{
    QVector<MINUTIA> minutiae;

    int x, y, type, quality;
    qreal angle;

    int byteCnt = 27;

    int minutia_num = this->bitsetToInt(ISOtemplate, byteCnt++, 1, false);

    for (int i = 0; i < minutia_num; i++) {
        type = this->bitsetToInt(ISOtemplate, byteCnt, 1, true);

        x = this->bitsetToInt(ISOtemplate, byteCnt++, 2, false);
        x += this->bitsetToInt(ISOtemplate, byteCnt++, 1, false);

        y = this->bitsetToInt(ISOtemplate, byteCnt++, 2, false);
        y += this->bitsetToInt(ISOtemplate, byteCnt++, 1, false);

        angle = this->bitsetToInt(ISOtemplate, byteCnt++, 1, false);
        angle = angle / (255.0/360) / 180 * M_PI;

        quality = this->bitsetToInt(ISOtemplate, byteCnt++, 1, false);

        minutiae.push_back({QPoint(x,y), type, angle, quality});
    }

    return minutiae;
}

void MatcherISOConverter::saveISOToFile(const QString & templateFilename)
{
        QFile file(templateFilename);
        if(file.open(QIODevice::WriteOnly)){
            QDataStream datastream(&file);
            for(int i = 0;i <this->templateSize; i++){
                datastream << this->_ISO_template[i];
            }
        }
        else{
            qDebug("Writing to text file failed.");
        }
        file.close();
}

void MatcherISOConverter::load(int _fpHeight, int _fpWidth, int _fpQuality, QVector<MINUTIA> _minData)
{
    this->minutiaeData.clear();
    this->fpHeight = _fpHeight;
    this->fpWidth = _fpWidth;
    this->fpQuality = _fpQuality;
    this->minutiaeData = QVector<MINUTIA>(_minData);
    this->templateSize = HEADER_LENGHT + (FINGER_VIEW_HEADER_LENGTH + (minutiaeData.size() * ISO_MINUTIA_LENGTH) + EXTENDED_DATA_BLOCK_LENGTH);
    this->_ISO_template = new unsigned char[this->templateSize];
}
