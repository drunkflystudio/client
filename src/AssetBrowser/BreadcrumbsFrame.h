#ifndef ASSETBROWSER_BREADCRUMBSFRAME_H
#define ASSETBROWSER_BREADCRUMBSFRAME_H

#include <QList>
#include <QFrame>

class QHBoxLayout;
class QToolButton;

class BreadcrumbsFrame final : public QFrame
{
    Q_OBJECT

public:
    explicit BreadcrumbsFrame(QWidget* parent = NULL);
    ~BreadcrumbsFrame() override;

    Q_SIGNAL void pathClicked(const QString& path);

    void setPath(const QString& path);

private:
    QHBoxLayout* m_layout;
    QToolButton* m_rootButton;
    QList<QToolButton*> m_buttons;
    QStringList m_items;

    void onButtonClicked();

    Q_DISABLE_COPY_MOVE(BreadcrumbsFrame)
};

#endif
