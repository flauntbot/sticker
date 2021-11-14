// SPDX-FileCopyrightText: © 2021 Chris and Flauntbot Contributors <flauntbot@chris-nz.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include <iostream>

#include <QPixmap>
#include <QGuiApplication>
#include <QWindow>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "StickerGenerator.h"

// this program is a simple example of generating an image (perhaps a telegram sticker) from some arbitrary JSON.
// the arbitrary JSON has a striking resemblance to message update json in tg bot api, but
// of course you could use it to draw weird little quote bubbles for some other purpose I guess.

// I make no guarantee that this will perform sanely.

int main(int argc, char **argv)
{
    QString defaultVal =
        R"({"backgroundColor":"#243447","width":512,"message":{"entities":[{"type":"bot_command","length":12,"offset":0}],"from":{"id":136958297,"avatar":"/tmp/photo.png","name":"Chris 🇳🇿"},"text":"/addsticker2","chatId":136958297},"scale":2})";

    // if we have no output file, exit immediately with a message
    if (!argv[1] || strcmp(argv[1],"") == 0) {
        std::printf("Usage:\n<cat_or_echo_some_json> | %s <output_image_filename>\nExample JSON:\n%s\n", argv[0], defaultVal.toStdString().c_str());
        return 1;
    }

    // Qt docs say it MUST run, but that just does setup we don't need and starts an event loop we also don't need
    QGuiApplication app(argc, argv);

    // get data from stdin, unmarshall it a bit so we can feed it to the appropriate method
    QTextStream stream(stdin);
    QString val = stream.readAll();
    if (val.isEmpty()) {
//        val = defaultVal;
        std::printf("%s\n%s\n", "You need to pass stdin some json, with structure like this:", defaultVal.toLocal8Bit().data());
        return 1;
    }
    QJsonObject j = QJsonDocument::fromJson(val.toUtf8()).object();
    auto m = j["message"].toObject();
    if (m.isEmpty()) {
        std::printf("%s\n%s\n", "You need to pass stdin in some json, with structure like this:", defaultVal.toLocal8Bit().data());
        return 1;
    }

    // we expect entities that look like telegram bot api's, but we normalize a bit
    QList<Entity> entities;
    for (auto en: m["entities"].toArray()) {
        auto ent = en.toObject();
        entities
            .push_back({.type=entityType(ent["type"].toString()), .offset=ent["offset"].toInt(), .length=ent["length"]
                .toInt()});
    }

    auto u = m["from"].toObject();

    ChatMessage
        m2(entities, {.name=u["name"].toString(), .avatar=u["avatar"].toString(), .first_name=u["first_name"].toString(), .last_name=u["last_name"].toString(), .id=u["id"].toDouble()}, m["text"].toString());

    auto w = j["width"].toInt();

    // we pass in ChatMessage m2 constructed from input json, and it includes Entities and ChatUser from the same
    auto pic2 = StickerGenerator::generate(QColor(j["backgroundColor"].toString()).rgb(),
                                           m2,
                                           w,
                                           j["scale"].toInt()).toImage();

    // tg says somewhere in docs that sticker input MUST be 512px along its longest edge,so
    pic2 = pic2.scaled(w, w, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    pic2.save(argv[1]);

    return 0;
}
