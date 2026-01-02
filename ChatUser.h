// SPDX-FileCopyrightText: © 2021 Chris and Flauntbot Contributors <flauntbot@chris-nz.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef CHATUSER_H
#define CHATUSER_H


#include <QString>

/*
 * Represents someone who sent a message. They are used as the author, so far.
 */
struct ChatUser
{
    // literally the name as it's displayed on screen
    QString name = "";
    // URI to user's avatar - technically should be a QUrl, maybe
    QString avatar = "";

    QString first_name = "";
    QString last_name = "";

    // user id, which we use for consistent colouring of their name and possibly avatar. Not important.
    double id = 0;
};


#endif //CHATUSER_H
