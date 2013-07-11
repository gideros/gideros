#include <QDesktopServices>
#include <QUrl>

void openUrl(const char* url)
{
    QDesktopServices::openUrl(QUrl::fromEncoded(url));
}

bool canOpenUrl(const char *url)
{
    return true;
}
