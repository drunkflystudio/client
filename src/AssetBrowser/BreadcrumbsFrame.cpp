#include "BreadcrumbsFrame.h"
#include <QHBoxLayout>
#include <QToolButton>

BreadcrumbsFrame::BreadcrumbsFrame(QWidget* parent)
    : QFrame(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    m_rootButton = new QToolButton(this);
    m_rootButton->setText(QStringLiteral("/"));
    m_layout->insertWidget(m_layout->count() - 1, m_rootButton);
    connect(m_rootButton, &QToolButton::clicked, this, &BreadcrumbsFrame::onButtonClicked);
}

BreadcrumbsFrame::~BreadcrumbsFrame()
{
}

void BreadcrumbsFrame::setPath(const QString& path)
{
    m_items = path.split('/');
    if (m_items.length() == 1 && m_items[0].length() == 0)
        m_items.clear();

    int btnN = m_buttons.count();
    int itemsN = m_items.count();
    int i;

    for (i = 0; i < itemsN; i++) {
        QToolButton* button;
        if (i < btnN)
            button = m_buttons[i];
        else {
            button = new QToolButton(this);
            connect(button, &QToolButton::clicked, this, &BreadcrumbsFrame::onButtonClicked);
            m_buttons.append(button);
            m_layout->insertWidget(m_layout->count() - 1, button);
            btnN++;
        }

        button->setText(m_items[i]);
        button->setVisible(true);
    }

    for (; i < btnN; i++)
        m_buttons[i]->setVisible(false);
}

void BreadcrumbsFrame::onButtonClicked()
{
    QObject* button = sender();

    if (button == m_rootButton) {
        emit pathClicked(QString());
        return;
    }

    int j = 0;
    for (QList<QToolButton*>::const_iterator it = m_buttons.begin(); it != m_buttons.end(); ++it) {
        if (*it == button)
            break;
        ++j;
    }

    QString path = m_items[0];
    for (int i = 1; i <= j; i++)
        path = QStringLiteral("%1/%2").arg(path).arg(m_items[i]);

    emit pathClicked(path);
}
