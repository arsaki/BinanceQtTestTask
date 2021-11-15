#include <QCoreApplication>
#include <binancequotesdownloader.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    BinanceQuotesDownloader binanceQuotesDownloader;

    return a.exec();
}
