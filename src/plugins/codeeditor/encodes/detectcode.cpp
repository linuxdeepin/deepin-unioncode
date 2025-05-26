// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detectcode.h"

#include <QByteArray>
#include <QString>
#include <QDebug>
#include <QDateTime>
#include <QTextCodec>
#include <QRegularExpression>

#include <stdio.h>

#define DISABLE_TEXTCODEC

QMap<QString, QByteArray> DetectCode::sm_LangsMap;

// Minimum detection accuracy judgment, less than 90% requires adjustment of strategy
static const float s_kMinConfidence = 0.9f;

// Manually add UTF BOM information
static QMap<QString, QByteArray> s_byteOrderMark = { { "UTF-16LE", QByteArray::fromHex("FFFE") },
                                                     { "UTF-16BE", QByteArray::fromHex("FEFF") },
                                                     { "UTF-32LE", QByteArray::fromHex("FFFE0000") },
                                                     { "UTF-32BE", QByteArray::fromHex("0000FEFF") } };

/**
  FIXME: Temporary solution, will modify iconv and other encoding conversion processing later
  # GB18030-2022 - UTF-8 encoding conversion
  {s_utf8MapGB18030Data} is the GB18030-UTF8 PUA region mapping table, corresponding to GB18030-2022 specification Appendix D Table D.1
  Replace the differences between 2005 and 2022 specifications, see \l{https://en.wikipedia.org/wiki/GB_18030}
 */
static QHash<QByteArray, quint32> s_utf8MapGB18030Data { { "\uE81E", 0x37903582 },
                                                         { "\uE826", 0x38903582 },
                                                         { "\uE82B", 0x39903582 },
                                                         { "\uE82C", 0x30913582 },
                                                         { "\uE832", 0x31913582 },
                                                         { "\uE843", 0x32913582 },
                                                         { "\uE854", 0x33913582 },
                                                         { "\uE864", 0x34913582 },
                                                         { "\uE78D", 0x36823184 },
                                                         { "\uE78F", 0x37823184 },
                                                         { "\uE78E", 0x38823184 },
                                                         { "\uE790", 0x39823184 },
                                                         { "\uE791", 0x30833184 },
                                                         { "\uE792", 0x31833184 },
                                                         { "\uE793", 0x32833184 },
                                                         { "\uE794", 0x33833184 },
                                                         { "\uE795", 0x34833184 },
                                                         { "\uE796", 0x35833184 } };

/**
  # GB18030-2022 specification Appendix D Table D.2 processing
  Special handling required when converting GB18030 -> UTF8 encoding to preserve special GB18030 encoded characters
  When converting 0xFE51...0xFE91 encoding, GB18030 specification and Unicode 4.1 specification have different conversion results, taking 0xFE51 as an example:
  1. According to GB18030-2022 encoding specification, 0xFE51 converts to \uE816 instead of \u20087, so replace 0xFE51 with conversion identifier 0xFFFFFF01 before conversion;
  2. When iconv encounters 0xFFFFFF01 during conversion, it will exit with error, then convert 0xFFFFFF01 to GB18030-2022 encoding \uE816;
  3. Since according to Unicode 4.1 specification, \uE816 should not exist when converting UTF-8 to GB18030 encoding, it is also preserved and manually restored to 0xFE51.
 */
static QHash<QByteArray, QByteArray> s_ReplaceFromGB18030_2005Error {
    { QByteArray::fromHex("FE51"), QByteArray::fromHex("FFFFFF01") },
    { QByteArray::fromHex("FE52"), QByteArray::fromHex("FFFFFF02") },
    { QByteArray::fromHex("FE53"), QByteArray::fromHex("FFFFFF03") },
    { QByteArray::fromHex("FE6C"), QByteArray::fromHex("FFFFFF04") },
    { QByteArray::fromHex("FE76"), QByteArray::fromHex("FFFFFF05") },
    { QByteArray::fromHex("FE91"), QByteArray::fromHex("FFFFFF06") }
};
static QHash<QByteArray, QByteArray> s_ReplaceToUTF8_2005Error { { "\uE816", QByteArray::fromHex("FFFFFF01") },
                                                                 { "\uE817", QByteArray::fromHex("FFFFFF02") },
                                                                 { "\uE818", QByteArray::fromHex("FFFFFF03") },
                                                                 { "\uE831", QByteArray::fromHex("FFFFFF04") },
                                                                 { "\uE83B", QByteArray::fromHex("FFFFFF05") },
                                                                 { "\uE855", QByteArray::fromHex("FFFFFF06") } };
