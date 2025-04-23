#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent> // Include for key events

class SearchLineEdit : public QLineEdit
{
    Q_OBJECT // Needed for signals/slots

public:
    explicit SearchLineEdit(QWidget *parent = nullptr);

signals:
    // Signal emitted when user presses Down Arrow or Up Arrow in the search field
    void navigationKeyPressed(QKeyEvent *event);

protected:
    // Override the key press event handler from QLineEdit
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // SEARCHLINEEDIT_H