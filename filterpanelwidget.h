#ifndef FILTERPANELWIDGET_H
#define FILTERPANELWIDGET_H

#include <QWidget>
#include <QList> // For QList<QCheckBox*>
#include <QSet>  // For QSet<QString>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QPushButton;
class QVBoxLayout;
class QScrollArea;
class QGroupBox;
QT_END_NAMESPACE

class FilterPanelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FilterPanelWidget(QWidget *parent = nullptr);

    void populateFilters(
        const QSet<QString>& authors,
        const QSet<QString>& sonicTags,
        const QSet<QString>& techniqueTags,
        const QSet<QString>& ugens
    );

    // Methods to get current filter states
    QStringList getCheckedAuthors() const;
    QStringList getCheckedSonicTags() const;
    QStringList getCheckedTechniqueTags() const;
    QStringList getCheckedUgens() const;
    bool isMatchAllLogic() const;
    bool isFavoritesFilterActive() const;
    void setFavoritesFilterActive(bool active);


public slots:
    void resetAllFilters();

signals:
    void filtersChanged(); // Emitted when any filter control changes state

private slots:
    void onFilterControlChanged(); // Connected to all checkboxes and buttons that trigger a filter refresh

private:
    void createCheckboxGroup(
        QVBoxLayout* mainLayout,
        const QString& title,
        const QSet<QString>& items,
        QList<QCheckBox*>& checkboxList
    );

    QVBoxLayout* m_mainLayout; // Main layout for the scrollable widget content
    QScrollArea* m_scrollArea;
    QWidget* m_scrollWidget; // Widget inside scroll area

    QCheckBox* m_filterLogicToggle;    // "Match All" / "Match Any"
    QPushButton* m_favoriteFilterButton; // Toggle for favorites filter
    QPushButton* m_resetFiltersButton;

    QList<QCheckBox*> m_authorCheckboxes;
    QList<QCheckBox*> m_sonicCheckboxes;
    QList<QCheckBox*> m_techniqueCheckboxes;
    QList<QCheckBox*> m_ugenCheckboxes;
};

#endif // FILTERPANELWIDGET_H