static QHash<QByteArray, QByteArray> s_ReplaceUtf8ToGB18030_2005Error { { "\uE816", QByteArray::fromHex("FE51") },
                                                                        { "\uE817", QByteArray::fromHex("FE52") },
                                                                        { "\uE818", QByteArray::fromHex("FE53") },
                                                                        { "\uE831", QByteArray::fromHex("FE6C") },
                                                                        { "\uE83B", QByteArray::fromHex("FE76") },
                                                                        { "\uE855", QByteArray::fromHex("FE91") } };

/**
   Similarly, since both 0xFE51 and 0x95329031 convert to \u20087 in Unicode 4.1 specification, but in reverse conversion, \u20087 converts to 0xFE51.
   This causes data to change during bidirectional conversion, so special handling is also needed. When converting UTF-8 to GB18030-2022, replace 0xF0A08287(\u20087),
   And replace with 0x95329031 during actual conversion to ensure accurate GB18030-2022 encoding conversion
 */
static QHash<QByteArray, QByteArray> s_ReplaceFromUtf8_2020Error {
    { QByteArray::fromHex("95329031"), QByteArray::fromHex("FFFF11") },
    { QByteArray::fromHex("95329033"), QByteArray::fromHex("FFFF12") },
    { QByteArray::fromHex("95329730"), QByteArray::fromHex("FFFF13") },
    { QByteArray::fromHex("9536B937"), QByteArray::fromHex("FFFF14") },
    { QByteArray::fromHex("9630BA35"), QByteArray::fromHex("FFFF15") },
    { QByteArray::fromHex("9635B630"), QByteArray::fromHex("FFFF16") },
};
// 0xF0A08287 is the UTF-8 HEX encoding for \u20087
static QHash<QByteArray, QByteArray> s_ReplaceToGB18030_2020Error {
    { QByteArray::fromHex("F0A08287"), QByteArray::fromHex("FFFF11") },
    { QByteArray::fromHex("F0A08289"), QByteArray::fromHex("FFFF12") },
    { QByteArray::fromHex("F0A0838C"), QByteArray::fromHex("FFFF13") },
    { QByteArray::fromHex("F0A19797"), QByteArray::fromHex("FFFF14") },
    { QByteArray::fromHex("F0A2A68F"), QByteArray::fromHex("FFFF15") },
    { QByteArray::fromHex("F0A487BE"), QByteArray::fromHex("FFFF16") },
};
static QHash<QByteArray, QByteArray> s_ReplaceFromUtf8ToGB18030_2020Error {
    { QByteArray::fromHex("95329031"), "\u20087" },
    { QByteArray::fromHex("95329033"), "\u20089" },
    { QByteArray::fromHex("95329730"), "\u200CC" },
    { QByteArray::fromHex("9536B937"), "\u215D7" },
    { QByteArray::fromHex("9630BA35"), "\u2298F" },
    { QByteArray::fromHex("9635B630"), "\u241FE" },
};

// See QTextCodec::mibEnum()
enum MibEncoding {
    UnknownMib = 0,
    UTF_8 = 106,
    GB18030 = 114,
    UTF_16BE = 1013,
    UTF_16LE = 1014,
    UTF_16 = 1015,
    UTF_32 = 1017,
    UTF_32BE = 1018,
    UTF_32LE = 1019,
};

/**
 * @brief Get character encoding format of file based on file header content \a content
 * @param filepath File to get character encoding
 * @param content File header content
 * @return File character encoding format
 *
 * @note For large text files, the file header content \a content may be truncated in the middle of the file, with truncated characters at the end of \a content,
 * Greatly reducing character encoding recognition rate. For this reason, when the recognition rate is too low, trim the tail data and re-detect to improve text recognition rate.
 */
