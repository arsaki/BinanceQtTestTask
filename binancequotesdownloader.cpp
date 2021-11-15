#include "binancequotesdownloader.h"

#define ESC_CODE_CLEAR_SCREEN "\u001b[2J"
#define ESC_CODE_GO_UP "\u001b[1H"



BinanceQuotesDownloader::BinanceQuotesDownloader()
{
    logMessage("Ok", "Application started");

    connect(&spotBidOfferWebSocket, &QWebSocket::textMessageReceived, this, &BinanceQuotesDownloader::spotBidOfferMessageReceived);
    connect(&featuresBidOfferWebSocket, &QWebSocket::textMessageReceived, this, &BinanceQuotesDownloader::featuresBidOfferMessageReceived);
    connect(&featuresFundRateWebSocket, &QWebSocket::textMessageReceived, this, &BinanceQuotesDownloader::featuresFundRateMessageReceived);

    spotBidOfferWebSocket.open(QUrl::fromUserInput("wss://stream.binance.com:9443/ws/btcusdt@bookTicker"));
    featuresBidOfferWebSocket.open(QUrl::fromUserInput("wss://fstream.binance.com/ws/btcusdt@bookTicker"));
    featuresFundRateWebSocket.open(QUrl::fromUserInput("wss://fstream.binance.com/ws/btcusdt@markPrice@1s"));

    logMessage("Ok", "WebSocket wss://stream.binance.com:9443/ws/btcusdt@bookTicker connected");
    logMessage("Ok", "WebSocket wss://fstream.binance.com/ws/btcusdt@bookTicker");
    logMessage("Ok", "wss://fstream.binance.com/ws/btcusdt@markPrice@1s");

    connect (&timerEverySec, &QTimer::timeout, this, &BinanceQuotesDownloader::displayCurrentValues);
    connect (&timerEvery30Sec, &QTimer::timeout, this, &BinanceQuotesDownloader::displayCurrentValues);

    timerEverySec.setInterval(1000);
    timerEvery30Sec.setInterval(30000);

    timerEverySec.start();
    timerEvery30Sec.start();
}


void BinanceQuotesDownloader::displayCurrentValues()
{
    //Clear screen by special ESC sequence
    std::cout << ESC_CODE_CLEAR_SCREEN << ESC_CODE_GO_UP;
    std::cout << "BTCUSDT: (bid + offer)/2 = " + std::to_string(bidOfferAverage);
    std::cout << ", BTCUSDT perpetual features: (bid + offer)/2 = " + std::to_string(bidOfferPerpetualAverage);
    std::cout << ", fund rate:  " + std::to_string(fundRate);
    std::cout << ", time to fund rate:  "
                        + countdownFundRate() + ".";
    std::cout << std::endl;
}

void BinanceQuotesDownloader::logCurrentValues()
{
    QFile file;
    file.setFileName(WEBSOCKETLOGFILE);
    file.open(QIODevice::Text | QIODevice::Append | QIODevice::WriteOnly);
    QTextStream textStream(&file);
    // Example:   10/21/2021 12:50:06.744921123; 63 600; 63 700; 0.0295; 04:44:46
    textStream << QDateTime::currentDateTime().toString("MM/dd/yyyy hh:mm:ss.zzzzzzzzz")
                            + "; " + QString::number(static_cast<int>(bidOfferAverage))
                            + "; " + QString::number(static_cast<int>(bidOfferPerpetualAverage))
                            + "; " + QString::number(fundRate)
                            + "; " + QString::fromStdString(countdownFundRate())
                            << Qt::endl;
    file.close();
}

/* Prints message into file, type is Ok either Error*/
void BinanceQuotesDownloader::logMessage(QString type, QString message)
{
    QFile file;
    file.setFileName(LOGFILE);
    file.open(QIODevice::Text | QIODevice::Append | QIODevice::WriteOnly);
    QTextStream textStream(&file);
    QString dateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    textStream<<dateTime + " [" + type + "]: " + message << Qt::endl;
    file.close();
}

std::string BinanceQuotesDownloader::countdownFundRate()
{
    qint64 currentTimeMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 fundRateTimeMs = fundRateTime.toMSecsSinceEpoch();
    qint64 countdownTimeMs = fundRateTimeMs - currentTimeMs;
    std::string countdownFundRateTimeString = QDateTime::fromMSecsSinceEpoch(countdownTimeMs).time().toString("hh:mm:ss").toStdString();
    return countdownFundRateTimeString;
}

void BinanceQuotesDownloader::spotBidOfferMessageReceived(const QString &message)
{
/* message example
    "u":400900217,     // order book updateId
    "s":"BNBUSDT",     // symbol
    "b":"25.35190000", // best bid price
    "B":"31.21000000", // best bid qty
    "a":"25.36520000", // best ask price
    "A":"40.66000000"  // best ask qty
*/

    QJsonObject json = QJsonDocument::fromJson(message.toLatin1()).object();
    double bid =  json["b"].toString().toDouble();
    double offer =  json["a"].toString().toDouble();
    bidOfferAverage = (bid + offer)/2;
}

void BinanceQuotesDownloader::featuresBidOfferMessageReceived(const QString &message)
{
/* message example
 {
  "e":"bookTicker",         // event type
  "u":400900217,            // order book updateId
  "E": 1568014460893,       // event time
  "T": 1568014460891,       // transaction time
  "s":"BNBUSDT",            // symbol
  "b":"25.35190000",        // best bid price
  "B":"31.21000000",        // best bid qty
  "a":"25.36520000",        // best ask price
  "A":"40.66000000"         // best ask qty
}
*/
    QJsonObject json = QJsonDocument::fromJson(message.toLatin1()).object();
    double bid =  json["b"].toString().toDouble();
    double offer =  json["a"].toString().toDouble();
    bidOfferPerpetualAverage = (bid + offer)/2;
}

void BinanceQuotesDownloader::featuresFundRateMessageReceived(const QString &message)
{
/* message example
    {
      "e": "markPriceUpdate",     // Event type
      "E": 1562305380000,         // Event time
      "s": "BTCUSDT",             // Symbol
      "p": "11794.15000000",      // Mark price
      "i": "11784.62659091",      // Index price
      "P": "11784.25641265",      // Estimated Settle Price, only useful in the last hour before the settlement starts
      "r": "0.00038167",          // Funding rate
      "T": 1562306400000          // Next funding time
    }
*/
    QJsonObject json = QJsonDocument::fromJson(message.toLatin1()).object();
    fundRate =  json["r"].toString().toDouble();
    fundRateTime = QDateTime::fromMSecsSinceEpoch( json["T"].toString().toLongLong());
}

