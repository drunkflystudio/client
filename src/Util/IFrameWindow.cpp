#include "IFrameWindow.h"
#include <sstream>

#ifdef WASM_TARGET

IFrameWindow::IFrameWindow(const QString& iframeId, QWindow* parent)
    : m_iframe(emscripten::val::undefined())
{
    createIFrame(iframeId);
}

IFrameWindow::~IFrameWindow()
{
}

void IFrameWindow::loadJS(const QString& url)
{
    emscripten::val document = emscripten::val::global("document");
    emscripten::val script = document.call<emscripten::val>("createElement", std::string("script"));
    script.set("src", url.toStdString());
    document["head"].call<void>("appendChild", script);
}

void IFrameWindow::setHtml(const QString& str)
{
    std::stringstream ss;

    ss << "data:text/html;charset=utf-8,";
    for (QChar ch : str) {
        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
             ch == '@' ||
             ch == '\\' ||
             ch == '*' ||
             ch == '_' ||
             ch == '+' ||
             ch == '-' ||
             ch == '.' ||
             ch == '/')
            ss << char(ch.unicode());
        else {
            static const char hex[] = "0123456789abcdef";
            auto value = unsigned(ch.unicode());
            if (value > 31 && value < 127) {
                ss << '%';
                ss << hex[(value >> 4) & 15];
                ss << hex[ value       & 15];
            } else {
                ss << "%26%23x"; // &#x
                ss << hex[ value >> 12     ];
                ss << hex[(value >> 8) & 15];
                ss << hex[(value >> 4) & 15];
                ss << hex[ value       & 15];
                ss << ';';
            }
        }
    }

    m_iframe.set("src", ss.str());
}

void IFrameWindow::resizeEvent(QResizeEvent* event)
{
    QRasterWindow::resizeEvent(event);
    updateIFrameGeometry();
}

void IFrameWindow::moveEvent(QMoveEvent* event)
{
    QRasterWindow::moveEvent(event);
    updateIFrameGeometry();
}

void IFrameWindow::createIFrame(const QString& iframeId)
{
    emscripten::val document = emscripten::val::global("document");
    m_iframe = document.call<emscripten::val>("createElement", std::string("iframe"));
    m_iframe.set("id", iframeId.toStdString());
    document["body"].call<void>("appendChild", m_iframe);

    emscripten::val style = m_iframe["style"];
    style.set("position", "absolute");
    style.set("border", "none");
    style.set("background-color", "#353535");

    updateIFrameGeometry();
}

void IFrameWindow::updateIFrameGeometry()
{
    QRect geom = geometry();

    emscripten::val document = emscripten::val::global("document");
    emscripten::val canvas = document.call<emscripten::val>("getElementById", std::string("qtcanvas"));
    if (canvas.isUndefined())
        qFatal("Could not find canvas element");
    emscripten::val rect = canvas.call<emscripten::val>("getBoundingClientRect");

    QPoint canvasPosition(rect["left"].as<int>(), rect["top"].as<int>());
    QRect iframeGeometry(geom.topLeft() + canvasPosition, geom.size());

    emscripten::val style = m_iframe["style"];
    style.set("left", QStringLiteral("%1px").arg(iframeGeometry.left()).toStdString());
    style.set("top", QStringLiteral("%1px").arg(iframeGeometry.top()).toStdString());
    style.set("width", QStringLiteral("%1px").arg(iframeGeometry.width()).toStdString());
    style.set("height", QStringLiteral("%1px").arg(iframeGeometry.height()).toStdString());
}

#endif