QByteArray DetectCode::getFileEncodingFormat(QString filepath, QByteArray content)
{
    QString charDetectedResult;
    QByteArray ucharDetectRet;
    QByteArrayList icuDetectRetList;
    QByteArray detectRet;
    float charDetConfidence = 0.0f;

    /* chardet encoding recognition */
    QString str(content);
    // Match is Chinese (only in UTF-8 encoding)
    bool bFlag = str.contains(QRegularExpression("[\\x4e00-\\x9fa5]+"));
    if (bFlag) {
        const QByteArray suffix = "为增加探测率保留的中文";
        QByteArray newContent = content;
        // Manually add Chinese characters to avoid misjudging encoding due to too short character length
        newContent += suffix;
        DetectCode::chartDetDetectingTextCoding(newContent, charDetectedResult, charDetConfidence);

        // Large text data may be truncated in the middle of the document, causing unicode Chinese characters to be truncated and parsed as garbled characters, handling some cases
        // According to the text interruption situation, try to parse the encoding and remove the tail characters each time until the recognition rate reaches above 90%
        int tryCount = 5;
        while (charDetConfidence < s_kMinConfidence && newContent.size() > suffix.size() && tryCount-- > 0) {
            // Remove possible garbled tail characters
            newContent.remove(newContent.size() - suffix.size(), 1);
            DetectCode::chartDetDetectingTextCoding(newContent, charDetectedResult, charDetConfidence);
        }
    } else {
        DetectCode::chartDetDetectingTextCoding(content, charDetectedResult, charDetConfidence);

        // Some non-unicode encodings are also Chinese, such as GB18030, BIG5 and other Chinese encodings, also judge the recognition rate, manually intervene multiple detections when the recognition rate is low
        int tryCount = 5;
        QByteArray newContent = content;
        while (charDetConfidence < s_kMinConfidence && !newContent.isEmpty() && tryCount-- > 0) {
            newContent.chop(1);
            DetectCode::chartDetDetectingTextCoding(newContent, charDetectedResult, charDetConfidence);
        }
    }
    ucharDetectRet = charDetectedResult.toLatin1();

    // uchardet encoding recognition. If recognition rate is too low, consider whether it is non-single byte encoding format.
    if (ucharDetectRet.contains("unknown") || ucharDetectRet.contains("ASCII") || ucharDetectRet.contains("???") || ucharDetectRet.isEmpty() || charDetConfidence < s_kMinConfidence) {
        ucharDetectRet = DetectCode::uchardetCode(filepath);
    }

    if (ucharDetectRet.contains("ASCII")) {
        // Use configured default file encoding, default is UTF-8
        detectRet = "UTF-8";
    } else {
        // icu encoding recognition
        DetectCode::icuDetectTextEncoding(filepath, icuDetectRetList);
        detectRet = selectCoding(ucharDetectRet, icuDetectRetList, charDetConfidence);

        if (detectRet.contains("ASCII") || detectRet.isEmpty()) {
            // Use configured default file encoding, default is UTF-8
            detectRet = "UTF-8";
        }
    }

    return detectRet.toUpper();
}

QByteArray DetectCode::uchardetCode(QString filePath)
{
    FILE *fp;
    QByteArray charset;

    size_t buffer_size = 0x10000;
    char *buff = new char[buffer_size];
    memset(buff, 0, buffer_size);

    /* Analyze text encoding through sample characters */
    uchardet_t handle = uchardet_new();

    /* Open the text file to be detected and read a certain number of sample characters */
    fp = fopen(filePath.toLocal8Bit().data(), "rb");

    if (fp) {
        while (!feof(fp)) {
            size_t len = fread(buff, 1, buffer_size, fp);
            int retval = uchardet_handle_data(handle, buff, len);
            if (retval != 0) {
                continue;
            }

            break;
        }
        fclose(fp);

        uchardet_data_end(handle);
        charset = uchardet_get_charset(handle);
    }

    uchardet_delete(handle);
    delete[] buff;
    buff = nullptr;

    if (charset == "MAC-CENTRALEUROPE")
        charset = "MACCENTRALEUROPE";
    if (charset == "MAC-CYRILLIC")
        charset = "MACCYRILLIC";
    if (charset.contains("WINDOWS-"))
        charset = charset.replace("WINDOWS-", "CP");
    return charset;
}

/**
 * @author guoshao
 * @brief  ICU encoding recognition
 * @param  filePath: file path, listDetectRet: variable to store encoding recognition results
 **/
