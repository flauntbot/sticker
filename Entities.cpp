// SPDX-FileCopyrightText: © 2021 Chris and Flauntbot Contributors <flauntbot@chris-nz.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "Entities.h"


Styles entityType(const QString &what)
{
    // inconsistency: I don't *think* my desktop tg highlights phone numbers
    if (what == "bold") {
        return bold;
    } if (what == "bot_command") {
        return bot_command;
    } if (what == "cashtag") {
        return cashtag;
    } if (what == "code") {
        return code;
    } if (what == "email") {
        return
            email;
    } if (what == "hashtag") {
        return
            hashtag;
    } if (what == "italic") {
        return
            italic;
    } if (what == "mention") {
        return
            mention;
    } if (what == "text_mention") {
        return
            mention;
    } if (what == "phone_number") {
        return
            phonenumber;
    } if (what == "pre") {
        return
            pre;
    } if (what == "strikethrough") {
        return
            strikethrough;
    } if (what == "text_link") {
        return
            text_link;
    } if (what == "underline") {
        return
            underline;
    } if (what == "url") {
        return
            url;
    }

    // default/fallback. MAY be (not guaranteed) a ZERO. A NO-OP for things we couldn't identify.
    return _;
}
