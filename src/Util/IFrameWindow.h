#ifndef UTIL_IFRAMEWINDOW_H
#define UTIL_IFRAMEWINDOW_H

#ifdef WASM_TARGET
#include <QRasterWindow>
#include <emscripten.h>
#include <emscripten/val.h>

class IFrameWindow final : public QRasterWindow
{
public:
    IFrameWindow(const QString& iframeId = QString(), QWindow* parent = nullptr);
    ~IFrameWindow() override;

    void loadJS(const QString& url);
    void setHtml(const QString& str);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;

private:
    emscripten::val m_iframe;

    void createIFrame(const QString& iframeId);
    void updateIFrameGeometry();

    Q_DISABLE_COPY_MOVE(IFrameWindow)
};

#endif

#endif