void DetectCode::icuDetectTextEncoding(const QString &filePath, QByteArrayList &listDetectRet)
{
    FILE *file;
    file = fopen(filePath.toLocal8Bit().data(), "rb");
    if (file == nullptr) {
        qInfo() << "fopen file failed.";
        return;
    }

    size_t iBuffSize = 4096;
    char *detected = nullptr;
    char *buffer = new char[iBuffSize];
    memset(buffer, 0, iBuffSize);

    int readed = 0;
    while (!feof(file)) {
        size_t len = fread(buffer, 1, iBuffSize, file);
        readed += len;
        if (readed > 1 * 1024 * 1024) {
            break;
        }

        if (detectTextEncoding(buffer, len, &detected, listDetectRet)) {
            break;
        }
    }

    delete[] buffer;
    buffer = nullptr;
    fclose(file);
}

/**
 * @author guoshao
 * @brief  Inner function for ICU encoding detection
 * @param  data: content to detect, len: length of content to detect, detected: variable to store detected encoding,
 *         listDetectRet: list to store detected encodings
 * @return true: detection successful, false: detection failed
 **/
bool DetectCode::detectTextEncoding(const char *data, size_t len, char **detected, QByteArrayList &listDetectRet)
{
    Q_UNUSED(detected);

    UCharsetDetector *csd;
    const UCharsetMatch **csm;
    int32_t matchCount = 0;

    UErrorCode status = U_ZERO_ERROR;
    csd = ucsdet_open(&status);
    if (status != U_ZERO_ERROR) {
        return false;
    }

    ucsdet_setText(csd, data, len, &status);
    if (status != U_ZERO_ERROR) {
        return false;
    }

    csm = ucsdet_detectAll(csd, &matchCount, &status);
    if (status != U_ZERO_ERROR) {
        return false;
    }

    int readMax = qMin(6, matchCount);
    for (int i = 0; i < readMax; i++) {
        auto str = ucsdet_getName(csm[i], &status);
        if (status != U_ZERO_ERROR) {
            return false;
        }
        listDetectRet << QByteArray(str);
    }

    ucsdet_close(csd);
    return true;
}

/**
 * @author guoshao
 * @brief  Filter detected encodings
 * @param  ucharDetectdRet: encoding result from chardet/uchardet, icuDetectRetList: list storing detected encodings
 * @return Filtered encoding result
 **/
QByteArray DetectCode::selectCoding(QByteArray ucharDetectRet, QByteArrayList icuDetectRetList, float confidence)
{
    // List is not allowed to be empty
    if (icuDetectRetList.isEmpty()) {
        return QByteArray();
    }

    if (!ucharDetectRet.isEmpty()) {
        // Chinese environment prioritizes GB18030 encoding
        if (QLocale::Chinese == QLocale::system().language()) {
            if (confidence < s_kMinConfidence && icuDetectRetList.contains("GB18030")) {
                return QByteArray("GB18030");
            }
        }

        if (ucharDetectRet.contains(icuDetectRetList[0])) {
            return ucharDetectRet;
        } else {
            if (icuDetectRetList.contains("GB18030")) {
                return QByteArray("GB18030");
            } else {
                // Filter out some encoding formats with suffixes, such as UTF-16 BE and UTF-16
                if (icuDetectRetList[0].contains(ucharDetectRet)) {
                    return icuDetectRetList[0];
                }

                return ucharDetectRet;
            }
        }
    }

    if (ucharDetectRet.isEmpty()) {
        if (icuDetectRetList.contains("GB18030")) {
            return QByteArray("GB18030");
        } else {
            return icuDetectRetList[0];
        }
    }

    return QByteArray();
}

/**
 * @brief Detect encoding using libchardet1 encoding detection library
 */
int DetectCode::chartDetDetectingTextCoding(const char *str, QString &encoding, float &confidence)
{
    DetectObj *obj = detect_obj_init();

    if (obj == nullptr) {
        // qInfo() << "Memory Allocation failed\n";
        return CHARDET_MEM_ALLOCATED_FAIL;
    }

#ifndef CHARDET_BINARY_SAFE
    // before 1.0.5. This API is deprecated on 1.0.5
    switch (detect(str, &obj))
#else
    // from 1.0.5
    switch (detect_r(str, strlen(str), &obj))
#endif
    {
    case CHARDET_OUT_OF_MEMORY:
        qInfo() << "On handle processing, occured out of memory\n";
        detect_obj_free(&obj);
        return CHARDET_OUT_OF_MEMORY;
    case CHARDET_NULL_OBJECT:
        qInfo() << "2st argument of chardet() is must memory allocation with detect_obj_init API\n";
        return CHARDET_NULL_OBJECT;
    }

#ifndef CHARDET_BOM_CHECK
    // qInfo() << "encoding:" << obj->encoding << "; confidence: " << obj->confidence;
#else
    // from 1.0.6 support return whether exists BOM
    qInfo() << "encoding:" << obj->encoding << "; confidence: " << obj->confidence << "; bom: " << obj->bom;
#endif

    encoding = obj->encoding;
    confidence = obj->confidence;
    detect_obj_free(&obj);

    return CHARDET_SUCCESS;
}

