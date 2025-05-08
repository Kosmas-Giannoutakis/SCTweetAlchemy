#include "filterpanelwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QLabel> // If needed for titles
#include <QDebug>

FilterPanelWidget::FilterPanelWidget(QWidget *parent)
    : QWidget(parent),
      m_filterLogicToggle(nullptr),
      m_favoriteFilterButton(nullptr),
      m_resetFiltersButton(nullptr)
{
    QVBoxLayout* panelLayout = new QVBoxLayout(this); // Layout for FilterPanelWidget itself
    panelLayout->setContentsMargins(0,0,0,0);
    panelLayout->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_scrollWidget = new QWidget(); // This widget will contain the actual filter groups
    m_mainLayout = new QVBoxLayout(m_scrollWidget); // Layout for content inside scroll area
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(6);

    // --- Create Control Buttons (Favorite, Reset, Logic Toggle) FIRST ---
    QWidget* buttonWidget = new QWidget();
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 0, 0, 4);
    buttonLayout->setSpacing(6);

    m_filterLogicToggle = new QCheckBox("Match All", buttonWidget);
    m_filterLogicToggle->setObjectName("FilterLogicToggle");
    m_filterLogicToggle->setToolTip("Check to show tweets matching ALL selected criteria.\nUncheck to show tweets matching ANY selected criterion.");
    m_filterLogicToggle->setChecked(true); // Default to AND logic
    connect(m_filterLogicToggle, &QCheckBox::checkStateChanged, this, &FilterPanelWidget::onFilterControlChanged);

    m_favoriteFilterButton = new QPushButton("Favorites Only", buttonWidget);
    m_favoriteFilterButton->setCheckable(true);
    connect(m_favoriteFilterButton, &QPushButton::toggled, this, &FilterPanelWidget::onFilterControlChanged);

    m_resetFiltersButton = new QPushButton("Reset Filters", buttonWidget);
    m_resetFiltersButton->setToolTip("Reset all filter checkboxes and toggles");
    connect(m_resetFiltersButton, &QPushButton::clicked, this, &FilterPanelWidget::resetAllFilters);

    buttonLayout->addWidget(m_filterLogicToggle);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(m_favoriteFilterButton);
    buttonLayout->addWidget(m_resetFiltersButton);
    m_mainLayout->addWidget(buttonWidget); // Add buttons to the scrollable content

    m_scrollArea->setWidget(m_scrollWidget);
    panelLayout->addWidget(m_scrollArea); // Add scroll area to the FilterPanelWidget's main layout
}

void FilterPanelWidget::populateFilters(
    const QSet<QString>& authors,
    const QSet<QString>& sonicTags,
    const QSet<QString>& techniqueTags,
    const QSet<QString>& ugens)
{
    // Clear previous groups if any (excluding the button bar)
    // This is a bit crude; a more robust way might involve storing group boxes and deleting them.
    QLayoutItem *child;
    while (m_mainLayout->count() > 1 && (child = m_mainLayout->takeAt(1)) != nullptr) { // Keep button bar at index 0
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
    m_authorCheckboxes.clear();
    m_sonicCheckboxes.clear();
    m_techniqueCheckboxes.clear();
    m_ugenCheckboxes.clear();

    createCheckboxGroup(m_mainLayout, "Author", authors, m_authorCheckboxes);
    createCheckboxGroup(m_mainLayout, "Sonic Characteristic", sonicTags, m_sonicCheckboxes);
    createCheckboxGroup(m_mainLayout, "Synthesis Technique", techniqueTags, m_techniqueCheckboxes);
    createCheckboxGroup(m_mainLayout, "UGen", ugens, m_ugenCheckboxes);

    // Add stretch at the very bottom of the scrollable content
    m_mainLayout->addStretch(1);
    qInfo() << "Filter panel populated.";
}


void FilterPanelWidget::createCheckboxGroup(
    QVBoxLayout* mainLayout,
    const QString& title,
    const QSet<QString>& items,
    QList<QCheckBox*>& checkboxList)
{
    if (items.isEmpty()) {
        qDebug() << "No items found for filter group:" << title;
        return;
    }
    QGroupBox *groupBox = new QGroupBox(title);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);
    groupLayout->setContentsMargins(4, 4, 4, 4);
    groupLayout->setSpacing(4);

    QStringList sortedItems = items.values();
    sortedItems.sort(Qt::CaseInsensitive);

    for (const QString& item : sortedItems) {
        QCheckBox *checkbox = new QCheckBox(item);
        checkbox->setObjectName("FilterCheck_" + title.simplified().replace(" ", "_") + "_" + item);
        connect(checkbox, &QCheckBox::checkStateChanged, this, &FilterPanelWidget::onFilterControlChanged);
        groupLayout->addWidget(checkbox);
        checkboxList.append(checkbox);
    }
    groupLayout->addStretch(1); // Stretch inside the group box
    mainLayout->addWidget(groupBox); // Add groupbox to the main scrollable layout
}


