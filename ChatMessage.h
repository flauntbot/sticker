// SPDX-FileCopyrightText: © 2021 Chris and Flauntbot Contributors <flauntbot@chris-nz.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <QString>
#include <QList>
#include <utility>
#include "ChatUser.h"
#include "Entities.h"

/*
 * Represents a chat message which we probably want to draw. Can come with formatting information.
 * Needs a sender.
 * Can be a reply to another message (but doesn't have to be).
 */
struct ChatMessage
{

ChatMessage(QList<Entity> ents, ChatUser frm, const QString &txt)
    {
        from = std::move(frm);
        entities = std::move(ents);
        text = txt;
        replyMessage = nullptr;
    }

    // the user who sent this message
    ChatUser from;

    // the text that they sent in the message
    QString text;

    // the (formatting/etc) entities to apply to the message
    QList<Entity> entities;

    // the reply message (if there is one) .... untested functionality
    ChatMessage *replyMessage{};
};


#endif //CHATMESSAGE_H
