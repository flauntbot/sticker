// SPDX-FileCopyrightText: © 2021 Chris and Flauntbot Contributors <flauntbot@chris-nz.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef ENTITIES_H
#define ENTITIES_H

#include <QString>

/*
 * these represent formatting in telegram. We mark them like this so we can push it through switch/case stuffs
 * the underscore is default/fallback. MAY be (not yet guaranteed) a ZERO. A NO-OP for things we couldn't identify.
 */
enum Styles
{
    _,
    bold,
    bot_command,
    cashtag,
    code,
    email,
    hashtag,
    italic,
    mention,
    phonenumber,
    pre,
    strikethrough,
    text_link,
    underline,
    url
};

// so for example, "abcdefgh" with "bold", offset "2", length "2", would be "ab*cd*efgh"
struct Entity
{
    // what to do
    Styles type;
    // offset from start of string, where to start doing it
    int offset;
    // how long (in characters) it's done for
    qsizetype length;
};

/*
 * Describes an entity type (given as a string) using our enum system
 *
 * @param what - a string, like "bold", saying what kind of entity it is
 *
 * @returns what type of entity it is, actually
 */
Styles entityType(const QString &what);

#endif //ENTITIES_H
