#include "searchlineedit.h"

#include <QKeyEvent> // Make sure QKeyEvent is included here too

// Constructor - simply calls the base class constructor
SearchLineEdit::SearchLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
}

// The overridden key press event handler
void SearchLineEdit::keyPressEvent(QKeyEvent *event)
{
    // Check if Down or Up arrow key was pressed
    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up)
    {
        // Emit our custom signal instead of letting QLineEdit handle it for navigation
        emit navigationKeyPressed(event);

        // We don't call the base class implementation for Up/Down keys,
        // effectively intercepting them. We also don't strictly need
        // event->accept() here because we aren't passing it on, but it's
        // good practice if we wanted to stop further event propagation.
    }
    else
    {
        // For ALL OTHER keys (letters, numbers, backspace, enter, etc.),
        // call the original QLineEdit::keyPressEvent. If you forget this,
        // you won't be able to type in the search box!
        QLineEdit::keyPressEvent(event);
    }
}