/**
 * @return Returns the number of bytes that the first character in string \a buf may occupy based on UTF-8 character encoding
 * @note Examples of different UTF-8 byte divisions, except first byte being 0
 *  If first bit of a byte is 0, that byte alone represents a character; if first bit is 1, number of consecutive 1s indicates how many bytes current character occupies.
 *  0000 0000-0000 007F | 0xxxxxxx
 *  0000 0080-0000 07FF | 110xxxxx 10xxxxxx
 *  0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
 *  0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 * @link https://zh.wikipedia.org/wiki/UTF-8
 */
int utf8MultiByteCount(char *buf, size_t size)
{
    // UTF character types: single byte, middle byte, double bytes, three bytes, four bytes
    enum UtfCharType {
        Single,
        Mid,
        DoubleBytes,
        ThreeBytes,
        FourBytes,
    };

    // Get UTF-8 byte sequence value
    auto LeftBitFunc = [](char data) -> int {
        // Return number of leading 1s
        int res = 0;
        while (data & 0x80) {
            res++;
            data <<= 1;
        }
        return res;
    };

    int count = 0;
    while (size > 0 && count < FourBytes) {
        int leftBits = LeftBitFunc(*buf);
        switch (leftBits) {
        case Mid:
            count++;
            break;
        case DoubleBytes:
        case ThreeBytes:
        case FourBytes:
            return leftBits;
        default:
            // Return 1 for more than 4 bytes or single byte
            return 1;
        }

        buf++;
        size--;
    }

    return count;
}

/**
   @brief Check for PUA region anomalies when converting GB18030 to UTF-8

   @note Differences in GB18030 PUA region encoding conversion between versions
   iconv currently uses 2022 flag, encounters error in PUA region (no longer supported in 2022), here compatible with 2005 design,
   When converting between GB18030 and UTF-8, convert PUA region characters according to 2005 standard
   | GB18030 Original Data (HEX)  | GB18030-2005 to UTF8  | GB18030-2022 toUtf8   |
   |------------------------|-----------------------|-----------------------|
   | 0x37903582             |		\uE81E          |		\u9FB4          |
   | 0x38903582             |		\uE826          |		\u9FB5          |
   | 0x39903582             |		\uE82B          |		\u9FB6          |
   | 0x30913582             |		\uE82C          |		\u9FB7          |
   | 0x31913582             |		\uE832          |		\u9FB8          |
   | 0x32913582             |		\uE843          |		\u9FB9          |
   | 0x33913582             |		\uE854          |		\u9FBA          |
   | 0x34913582             |		\uE864          |		\u9FBB          |
   | 0x36823184             |		\uE78D          |		\uFE10          |
   | 0x37823184             |		\uE78F          |		\uFE11          |
   | 0x38823184             |		\uE78E          |		\uFE12          |
   | 0x39823184             |		\uE790          |		\uFE13          |
   | 0x30833184             |		\uE791          |		\uFE14          |
   | 0x31833184             |		\uE792          |		\uFE15          |
   | 0x32833184             |		\uE793          |		\uFE16          |
   | 0x33833184             |		\uE794          |		\uFE17          |
   | 0x34833184             |		\uE795          |		\uFE18          |
   | 0x35833184             |		\uE796          |		\uFE19          |

   The following are special characters, 2005 and 2022 converted encoding, mapping to Unicode encoding
   | GB18030 Original Data (HEX)  | GB18030 to UTF8      | Unicode 4.1 Mapping        |
   |------------------------|----------------------|------------------------|
   | 0x31903295             |       U+E816         |		\u20087         |
   | 0x33903295             |       U+E817         |		\u20089         |
   | 0x30973295             |       U+E818         |		\u200CC         |
   | 0x37B93695             |       U+E831         |		\u215D7         |
   | 0x35BA3096             |       U+E83B         |		\u2298F         |
   | 0x30B63596             |       U+E855         |		\u241FE         |
 */
