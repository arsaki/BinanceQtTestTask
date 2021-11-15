#ifndef BINANCEQUOTESDOWNLOADER_H
#define BINANCEQUOTESDOWNLOADER_H
#include <QObject>
#include <QWebSocket>
//#include <QAbstractSocket>
#include <QDateTime>
#include <QTimer>
#include <QThread>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <string>
#include <unistd.h>
//#include <chrono>
//using namespace std::chrono;

#define LOGFILE "BinanceQuotesDownloader.log"
#define WEBSOCKETLOGFILE "BinanceWebSocket.log"


class BinanceQuotesDownloader : public QObject
{
    Q_OBJECT
    QWebSocket spotBidOfferWebSocket;
    QWebSocket featuresBidOfferWebSocket;
    QWebSocket featuresFundRateWebSocket;

    double bidOfferAverage;
    double bidOfferPerpetualAverage;
    double fundRate;
    QDateTime fundRateTime;
    QTimer timerEverySec;
    QTimer timerEvery30Sec;


    void logMessage(QString type, QString message);
    std::string countdownFundRate();
public:
    BinanceQuotesDownloader();
public slots:
    void displayCurrentValues();
    void logCurrentValues();
    void spotBidOfferMessageReceived(const QString &message);
    void featuresBidOfferMessageReceived(const QString &message);
    void featuresFundRateMessageReceived(const QString &message);


};

#endif // BINANCEQUOTESDOWNLOADER_H