void FilterPanelWidget::onFilterControlChanged()
{
    emit filtersChanged();
}

void FilterPanelWidget::resetAllFilters()
{
    // Block signals to prevent multiple emissions of filtersChanged
    bool logicBlocked = m_filterLogicToggle->signalsBlocked();
    bool favBlocked = m_favoriteFilterButton->signalsBlocked();

    m_filterLogicToggle->blockSignals(true);
    m_favoriteFilterButton->blockSignals(true);

    QList<QList<QCheckBox*>> allCheckboxLists = {
        m_authorCheckboxes, m_sonicCheckboxes, m_techniqueCheckboxes, m_ugenCheckboxes
    };

    for (const auto& list : allCheckboxLists) {
        for (QCheckBox* checkbox : list) {
            checkbox->blockSignals(true);
            checkbox->setChecked(false);
            checkbox->blockSignals(false);
        }
    }

    m_filterLogicToggle->setChecked(true); // Default to AND
    m_favoriteFilterButton->setChecked(false);

    m_filterLogicToggle->blockSignals(logicBlocked);
    m_favoriteFilterButton->blockSignals(favBlocked);

    emit filtersChanged(); // Emit signal once after all changes
    qInfo() << "Filters reset in panel.";
}

QStringList FilterPanelWidget::getCheckedAuthors() const {
    QStringList checked;
    for (const QCheckBox* cb : m_authorCheckboxes) { if (cb->isChecked()) checked.append(cb->text()); }
    return checked;
}
QStringList FilterPanelWidget::getCheckedSonicTags() const {
    QStringList checked;
    for (const QCheckBox* cb : m_sonicCheckboxes) { if (cb->isChecked()) checked.append(cb->text()); }
    return checked;
}
QStringList FilterPanelWidget::getCheckedTechniqueTags() const {
    QStringList checked;
    for (const QCheckBox* cb : m_techniqueCheckboxes) { if (cb->isChecked()) checked.append(cb->text()); }
    return checked;
}
QStringList FilterPanelWidget::getCheckedUgens() const {
    QStringList checked;
    for (const QCheckBox* cb : m_ugenCheckboxes) { if (cb->isChecked()) checked.append(cb->text()); }
    return checked;
}

bool FilterPanelWidget::isMatchAllLogic() const {
    return m_filterLogicToggle ? m_filterLogicToggle->isChecked() : true;
}

bool FilterPanelWidget::isFavoritesFilterActive() const {
    return m_favoriteFilterButton ? m_favoriteFilterButton->isChecked() : false;
}

void FilterPanelWidget::setFavoritesFilterActive(bool active) {
    if (m_favoriteFilterButton) {
        bool wasBlocked = m_favoriteFilterButton->signalsBlocked();
        m_favoriteFilterButton->blockSignals(true);
        m_favoriteFilterButton->setChecked(active);
        m_favoriteFilterButton->blockSignals(wasBlocked);
    }
}