bool checkGB18030ToUtf8Error(char *buf, size_t size, size_t &replaceLen, QByteArray &appendChar)
{
    // For GB18030-2005 encoding specification PUA region characters, iconv reports error, replace with corresponding character sequence.
    static const int sc_minGB18030PUACharLen = 4;
    if (size < sc_minGB18030PUACharLen) {
        replaceLen = 1;
        appendChar = "?";
        return false;
    }

    quint32 puaChar = *reinterpret_cast<quint32 *>(buf);
    appendChar = s_utf8MapGB18030Data.key(puaChar);
    if (appendChar.isEmpty()) {
        // FFFFFF0X -> \uE816...\uE855
        appendChar = s_ReplaceToUTF8_2005Error.key(QByteArray(buf, sc_minGB18030PUACharLen));
    }

    if (appendChar.isEmpty()) {
        replaceLen = 1;
        appendChar = "?";
        return false;
    } else {
        replaceLen = sc_minGB18030PUACharLen;
        return true;
    }
}

/**
   @brief Check for PUA region anomalies when converting UTF-8 to GB18030
 */
bool checkUTF8ToGB18030Error(char *buf, size_t size, size_t &replaceLen, QByteArray &appendChar)
{
    // The PUA characters to be converted are all 3
    static const int sc_minUTFPUACharLen = 3;
    if (size < sc_minUTFPUACharLen) {
        replaceLen = 1;
        appendChar = "?";
        return false;
    }

    QByteArray puaChar(buf, sc_minUTFPUACharLen);
    quint32 gb18030char = s_utf8MapGB18030Data.value(puaChar, 0);
    if (!gb18030char) {
        // \uE816 -> 0xFE51
        appendChar = s_ReplaceUtf8ToGB18030_2005Error.value(puaChar);
        if (appendChar.isEmpty()) {
            // 0xFFFF11 -> 0x95329031
            appendChar = s_ReplaceFromUtf8_2020Error.key(puaChar);
        }

        if (!appendChar.isEmpty()) {
            replaceLen = sc_minUTFPUACharLen;
            return true;
        }

        replaceLen = 1;
        appendChar = "?";
        return false;
    } else {
        replaceLen = sc_minUTFPUACharLen;
        appendChar = QByteArray(reinterpret_cast<char *>(&gb18030char), sizeof(gb18030char));
        return true;
    }
}

/**
 * @brief Convert input character sequence \a inputStr from encoding \a fromCode to encoding \a toCode, and return the converted character sequence.
 * @return Whether character encoding conversion was successful
 */
