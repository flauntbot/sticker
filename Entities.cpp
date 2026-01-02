// SPDX-FileCopyrightText: © 2021 Chris and Flauntbot Contributors <flauntbot@chris-nz.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "Entities.h"


Styles entityType(const QString &what)
{
    // inconsistency: I don't *think* my desktop tg highlights phone numbers
    if (what == "bold") {
        return bold;
    } else if (what == "bot_command") {
        return bot_command;
    } else if (what == "cashtag") {
        return cashtag;
    } else if (what == "code") {
        return code;
    } else if (what == "email") {
        return
            email;
    } else if (what == "hashtag") {
        return
            hashtag;
    } else if (what == "italic") {
        return
            italic;
    } else if (what == "mention") {
        return
            mention;
    } else if (what == "phone_number") {
        return
            phonenumber;
    } else if (what == "pre") {
        return
            pre;
    } else if (what == "strikethrough") {
        return
            strikethrough;
    } else if (what == "text_link") {
        return
            text_link;
    } else if (what == "underline") {
        return
            underline;
    } else if (what == "url") {
        return
            url;
    }

    // default/fallback. MAY be (not guaranteed) a ZERO. A NO-OP for things we couldn't identify.
    return _;
}