bool DetectCode::changeFileEncodingFormat(QByteArray &inputStr,
                                          QByteArray &outStr,
                                          const QString &fromCode,
                                          const QString &toCode)
{
    if (fromCode == toCode) {
        outStr = inputStr;
        return true;
    }

    if (inputStr.isEmpty()) {
        outStr.clear();
        return true;
    }

#ifndef DISABLE_TEXTCODEC
    // Use QTextCodec to handle some encodings
    static QStringList codecList { "GB18030" };
    if (codecList.contains(fromCode) || codecList.contains(toCode)) {
        return convertEncodingTextCodec(inputStr, outStr, fromCode, toCode);
    }
#endif

    iconv_t handle = iconv_open(toCode.toLocal8Bit().data(), fromCode.toLocal8Bit().data());
    if (handle != reinterpret_cast<iconv_t>(-1)) {
        MibEncoding fromMib = UnknownMib;
        QTextCodec *fromCodec = QTextCodec::codecForName(fromCode.toUtf8());
        if (fromCodec) {
            fromMib = static_cast<MibEncoding>(fromCodec->mibEnum());
        }
        // When not using modified Iconv processing from upper layer, skip GB18030 conversion special handling without detecting encoding format
        MibEncoding toMib = UnknownMib;
        char *inbuf = inputStr.data();
        size_t inbytesleft = static_cast<size_t>(inputStr.size());
        size_t outbytesleft = 4 * inbytesleft;
        char *outbuf = new char[outbytesleft];
        char *bufferHeader = outbuf;
        size_t maxBufferSize = outbytesleft;

        memset(outbuf, 0, outbytesleft);

        int errorNum = 0;
        try {
            size_t ret = 0;
            do {
                ret = iconv(handle, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
                if (static_cast<size_t>(-1) == ret) {
                    // Record error information
                    errorNum = errno;

                    // For input error, error code EILSEQ (84), skip current position and add '?'
                    if (EILSEQ == errorNum) {
                        // Exit if buffer insufficient
                        if (outbytesleft == 0) {
                            break;
                        }

                        size_t replaceLen = 1;
                        // Skip error character, set error character to '?'
                        QByteArray appendChar = "?";

                        switch (fromMib) {
                        case UTF_8: {
                            // Special handling for UTF-8 to GB18030 conversion, check for anomalies first
                            if (GB18030 == toMib) {
                                if (checkUTF8ToGB18030Error(inbuf, inbytesleft, replaceLen, appendChar)) {
                                    break;
                                }
                            }

                            // For UTF-8 source encoding, calculate number of bytes to skip
                            replaceLen = static_cast<size_t>(utf8MultiByteCount(inbuf, inbytesleft));
                        } break;
                        case GB18030:
                            // Special handling for GB18030 to UTF-8 conversion, check for anomalies first
                            if (UTF_8 == toMib) {
                                if (checkGB18030ToUtf8Error(inbuf, inbytesleft, replaceLen, appendChar)) {
                                    break;
                                }
                            }
                            break;
                        default:
                            break;
                        }

                        // Replace error character with corresponding character
                        size_t appendSize = static_cast<size_t>(appendChar.size());
                        if (outbytesleft < appendSize) {
                            break;
                        }

                        outbytesleft -= appendSize;
                        ::memcpy(outbuf, appendChar.data(), appendSize);
                        outbuf += appendSize;

                        if (inbytesleft <= replaceLen) {
                            // Move to end
                            inbuf += inbytesleft;
                            inbytesleft = 0;
                            break;
                        }

                        inbuf += replaceLen;
                        inbytesleft -= replaceLen;
                    } else {
                        break;
                    }
                }
            } while (static_cast<size_t>(-1) == ret);

        } catch (const std::exception &e) {
            qWarning() << qPrintable("iconv convert encoding catching exception") << qPrintable(e.what());
        }

        if (errorNum) {
            qWarning() << qPrintable("iconv() convert text encoding error, errocode:") << errorNum;
        }
        iconv_close(handle);

        // Manually add UTF BOM information
        outStr.append(s_byteOrderMark.value(toCode));

        // Calculate actual number of bytes converted by iconv()
        size_t realConvertSize = maxBufferSize - outbytesleft;
        outStr += QByteArray(bufferHeader, static_cast<int>(realConvertSize));

        delete[] bufferHeader;
        bufferHeader = nullptr;

        return true;

    } else {
        qWarning() << qPrintable("Text encoding convert error, iconv_open() failed.");
        // Try using QTextCodec to load
        return convertEncodingTextCodec(inputStr, outStr, fromCode, toCode);
    }
}

/**
 * @brief Use QTextCodec to convert input character sequence \a inputStr from encoding \a fromCode to encoding \a toCode, and return the converted character sequence.
 * @return Whether character encoding conversion was successful
 */
bool DetectCode::convertEncodingTextCodec(QByteArray &inputStr,
                                          QByteArray &outStr,
                                          const QString &fromCode,
                                          const QString &toCode)
{
    QString convertData;
    if (fromCode != "UTF-8") {
        QTextCodec *fromCodec = QTextCodec::codecForName(fromCode.toUtf8());
        if (!fromCodec) {
            return false;
        }

        convertData = fromCodec->toUnicode(inputStr);
    } else {
        convertData = QString::fromUtf8(inputStr);
    }

    if (toCode != "UTF-8") {
        QTextCodec *toCodec = QTextCodec::codecForName(toCode.toUtf8());
        if (!toCodec) {
            return false;
        }

        outStr = toCodec->fromUnicode(convertData);
    } else {
        outStr = convertData.toUtf8();
    }

    // Manually add UTF BOM information
    outStr.append(s_byteOrderMark.value(toCode));
    return true;
}
